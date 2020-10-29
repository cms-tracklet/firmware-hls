#ifndef TRACKLETENGINEUNIT_H
#define TRACKLETENGINEUNIT_H

#include "Constants.h"
#include "VMStubTEOuterMemoryCM.h"

class TrackletEngineUnitBase {
 public:
  enum BitWidths{
    kNBitsBuffer=4
  };
};

template<int VMSTEType>
class TrackletEngineUnit : public TrackletEngineUnitBase {

 public:
  typedef ap_uint<VMStubTEOuter<VMSTEType>::kVMSTEOIDSize+AllStubInner<BARRELPS>::kAllStubInnerSize> STUBID;
  typedef ap_uint<kNBits_MemAddrBinned> NSTUBS;
  typedef ap_uint<TrackletEngineUnitBase::kNBitsBuffer> INDEX;

 TrackletEngineUnit() {
    idle_ = true;
  }


 inline void init(BXType bxin, 
		  AllStubInner<BARRELPS>::AllStubInnerData innerstub,
		  ap_uint<64> memstubs,
		  ap_uint<16> memmask,
		  ap_uint<3> slot,
		  ap_uint<3> rzbinfirst,
		  ap_uint<3> rzbindiffmax
 ) {
#pragma HLS inline
  idle_ = false;
  bx_ = bxin;
  memstubs_ = memstubs;
  memmask_ = memmask;
  maskmask_ = 0xFFFFFFFF;
#ifndef __SYNTHESIS__
  std::cout << "memstubs_: "<<memstubs_.to_string(2)<<std::endl;
  std::cout << "memmask_: "<<memmask_.to_string(2)<<std::endl;
#endif
   

  //std::cout << "TEUnit::init memstubs = "<<memstubs.to_string(2)<<std::endl; 
  //memindex_ = 0;
  istub_=0;
  innerstub_=innerstub;
  slot_=slot;
  rzbinfirst_=rzbinfirst;
  rzbindiffmax_=rzbindiffmax;
  //(ireg_, next_, nstubs_)=memstubs_.range(7,0);
}
 

 void reset() {
   writeindex_ = 0;
   readindex_ = 0;
   idle_ = true;
 }

bool empty() {
#pragma HLS inline  
  return (writeindex_==readindex_);
}

bool idle() {
#pragma HLS inline  
  return idle_;
}

bool full() {
#pragma HLS inline  
  return (writeindex_+1==readindex_);
}

bool nearfull() {
#pragma HLS inline  
  return ((writeindex_+2==readindex_)||(writeindex_+1==readindex_));
}

STUBID read() {
#pragma HLS inline  
  //std::cout << "TEUNIT read: "<<this<<" "<<readindex_<<std::endl;
  return stubids_[readindex_++];
}

void write(STUBID stubs) {
#pragma HLS inline  
  //std::cout << "TEUNIT write: "<<this<<" "<<writeindex_<<std::endl;
  stubids_[writeindex_++]=stubs;
}

