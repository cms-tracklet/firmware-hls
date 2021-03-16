#include "InputRouterTop.h"


void InputRouterTop( const BXType bx
  , const unsigned int hNmemories // number of memories 
  , const ap_uint<kLINKMAPwidth> hLinkWord // input link LUT 
  , const ap_uint<kBINMAPwidth> hPhBnWord  // n phi bins LUT 
  , const int kPhiCorrtable_L1[cNEntriesLUT] // corrections frst brl lyr  
  , const int kPhiCorrtable_L2[cNEntriesLUT] // corrections scnd brl lyr   
  , const int kPhiCorrtable_L3[cNEntriesLUT] // corrections thrd brl lyr  
  , ap_uint<kNBits_DTC> hInputStubs[kMaxStubsFromLink] // input stubs 
  , BXType & bx_o // output bx 
  , DTCStubMemory hOutputStubs[cNMemories]) {

  #pragma HLS clock domain = slow_clock
  #pragma HLS stream variable = hInputStubs depth = 1
  
  // unsigned int nMems=0;
  // unsigned int nMemsPerLyr[kMaxLyrsPerDTC]; 
  // CountMemories<kMaxLyrsPerDTC>(hPhBnWord, nMems, nMemsPerLyr);
  // #ifndef __SYNTHESIS__
  //   std::cout << "Numnber of memories is " << nMems << "\n";
  // #endif
  //DTCStubMemory *cLclCpy = new DTCStubMemory[nMems]; 
  InputRouter<cNMemories,cNEntriesLUT>( bx
      , hLinkWord
      , hPhBnWord
      , kPhiCorrtable_L1
      , kPhiCorrtable_L2
      , kPhiCorrtable_L3
      , hInputStubs
      , bx_o
      , hOutputStubs);

  //delete hOutputStubs;
  //hOutputStubs = cLclCpy;
}
