#ifndef _AUX_
#define _AUX_


#include "ns3/address.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

using namespace ns3;


uint32_t C_DC = 0;
uint32_t C_DT= 0;
uint32_t tempCollisionDC = 0;
uint32_t collisionDCStore = 0;
uint64_t lastTime = 0;
uint64_t beforeLastTime = 0;

#endif
