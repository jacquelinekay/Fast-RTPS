/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterProxy.cpp
 *
 */

#include <fastrtps/rtps/reader/WriterProxy.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/utils/RTPSLog.h>
#include <fastrtps/utils/TimeConversion.h>

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <fastrtps/rtps/reader/timedevent/HeartbeatResponseDelay.h>
#include <fastrtps/rtps/reader/timedevent/WriterProxyLiveliness.h>

using namespace eprosima::fastrtps::rtps;

/*!
 * @brief Auxiliary function to change status in a range.
 */
void WriterProxy::for_each_set_status_from(decltype(WriterProxy::m_changesFromW)::iterator first,
        decltype(WriterProxy::m_changesFromW)::iterator last,
        ChangeFromWriterStatus_t status,
        ChangeFromWriterStatus_t new_status)
{
    auto it = first;
    while(it != last)
    {
        if(it->getStatus() == status)
        {
            ChangeFromWriter_t newch(*it);
            newch.setStatus(new_status);

            auto hint = m_changesFromW.erase(it);

            it = m_changesFromW.insert(hint, newch);
        }

        ++it;
    }
}

void WriterProxy::for_each_set_status_from_and_maybe_remove(decltype(WriterProxy::m_changesFromW)::iterator first,
        decltype(WriterProxy::m_changesFromW)::iterator last,
        ChangeFromWriterStatus_t status,
        ChangeFromWriterStatus_t orstatus,
        ChangeFromWriterStatus_t new_status)
{
    auto it = first;
    while(it != last)
    {
        if(it->getStatus() == status || it->getStatus() == orstatus)
        {
            if(it != m_changesFromW.begin())
            {
                ChangeFromWriter_t newch(*it);
                newch.setStatus(new_status);

                auto hint = m_changesFromW.erase(it);

                it = m_changesFromW.insert(hint, newch);
                ++it;
                continue;
            }
        }

        // UNKNOWN or MISSING at the beginning or
        // LOST or RECEIVED at the beginning.
        if(it == m_changesFromW.begin())
        {
            changesFromWLowMark_ = it->getSequenceNumber();
            it = m_changesFromW.erase(it);
        }
    }
}

static const int WRITERPROXY_LIVELINESS_PERIOD_MULTIPLIER = 1;

static const char* const CLASS_NAME = "WriterProxy";

WriterProxy::~WriterProxy()
{
	//pDebugInfo("WriterProxy destructor"<<endl;);
	if(mp_writerProxyLiveliness!=nullptr)
		delete(mp_writerProxyLiveliness);

	delete(mp_heartbeatResponse);
	delete(mp_mutex);
}

WriterProxy::WriterProxy(RemoteWriterAttributes& watt,
		StatefulReader* SR) :
												mp_SFR(SR),
												m_att(watt),
												m_acknackCount(0),
												m_nackfragCount(0),
												m_lastHeartbeatCount(0),
												mp_heartbeatResponse(nullptr),
												mp_writerProxyLiveliness(nullptr),
												m_heartbeatFinalFlag(false),
												m_isAlive(true),
												mp_mutex(new boost::recursive_mutex())

{
	const char* const METHOD_NAME = "WriterProxy";
	m_changesFromW.clear();
	//Create Events
	mp_writerProxyLiveliness = new WriterProxyLiveliness(this,TimeConv::Time_t2MilliSecondsDouble(m_att.livelinessLeaseDuration)*WRITERPROXY_LIVELINESS_PERIOD_MULTIPLIER);
	mp_heartbeatResponse = new HeartbeatResponseDelay(this,TimeConv::Time_t2MilliSecondsDouble(mp_SFR->getTimes().heartbeatResponseDelay));
	if(m_att.livelinessLeaseDuration < c_TimeInfinite)
		mp_writerProxyLiveliness->restart_timer();
	logInfo(RTPS_READER,"Writer Proxy created in reader: "<<mp_SFR->getGuid().entityId);
}

