/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Domain.h
 *
 */

#ifndef DOMAIN_H_
#define DOMAIN_H_

#include "attributes/ParticipantAttributes.h"

namespace eprosima{
namespace fastrtps{


class ParticipantListener;
class Participant;
class ParticipantImpl;
class Publisher;
class PublisherAttributes;
class PublisherListener;
class Subscriber;
class SubscriberAttributes;
class SubscriberListener;
class TopicDataType;

/**
 * Class Domain, use to interact with the Publisher Subscriber API of the Fast RTPS implementation.
 *  @ingroup FASTRTPS_MODULE
 */
class Domain
{
	typedef std::pair<Participant*,ParticipantImpl*> t_p_Participant;

	Domain();
	virtual ~Domain();
public:
	/**
	 * Create a Participant.
	 * @param att Participant Attributes.
	 * @param listen ParticipantListener Pointer.
	 * @return Participant pointer. (nullptr if not created.)
	 */
	RTPS_DllAPI static Participant* createParticipant(ParticipantAttributes& att, ParticipantListener* listen = nullptr);
	/**
	 * Create a Publisher in a Participant.
	 * @param part Pointer to the participant where you want to create the Publisher.
	 * @param att PublisherAttributes.
	 * @param listen Pointer to the PublisherListener.
	 * @return Pointer to the created Publisher (nullptr if not created).
	 */
	RTPS_DllAPI static Publisher* createPublisher(Participant* part, PublisherAttributes& att, PublisherListener* listen = nullptr);
	/**
	 * Create a Subscriber in a Participant.
	 * @param part Pointer to the participant where you want to create the Publisher.
	 * @param att SubscriberAttributes.
	 * @param listen Pointer to the SubscriberListener.
	 * @return Pointer to the created Subscriber (nullptr if not created).
	 */
	RTPS_DllAPI static Subscriber* createSubscriber(Participant* part, SubscriberAttributes& att, SubscriberListener* listen = nullptr);
	/**
	 * Remove a Participant and all associated publishers and subscribers.
	 * @param part Pointer to the participant.
	 * @return True if correctly removed.
	 */
	RTPS_DllAPI static bool removeParticipant(Participant* part);
	/**
	 * Remove a Publisher.
	 * @param pub Pointer to the Publisher.
	 * @return True if correctly removed.
	 */
	RTPS_DllAPI static bool removePublisher(Publisher* pub);
	/**
	 * Remove a Subscriber.
	 * @param sub Pointer to the Subscriber.
	 * @return True if correctly removed.
	 */
	RTPS_DllAPI static bool removeSubscriber(Subscriber* sub);

	/**
	 * Return a registered type.
	 * @param part Pointer to the Participant.
	 * @param typeName Name of the type.
	 * @param type Returned type.
	 * @return True if type was found.
	 */
    RTPS_DllAPI static bool getRegisteredType(Participant* part, const char* typeName, TopicDataType** type);

	/**
	 * Register a type in a participant.
	 * @param part Pointer to the Participant.
	 * @param type Pointer to the Type.
	 * @return True if correctly registered.
	 */
	RTPS_DllAPI static bool registerType(Participant* part, TopicDataType * type);

	/**
	 * Unregister a type in a participant.
	 * @param part Pointer to the Participant.
	 * @param typeName Name of the type.
	 * @return True if correctly unregistered.
	 */
	RTPS_DllAPI static bool unregisterType(Participant* part, const char* typeName);
	/**
	 * Stop and remove all participants, publishers and subscribers in this Domain.
	 */
	RTPS_DllAPI static void stopAll();
private:
	static std::vector<t_p_Participant> m_participants;


};

} /* namespace  */
} /* namespace eprosima */

#endif /* DOMAIN_H_ */
