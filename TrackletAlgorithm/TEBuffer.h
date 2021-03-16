#ifndef TrackletAlgorithm_TEBuffer_h
#define TrackletAlgorithm_TEBuffer_h

#include "Constants.h"
#include "MemoryTemplate.h"
#include "AllStubInnerMemory.h"
#include "TrackletEngineUnit.h"

class TEData {

 public:

  enum BitWidths {
    kNBitsIMem = 2
  };

  enum BitLocations {
    // The location of the least significant bit (LSB) and most significant bit (MSB) in the ProjectionRouterBufferMemory word for different fields
    kTEDataStubMaskLSB = 0,
    kTEDataStubMaskMSB = kTEDataStubMaskLSB + 2*(1<<TrackletEngineUnit<BARRELPS>::kNBitsPhiBins) - 1,
    kTEDatarzbinfirstLSB = kTEDataStubMaskMSB+1,
    kTEDatarzbinfirstMSB = kTEDatarzbinfirstLSB + TrackletEngineUnit<BARRELPS>::kNBitsRZFine - 1,
    kTEDatastartLSB = kTEDatarzbinfirstMSB + 1,
    kTEDatastartMSB = kTEDatastartLSB + TrackletEngineUnit<BARRELPS>::kNBitsRZBin - 1,
    kTEDatarzdiffmaxLSB = kTEDatastartMSB + 1,
    kTEDatarzdiffmaxMSB = kTEDatarzdiffmaxLSB + TrackletEngineUnit<BARRELPS>::kNBitsRZFine - 1,
    kTEDataAllStubLSB = kTEDatarzdiffmaxMSB + 1,
    kTEDataAllStubMSB = kTEDataAllStubLSB + AllStubInner<BARRELPS>::kAllStubInnerSize - 1
  };

  typedef ap_uint<kNBitsIMem> IMEM;
  typedef ap_uint<kTEDataAllStubMSB+1> TEDATA;

 TEData():
  data_(0)
    {}

  AllStubInner<BARRELPS>::AllStubInnerData getAllStub() const {
    return data_.range(kTEDataAllStubMSB,kTEDataAllStubLSB);
  }

  TrackletEngineUnit<BARRELPS>::RZBIN getStart() const {
    return data_.range(kTEDatastartMSB,kTEDatastartLSB);
  }

  TrackletEngineUnit<BARRELPS>::RZFINE getrzbinfirst() const {
    return data_.range(kTEDatarzbinfirstMSB,kTEDatarzbinfirstLSB);
  }

  TrackletEngineUnit<BARRELPS>::RZFINE getrzdiffmax() const {
    return data_.range(kTEDatarzdiffmaxMSB,kTEDatarzdiffmaxLSB);
  }

  const TrackletEngineUnit<BARRELPS>::MEMMASK getStubMask() const {
    return data_.range(kTEDataStubMaskMSB,kTEDataStubMaskLSB);
  }

 TEData(const TEData& tedata):
  data_(tedata.data_)
  {}

 TEData(const TEDATA& tedata):
  data_(tedata)
  {}
  
 TEData( const TrackletEngineUnit<BARRELPS>::MEMMASK stubmask,
	 const TrackletEngineUnit<BARRELPS>::RZFINE rzbinfirst, 
	 const TrackletEngineUnit<BARRELPS>::RZBIN start, 
	 const TrackletEngineUnit<BARRELPS>::RZFINE rzdiffmax, 
	 const AllStubInner<BARRELPS>::AllStubInnerData stub
	 ):
  data_( (stub, rzdiffmax, start, rzbinfirst, stubmask) )
    {}

  TEDATA raw() const {return data_;}
  
 private:

  TEDATA data_;

};
  

// Data object definition
class TEBuffer {

 public:

  enum BitWidths {
    kNBufferDepthBits=3
  };

  typedef ap_uint<kNBufferDepthBits> TEBUFFERINDEX;
  typedef ap_uint<kNBits_MemAddr> NSTUBS;

  // Constructors
 TEBuffer():
  writeptr_(0), readptr_(0), istub_(0), imem_(0), imembegin_(0), imemend_(0)
    {
#pragma HLS array_partition variable=buffer_ complete dim=0
}

  void setMemBegin(const TEData::IMEM& begin) {
    imembegin_=begin;
  }
  
  void setMemEnd(const TEData::IMEM& end) {
    imemend_=end;
  }

  void reset() {
    imem_=imembegin_;
    writeptr_=0;
    readptr_=0;
  }

  TEData::IMEM& getMem() {
    return imem_;
  }

  const TEData::IMEM& getMemEnd() {
    return imemend_;
  }

  NSTUBS& getIStub() {
    return istub_;
  }

  void setIStub(const NSTUBS& istub) {
    istub_=istub;
  }

  void store(const TEData::TEDATA& tedata) {
    auto writeptrtmp=writeptr_;
    buffer_[writeptrtmp]=tedata;
    writeptr_++;
  }

  TEData::TEDATA peek() const {
    return buffer_[readptr_];
  }

  void increment_readptr() {
    readptr_++;
  }

  bool full() const {
    TEBUFFERINDEX writeptrnext=writeptr_+1;
    return writeptrnext==readptr_;
  }

  bool empty() const {
    return writeptr_==readptr_;
  }

  TEBUFFERINDEX readptr() const {
    return readptr_;
  }

  TEBUFFERINDEX writeptr() const {
    return writeptr_;
  }

  TEBUFFERINDEX writeptr_, readptr_;

  NSTUBS istub_;
  TEData::IMEM imem_, imembegin_, imemend_;
  
  TEData::TEDATA buffer_[1<<kNBufferDepthBits];
  
private:

};

#endif
