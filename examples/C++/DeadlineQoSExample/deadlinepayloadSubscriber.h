/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This generated file is licensed to you under the terms described in the
 * fastcdr_LICENSE file included in this fastcdr distribution.
 *
 *************************************************************************
 * 
 * @file deadlinepayloadSubscriber.h
 * This header file contains the declaration of the subscriber functions.
 *
 * This file was generated by the tool fastcdrgen.
 */


#ifndef _DEADLINEPAYLOAD_SUBSCRIBER_H_
#define _DEADLINEPAYLOAD_SUBSCRIBER_H_

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include "deadlinepayloadPubSubTypes.h"

#include "deadlineQoS.h"
#include "boost/thread.hpp"
#include "boost/bind.hpp"
#include "boost/asio.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "mapableKey.h"

using namespace eprosima::fastrtps;
using namespace boost::asio;


class deadlinepayloadSubscriber 
{
public:
	deadlinepayloadSubscriber(deadline_timer &timer,io_service &io_service);
	virtual ~deadlinepayloadSubscriber();
	bool init();
	void run();
private:
	Participant *mp_participant;
	Subscriber *mp_subscriber;
	//io_service &io;
	class SubListener : public SubscriberListener
	{
	public:
		SubListener(deadline_timer &timer, io_service &ioserv) : n_matched(0),n_msg(0), myDeadline(timer,ioserv){};
		~SubListener(){};
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info);
		void onNewDataMessage(Subscriber* sub);
		SampleInfo_t m_info;
		int n_matched;
		int n_msg;
		deadlineQoS myDeadline;
		
	} m_listener;
	HelloMsgPubSubType myType;
};

#endif // _deadlinepayload_SUBSCRIBER_H_
