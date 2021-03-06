#include "deadlineQoS.h"

using namespace boost:: asio;
using namespace eprosima;
using namespace eprosima::fastrtps;

void deadlineQoS::callback()
{
	//for(int i=0;i<32;i++){
	//	deadlineQoSlist[i].mtx.lock();
	//	if(deadlineQoSlist[i].flag == 0){										//Check if flag has been set since last execution
	//		//Flag inactive -> QoS missed
	//		std::cout << "Deadline QoS on key index " << i << " missed." << std::endl;	//TODO: Bring is out of the mutex
	//	}
	//	deadlineQoSlist[i].flag = 0;											//Always reset flag for the next iteration
	//	deadlineQoSlist[i].mtx.unlock();
	//}
	mapmtx.lock();
	std::cout << "Map holds " << deadlineQoSmap.size() << " different keys" << std::endl;
	for(auto it = deadlineQoSmap.begin(); it != deadlineQoSmap.end(); it++){
		
		if(it->second == false){
			std::cout << "Deadline QoS on key index ";
			for(int i=0;i<16;i++){
				std::cout << (char)it->first.value[i];
			}
			std::cout << " missed." << std::endl;
		}
		//After checking the value, reset flag to zero
		it->second = false;
	}
	mapmtx.unlock();
	wait();
}

void deadlineQoS::wait()
{
	 t.expires_from_now(boost::posix_time::seconds(1)); //repeat rate here
     t.async_wait(boost::bind(&deadlineQoS::callback, this));
}

void deadlineQoS::setFlag(mapable_key target)
{
	mapmtx.lock();
	if(deadlineQoSmap.find(target)!=deadlineQoSmap.end()){
		//Exists
		deadlineQoSmap.at(target)=true;
	}else{
		//Does not exist
		deadlineQoSmap.insert(std::pair<mapable_key,bool>(target,true));
	}
	//deadlineQoSmap.[target]=true;
	mapmtx.unlock();
	//if(index<32){
	//	deadlineQoSlist[index].mtx.lock();
	//	deadlineQoSlist[index].flag = true;
	//	deadlineQoSlist[index].mtx.unlock();
	//}
}

void deadlineQoS::init(){
	//Not really needed now
	std::cout << "Deadline QoS Service started" << std::endl;
	//for(int i=0;i<32;i++){
	//	deadlineQoSlist[i].mtx.lock();
	//	deadlineQoSlist[i].flag = false;
	//	deadlineQoSlist[i].mtx.unlock();
	//}
}

void deadlineQoS::runner(){
		wait();
		io.run();
}

void deadlineQoS::run(){
		boost::thread dlqos(boost::bind(&deadlineQoS::runner,this));
}

void deadlineQoS::stop(){
	t.cancel();
	io.stop();
}
