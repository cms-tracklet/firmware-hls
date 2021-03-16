#ifndef TrackletAlgorithm_InputRouterTop_h
#define TrackletAlgorithm_InputRouterTop_h

#include "InputRouter.h"


// at the moment I'm changing this for each link 
// is that really the only way to do this? 

// only for PS10G_1_A for now 
// other cases to be added 
// when I have final link map
//constexpr unsigned int cTest = (const)(std::atoi(getenv ("TEST")));
constexpr unsigned int cNMemories = 17; 
constexpr unsigned int cNEntriesLUT = kSizePhiCorrTablePS;


void InputRouterTop( const BXType bx
	, const unsigned int hNmemories // number of memories 
	, const ap_uint<kLINKMAPwidth> kInputLink // input link LUT 
  	, const ap_uint<kBINMAPwidth> kNPhiBns  // n phi bins LUT 
  	, const int kPhiCorrtable_L1[] // corrections frst brl lyr  
	, const int kPhiCorrtable_L2[] // corrections scnd brl lyr  
	, const int kPhiCorrtable_L3[] // corrections thrd brl lyr   
	, ap_uint<kNBits_DTC> hInputStubs[kMaxStubsFromLink]//input stubs 
	, BXType & bx_o // output bx 
    , DTCStubMemory hOutputStubs[cNMemories]);//output memories 


#endif


