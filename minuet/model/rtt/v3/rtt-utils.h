#ifndef RTT_UTILS_H
#define RTT_UTILS_H

#include "ns3/core-module.h"

namespace ns3 {

class RTTUtils {

public:
	virtual ~RTTUtils ();

	static constexpr double_t DISCOVERY_INTERVAL = 1.0;
	static constexpr double_t RTT_THRESHOLD = 0.01;
    static constexpr double_t NEIGHBOR_EXPIRATION_TIME = 3.0;
    static constexpr double_t GRACE_PERIOD = 2.0;
};

}


#endif
