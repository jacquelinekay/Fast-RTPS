/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This generated file is licensed to you under the terms described in the
 * fastcdr_LICENSE file included in this fastcdr distribution.
 *
 *************************************************************************
 * 
 * @file deadlinepayloadPublisher.h
 * This header file contains the declaration of the publisher functions.
 *
 * This file was generated by the tool fastcdrgen.
 */


#ifndef _DEADLINEPAYLOAD_PUBLISHER_H_
#define _DEADLINEPAYLOAD_PUBLISHER_H_

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/publisher/PublisherListener.h>

#include "deadlinepayloadPubSubTypes.h"

using namespace eprosima::fastrtps;

class deadlinepayloadPublisher 
{
public:
	deadlinepayloadPublisher();
	virtual ~deadlinepayloadPublisher();
	bool init();
	void run();
private:
	Participant *mp_participant;
	Publisher *mp_publisher;
	
	bool double_time;								//Used to force a period double on a certain key

	class PubListener : public PublisherListener
	{
	public:
		PubListener() : n_matched(0){};
		~PubListener(){};
		void onPublicationMatched(Publisher* pub,MatchingInfo& info);
		int n_matched;
	} m_listener;
	HelloMsgPubSubType myType;
};

#endif // _DEADLINEPAYLOAD_PUBLISHER_H_