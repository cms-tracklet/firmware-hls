#include "TrackletProcessor.h"



ap_uint<1> nearFullTEBuff(const ap_uint<TEBuffer::kNBufferDepthBits>& writeptr, 
			  const ap_uint<TEBuffer::kNBufferDepthBits>& readptr) {
  ap_uint<3> writeptr1=writeptr+1;
  ap_uint<3> writeptr2=writeptr+2;
  ap_uint<3> writeptr3=writeptr+3;
  ap_uint<1> result=writeptr1==readptr||writeptr2==readptr||writeptr3==readptr;
  return result;
}

ap_uint<(1<<(2*TrackletEngineUnit<BARRELPS>::kNBitsBuffer))> nearFullTEUnitInit() {

  ap_uint<(1<<(2*TrackletEngineUnit<BARRELPS>::kNBitsBuffer))> lut(0);
  int i;
  for(i=0;i<(1<<(2*TrackletEngineUnit<BARRELPS>::kNBitsBuffer));i++) {
    ap_uint<TrackletEngineUnit<BARRELPS>::kNBitsBuffer> wptr,rptr;
    ap_uint<2*TrackletEngineUnit<BARRELPS>::kNBitsBuffer> address(i);
    (rptr,wptr)=address;
    ap_uint<TrackletEngineUnit<BARRELPS>::kNBitsBuffer> wptr1=wptr+1;
    ap_uint<TrackletEngineUnit<BARRELPS>::kNBitsBuffer> wptr2=wptr+2;
    ap_uint<TrackletEngineUnit<BARRELPS>::kNBitsBuffer> wptr3=wptr+3;
    ap_uint<1> result=wptr1==rptr||wptr2==rptr||wptr3==rptr;
    lut[i]=result;
  }
  return lut;
}



////////////////////////////////////////////////////////////////////////////////
// Top functions for various TrackletProcessors (TP). For each iteration of
// the main processing loop, a TC retrieves a pair of stub indices from one of
// the stub-pair memories, and in turn, these indices are used to retrieve one
// stub each from an inner and an outer all-stub memory. This pair of stubs is
// used to calculate a rough set of helix parameters, which are written to the
// tracklet-parameter memory if the tracklet passes cuts on rinv and z0. Rough
// projections to additional layers and disks are also calculated and are
// written to the appropriate tracklet-projection memories.
////////////////////////////////////////////////////////////////////////////////
void TrackletProcessor_L1L2D(
    const BXType bx,
    const ap_uint<10> lut[2048],
    const ap_uint<8> regionlut[2048],
    const AllStubInnerMemory<BARRELPS> innerStubs[2],
    const AllStubMemory<BARRELPS>* outerStubs,
    const VMStubTEOuterMemoryCM<BARRELPS, kNbitsrzbin, kNbitsphibin, kNTEUnits> outerVMStubs,
    TrackletParameterMemory * trackletParameters,
    TrackletProjectionMemory<BARRELPS> projout_barrel_ps[TC::N_PROJOUT_BARRELPS],
    TrackletProjectionMemory<BARREL2S> projout_barrel_2s[TC::N_PROJOUT_BARREL2S],
    TrackletProjectionMemory<DISK> projout_disk[TC::N_PROJOUT_DISK]
) {
#pragma HLS inline recursive
#pragma HLS resource variable=lut core=ROM_2P_BRAM  latency=2
#pragma HLS resource variable=regionlut core=ROM_2P_BRAM latency=2
#pragma HLS resource variable=innerStubs[0].get_mem() latency=2
#pragma HLS resource variable=innerStubs[1].get_mem() latency=2
#pragma HLS resource variable=outerStubs->get_mem() latency=2
#pragma HLS resource variable=outerVMStubs.get_mem() latency=2
#pragma HLS array_partition variable=projout_barrel_ps complete
#pragma HLS array_partition variable=projout_barrel_2s complete
#pragma HLS array_partition variable=projout_disk complete

 TP_L1L2D: TrackletProcessor<TF::L1L2, 
			     TC::D, 
			     kNTEUnits,
			     BARRELPS, 
			     BARRELPS,
			     1, //Outer phi region
			     kNbitsrzbin,
			     kNbitsphibin,
			     2, //Number of inner AS memories
			     108>(bx, 
				  lut, 
				  regionlut, 
				  innerStubs, 
				  outerStubs, 
				  outerVMStubs,
				  trackletParameters,
				  projout_barrel_ps,
				  projout_barrel_2s,
				  projout_disk
				  );

}

////////////////////////////////////////////////////////////////////////////////