void WriterProxy::missing_changes_update(const SequenceNumber_t& seqNum)
{
	const char* const METHOD_NAME = "missing_changes_update";
	logInfo(RTPS_READER,m_att.guid.entityId<<": changes up to seqNum: " << seqNum <<" missing.");
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    // Check was not removed from container.
    if(seqNum > changesFromWLowMark_)
    {
        if(m_changesFromW.size() == 0 || m_changesFromW.rbegin()->getSequenceNumber() < seqNum)
        {
            // Set already values in container.
            for_each_set_status_from(m_changesFromW.begin(), m_changesFromW.end(),
                    ChangeFromWriterStatus_t::UNKNOWN, ChangeFromWriterStatus_t::MISSING);

            // Changes only already inserted values.
            bool will_be_the_last = maybe_add_changes_from_writer_up_to(seqNum, ChangeFromWriterStatus_t::MISSING);
            (void)will_be_the_last;
            assert(will_be_the_last);

            // Add requetes sequence number.
            ChangeFromWriter_t newch(seqNum);
            newch.setStatus(ChangeFromWriterStatus_t::MISSING);
            m_changesFromW.insert(m_changesFromW.end(), newch);
        }
        else
        {
            // Find it. Must be there.
            auto last_it = m_changesFromW.find(ChangeFromWriter_t(seqNum));
            assert(last_it != m_changesFromW.end());
            for_each_set_status_from(m_changesFromW.begin(), ++last_it,
                    ChangeFromWriterStatus_t::UNKNOWN, ChangeFromWriterStatus_t::MISSING);
        }
    }

	print_changes_fromWriter_test2();
}

bool WriterProxy::maybe_add_changes_from_writer_up_to(const SequenceNumber_t& sequence_number,
        const ChangeFromWriterStatus_t default_status)
{
    bool returnedValue = false;
    // Check if CacheChange_t is in the container or not.
    SequenceNumber_t lastSeqNum = changesFromWLowMark_;

    if(m_changesFromW.size() > 0)
        lastSeqNum = m_changesFromW.rbegin()->getSequenceNumber();

    if(sequence_number > lastSeqNum)
    {
        returnedValue = true;

        // If it is not in the container, create info up to its sequence number.
        ++lastSeqNum;
        for(; lastSeqNum < sequence_number; ++lastSeqNum)
        {
            ChangeFromWriter_t newch(lastSeqNum);
            newch.setStatus(default_status);
            m_changesFromW.insert(m_changesFromW.end(), newch);
        }
    }

    return returnedValue;
}

void WriterProxy::lost_changes_update(const SequenceNumber_t& seqNum)
{
	const char* const METHOD_NAME = "lost_changes_update";
	logInfo(RTPS_READER,m_att.guid.entityId<<": up to seqNum: "<<seqNum);
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    // Check was not removed from container.
    if(seqNum > changesFromWLowMark_)
    {
        if(m_changesFromW.size() == 0 || m_changesFromW.rbegin()->getSequenceNumber() < seqNum)
        {
            // Set already values in container.
            for_each_set_status_from_and_maybe_remove(m_changesFromW.begin(), m_changesFromW.end(),
                    ChangeFromWriterStatus_t::UNKNOWN, ChangeFromWriterStatus_t::MISSING,
                    ChangeFromWriterStatus_t::LOST);

            // There is at least one not LOST or RECEIVED.
            if(m_changesFromW.size() != 0)
            {
                // Changes only already inserted values.
                bool will_be_the_last = maybe_add_changes_from_writer_up_to(seqNum, ChangeFromWriterStatus_t::LOST);
                (void)will_be_the_last;
                assert(will_be_the_last);
            }
            // Any in container, then not insert new lost.
            else
            {
                changesFromWLowMark_ = seqNum - 1;
            }
        }
        else
        {
            // Find it. Must be there.
            auto last_it = m_changesFromW.find(ChangeFromWriter_t(seqNum));
            assert(last_it != m_changesFromW.end());
            for_each_set_status_from_and_maybe_remove(m_changesFromW.begin(), last_it,
                    ChangeFromWriterStatus_t::UNKNOWN, ChangeFromWriterStatus_t::MISSING,
                    ChangeFromWriterStatus_t::LOST);
        }
    }

	print_changes_fromWriter_test2();
}

bool WriterProxy::received_change_set(const SequenceNumber_t& seqNum)
{
	const char* const METHOD_NAME = "received_change_set";
	logInfo(RTPS_READER, m_att.guid.entityId << ": seqNum: " << seqNum);
    return received_change_set(seqNum, true);
}

bool WriterProxy::irrelevant_change_set(const SequenceNumber_t& seqNum)
{
    return received_change_set(seqNum, false);
}

