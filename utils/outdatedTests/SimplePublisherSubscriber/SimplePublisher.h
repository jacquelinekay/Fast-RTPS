/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This generated file is licensed to you under the terms described in the
 * fastrtps_LICENSE file included in this fastrtps distribution.
 *
 *************************************************************************
 * 
 * @file SimplePublisher.h
 * This header file contains the declaration of the publisher functions.
 *
 * This file was generated by the tool fastrtpsgen.
 */


#ifndef _Simple_PUBLISHER_H_
#define _Simple_PUBLISHER_H_

#include "fastrtps/rtps_all.h"

class SimplePublisher 
{
public:
	SimplePublisher();
	virtual ~SimplePublisher();
	bool init();
	void run();
private:
	RTPSParticipant *mp_RTPSParticipant;
	Publisher *mp_publisher;
	
	class PubListener : public PublisherListener
	{
	public:
		PubListener() : n_matched(0){};
		~PubListener(){};
		void onPublicationMatched(MatchingInfo info);
		int n_matched;
	} m_listener;
};

#endif // _Simple_PUBLISHER_H_