 inline void step( const VMStubTEOuterMemoryCM<VMSTEType> &outervmstubs,
		   const ap_uint<1> ptinnerLUT[256], 
		   const ap_uint<1> ptouterLUT[256],
		   bool nearfull) {
#pragma HLS inline
#pragma HLS PIPELINE II=1
   
   ap_uint<4> memindex(0);

   ap_uint<16> masktmp=memmask_&maskmask_;

   memindex = masktmp.test(0) ? 0 :
     masktmp.test(1) ? 1 :
     masktmp.test(2) ? 2 :
     masktmp.test(3) ? 3 :
     masktmp.test(4) ? 4 :
     masktmp.test(5) ? 5 :
     masktmp.test(6) ? 6 :
     masktmp.test(7) ? 7 :
     masktmp.test(8) ? 8 :
     masktmp.test(9) ? 9 :
     masktmp.test(10) ? 10 :
     masktmp.test(11) ? 11 :
     masktmp.test(12) ? 12 :
     masktmp.test(13) ? 13 :
     masktmp.test(14) ? 14 :
     15;


   (next_, ireg_)=memindex;
   nstubs_=memstubs_.range((memindex*4)+3,memindex*4);
   idle_=idle_||nstubs_==0;
   
   bool good=!(idle()||nearfull);
   
   
   ap_uint<2> iAllstub=1; //FIXME need to be template parameter
   ap_uint<5> iAllstub_ireg =  (iAllstub, ireg_);
   
   ap_uint<3> ibin(slot_+next_);
   ap_uint<12> stubadd( (ireg_, ibin,istub_) );
   ap_uint<1> current_next=next_;
   

#ifndef __SYNTHESIS__
   if (good) {
     assert(nstubs_!=0);
     //std::cout << "nstubs_ nentries memmask_ memindex ireg_ ibin slot_ next_: "
     // 	       <<" "<<nstubs_<<" "<<outervmstubs.getEntries(bx_,(ireg_,ibin))
     //	       << " " << memmask_.to_string(2)<<" "<<memindex<<" "<<ireg_<<" "<<ibin<<" "<<slot_<<" "<<next_<<std::endl;
     assert(nstubs_==outervmstubs.getEntries(bx_,(ireg_,ibin)));
   }
#endif
   
   NSTUBS zero(0);
   NSTUBS istubtmp=istub_+1;
   ap_uint<1> allstubs=istubtmp==nstubs_;
   ap_uint<1> notallstubs=!allstubs;
   istub_=allstubs?zero:istubtmp;
   maskmask_.range(memindex,memindex)=notallstubs;

#ifndef __SYNTHESIS__
   //std::cout << "Updated memmask_: "<<memmask_.to_string(2)<<std::endl;
#endif


   idle_=idle_||(memmask_==0&&allstubs);

   const auto& outervmstub = outervmstubs.read_mem(bx_,stubadd);

   const auto& finephi = outervmstub.getFinePhi();
   const auto& rzbin = (current_next, outervmstub.getFineZ()); 
   
   ap_uint<8> outerfinephi = (iAllstub_ireg, finephi);
   
   int nbitsfinephidiff=5;

   int idphi =  outerfinephi - AllStubInner<BARRELPS>(innerstub_).getFinePhi();
   //int idphi =  ((1<<nbitsfinephidiff)+outerfinephi - AllStubInner<BARRELPS>(innerstub_).getFinePhi())&((1<<nbitsfinephidiff)-1);
   bool inrange = abs(idphi) < (1<<(nbitsfinephidiff));
   
   if (idphi<0) idphi+=(1<<nbitsfinephidiff);

   bool rzcut=!(rzbin<rzbinfirst_ || rzbin > rzbinfirst_ + rzbindiffmax_);

   const auto& outerbend = outervmstub.getBend();
   const auto& innerbend = innerstub_.getBend();
     
   auto ptinnerindex = (idphi, innerbend);
   auto ptouterindex = (idphi, outerbend);
   
   ap_uint<1> savestub = good&&inrange && ptinnerLUT[ptinnerindex] && ptouterLUT[ptouterindex] && rzcut;
   //if (good&&inrange && ptinnerLUT[ptinnerindex] && ptouterLUT[ptouterindex] && rzcut){
   //  write( (outervmstub.getIndex(), innerstub_.raw()) );
   //}

   stubids_[writeindex_] = (outervmstub.getIndex(), innerstub_.raw());
   writeindex_+=savestub;


   return;

} // end step
 

 ap_uint<64> memstubs_;
 ap_uint<16> memmask_;
 ap_uint<16> maskmask_;
 //ap_uint<3> memindex_;

 ap_uint<3> slot_;
 ap_uint<3> rzbinfirst_;
 ap_uint<3> rzbindiffmax_;

 ap_uint<3> ireg_;
 ap_uint<1> next_;
 ap_uint<4> nstubs_;
 


 AllStubInner<BARRELPS> innerstub_; 

 INDEX writeindex_;
 INDEX readindex_;
 bool idle_;
 BXType bx_;
 
 NSTUBS istub_=0;
 STUBID stubids_[1<<TrackletEngineUnitBase::kNBitsBuffer];

 private:

}; // end class


#endif