bool WriterProxy::received_change_set(const SequenceNumber_t& seqNum, bool is_relevance)
{
	const char* const METHOD_NAME = "received_change_set";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    // Check if CacheChange_t was already and it was already removed from changesFromW container.
    if(seqNum <= changesFromWLowMark_)
    {
        logInfo(RTPS_READER, "Change " << seqNum << " <= than max available sequence number " << changesFromWLowMark_);
        return false;
    }

    // Maybe create information because it is not in the m_changesFromW container.
    bool will_be_the_last = maybe_add_changes_from_writer_up_to(seqNum);

    // If will be the last element, insert it at the end.
    if(will_be_the_last)
    {
        // There are others.
        if(m_changesFromW.size() > 0)
        {
            ChangeFromWriter_t chfw(seqNum);
            chfw.setStatus(RECEIVED);
            chfw.setRelevance(is_relevance);
            m_changesFromW.insert(m_changesFromW.end(), chfw);
        }
        // Else not insert
        else
            changesFromWLowMark_ = seqNum;
    }
    // Else it has to be found and change state.
    else
    {
        auto chit = m_changesFromW.find(ChangeFromWriter_t(seqNum));

        // Has to be in the container.
        assert(chit != m_changesFromW.end());
        // Has not be received yet or lost.
        assert(chit->getStatus() == UNKNOWN || chit->getStatus() == MISSING);

        if(chit != m_changesFromW.begin())
        {
            ChangeFromWriter_t newch(*chit);
            newch.setStatus(RECEIVED);
            newch.setRelevance(is_relevance);

            auto hint = m_changesFromW.erase(chit);

            m_changesFromW.insert(hint, newch);
        }
        else
        {
            changesFromWLowMark_ = seqNum;
            m_changesFromW.erase(chit);
            cleanup();
        }

    }

    print_changes_fromWriter_test2();

	return true;
}


const std::vector<ChangeFromWriter_t> WriterProxy::missing_changes()
{
    std::vector<ChangeFromWriter_t> returnedValue;
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    for(auto ch : m_changesFromW)
    {
        if(ch.getStatus() == MISSING)
        {
            // If MISSING, then is relevant.
            assert(ch.isRelevant());
            returnedValue.push_back(ch);
        }
    }


    print_changes_fromWriter_test2();

    return returnedValue;
}



const SequenceNumber_t WriterProxy::available_changes_max() const
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    return changesFromWLowMark_;
}

void WriterProxy::print_changes_fromWriter_test2()
{
	const char* const METHOD_NAME = "status";
	std::stringstream ss;
	ss << this->m_att.guid.entityId<<": ";

	for(auto it = m_changesFromW.begin(); it != m_changesFromW.end(); ++it)
	{
		ss << it->getSequenceNumber() <<"("<<it->isRelevant()<<","<<it->getStatus()<<")-";
	}

	std::string auxstr = ss.str();
	logInfo(RTPS_READER,auxstr;);
}

void WriterProxy::assertLiveliness()
{
	const char* const METHOD_NAME = "assertLiveliness";

	logInfo(RTPS_READER,this->m_att.guid.entityId << " Liveliness asserted");

	//boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

	m_isAlive=true;

	this->mp_writerProxyLiveliness->cancel_timer();
	this->mp_writerProxyLiveliness->restart_timer();
}

void WriterProxy::setNotValid(const SequenceNumber_t& seqNum)
{
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    // Check sequence number is in the container, because it was not clean up.
    if(seqNum <= changesFromWLowMark_)
        return;

    auto chit = m_changesFromW.find(ChangeFromWriter_t(seqNum));

    // Element must be in the container. In other case, bug.
    assert(chit != m_changesFromW.end());
    // If the element will be set not valid, element must be received.
    // In other case, bug.
    assert(chit->getStatus() == RECEIVED);

    // Cannot be in the beginning because process of cleanup
    assert(chit != m_changesFromW.begin());

    ChangeFromWriter_t newch(*chit);
    newch.notValid();

    auto hint = m_changesFromW.erase(chit);

    m_changesFromW.insert(hint, newch);
}

void WriterProxy::cleanup()
{
    auto chit = m_changesFromW.begin();

    while(chit != m_changesFromW.end() &&
            (chit->getStatus() == RECEIVED || chit->getStatus() == LOST))
    {
        changesFromWLowMark_ = chit->getSequenceNumber();
        chit = m_changesFromW.erase(chit);
    }
}

bool WriterProxy::areThereMissing()
{
    bool returnedValue = false;
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    for(auto ch : m_changesFromW)
    {
        if(ch.getStatus() == ChangeFromWriterStatus_t::MISSING)
        {
            returnedValue = true;
            break;
        }
    }

    return returnedValue;
}

size_t WriterProxy::unknown_missing_changes_up_to(const SequenceNumber_t& seqNum)
{
    size_t returnedValue = 0;
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    if(seqNum > changesFromWLowMark_)
    {
        for(auto ch = m_changesFromW.begin(); ch != m_changesFromW.end() && ch->getSequenceNumber() < seqNum; ++ch)
        {
            if(ch->getStatus() == ChangeFromWriterStatus_t::UNKNOWN ||
                    ch->getStatus() == ChangeFromWriterStatus_t::MISSING)
                ++returnedValue;
        }
    }

    return returnedValue;
}
