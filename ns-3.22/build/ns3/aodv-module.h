
#ifdef NS3_MODULE_COMPILATION
# error "Do not include ns3 module aggregator headers from other modules; these are meant only for end user scripts."
#endif

#ifndef NS3_MODULE_AODV
    

// Module headers:
#include "Confidant-packet.h"
#include "Detector.h"
#include "Monitor.h"
#include "PathManager.h"
#include "ReputationSystem.h"
#include "TrustManager.h"
#include "aodv-dpd.h"
#include "aodv-helper.h"
#include "aodv-id-cache.h"
#include "aodv-neighbor.h"
#include "aodv-packet.h"
#include "aodv-routing-protocol.h"
#include "aodv-rqueue.h"
#include "aodv-rtable.h"
#include "confidant.h"
#endif
