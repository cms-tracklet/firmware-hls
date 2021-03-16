#include "MatchCalculatorTop.h"

void MatchCalculatorTop(BXType bx,
                        const CandidateMatchMemory match[maxMatchCopies],
                        const AllStubMemory<BARRELPS>* allstub,
                        const AllProjectionMemory<BARRELPS>* allproj,
                        BXType& bx_o,
                        FullMatchMemory<BARREL> fullmatch[maxFullMatchCopies]
                        )
{
 #pragma HLS inline off
 #pragma HLS interface register port=bx_o
 #pragma HLS resource variable=match[0].get_mem() latency=2
 #pragma HLS resource variable=match[1].get_mem() latency=2
 #pragma HLS resource variable=match[2].get_mem() latency=2
 #pragma HLS resource variable=match[3].get_mem() latency=2
 #pragma HLS resource variable=match[4].get_mem() latency=2
 #pragma HLS resource variable=match[5].get_mem() latency=2
 #pragma HLS resource variable=match[6].get_mem() latency=2
 #pragma HLS resource variable=match[7].get_mem() latency=2
 #pragma HLS resource variable=allstub->get_mem() latency=2
 #pragma HLS resource variable=allproj->get_mem() latency=2

  const auto LAYER  = TF::L3;
  const auto DISK   = TF::D1;
  const auto PHISEC = MC::C;
  MC_L3PHIC: MatchCalculator<BARRELPS, BARRELPS, BARREL, maxMatchCopies, maxFullMatchCopies, LAYER, DISK, PHISEC>
    (
      bx,
      match,
      allstub, allproj, bx_o,
      fullmatch
    );

} 
