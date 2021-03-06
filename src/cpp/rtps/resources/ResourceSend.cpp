/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResourceSend.cpp
 *
 */

#include <fastrtps/rtps/resources/ResourceSend.h>
#include "ResourceSendImpl.h"
#include "../participant/RTPSParticipantImpl.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

ResourceSend::ResourceSend()
{
	// TODO Auto-generated constructor stub
	mp_impl = new ResourceSendImpl();

}

ResourceSend::~ResourceSend() {
	// TODO Auto-generated destructor stub
	delete(mp_impl);
}

bool ResourceSend::initSend(RTPSParticipantImpl* pimpl, const Locator_t& loc,
		uint32_t sendsockBuffer,bool useIP4, bool useIP6)
{
	return mp_impl->initSend(pimpl,loc,sendsockBuffer,useIP4,useIP6);
}

void ResourceSend::sendSync(CDRMessage_t* msg, const Locator_t& loc)
{
	return mp_impl->sendSync(msg,loc);
}

boost::recursive_mutex* ResourceSend::getMutex()
{
	return mp_impl->getMutex();
}

void ResourceSend::loose_next_change()
{
	return mp_impl->loose_next();
}

}
} /* namespace rtps */
} /* namespace eprosima */
