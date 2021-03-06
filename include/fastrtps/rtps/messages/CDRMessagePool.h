/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CDRMessagePool.h
 *
 */

#ifndef CDRMESSAGEPOOL_H_
#define CDRMESSAGEPOOL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "../common/CDRMessage_t.h"
#include <vector>

namespace boost
{
    class mutex;
}

namespace eprosima {
namespace fastrtps{
namespace rtps {

/**
*@ingroup COMMON_MODULE
*/
class CDRMessagePool {
public:
	/**
	* @param defaultGroupSize Number of messages per allocated group.
	*/
	CDRMessagePool(uint32_t defaultGroupSize);
	virtual ~CDRMessagePool();
	
	//!
	CDRMessage_t& reserve_CDRMsg();
	
	/**
	* @param payload Payload size for the reserved message.
	*/
	CDRMessage_t& reserve_CDRMsg(uint16_t payload);
	
	/**
	* @param obj
	*/
	void release_CDRMsg(CDRMessage_t& obj);
protected:
	std::vector<CDRMessage_t*> m_free_objects;
	std::vector<CDRMessage_t*> m_all_objects;
	uint16_t m_group_size;
	void allocateGroup();
	void allocateGroup(uint16_t payload);
    boost::mutex *mutex_;
};





}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* CDRMESSAGEPOOL_H_ */
