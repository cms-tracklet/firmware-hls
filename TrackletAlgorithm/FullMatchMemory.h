#ifndef TrackletAlgorithm_FullMatchMemory_h
#define TrackletAlgorithm_FullMatchMemory_h

#include "Constants.h"
#include "MemoryTemplate.h"

// FullMatchBase is where we define the bit widths, which depend on the class template parameter.
template<int FMType> class FullMatchBase {};

template<>
class FullMatchBase<BARREL>
{
public:
  enum BitWidths {
    // Bit size for FullMatchMemory fields
    kFMZResSize = 9,
    kFMPhiResSize = 12,
    kFMStubRSize = 7,
    kFMStubIndexSize = 10,
    kFMStubPhiIDSize = 3,   // subdivision of StubIndex
    kFMStubIDSize = 7,      // subdivision of StubIndex
    kFMTrackletIndexSize = 7,
    kFMTCIDSize = 7,
    kFMSeedSize = 3, // upper three bits of TCID
    kFMITCSize = 4, // lower four bits of TCID
    // Bit size for full FullMatchMemory
    kFullMatchSize = kFMTCIDSize + kFMTrackletIndexSize + kFMStubIndexSize + kFMStubRSize + kFMPhiResSize + kFMZResSize
  };
};

template<>
class FullMatchBase<DISK>
{
public:
  enum BitWidths {
    // Bit size for FullMatchMemory fields
    kFMZResSize = 7,
    kFMPhiResSize = 12,
    kFMStubRSize = 12,
    kFMStubIndexSize = 10,
    kFMStubPhiIDSize = 3,   // subdivision of StubIndex
    kFMStubIDSize = 7,      // subdivision of StubIndex
    kFMTrackletIndexSize = 7,
    kFMTCIDSize = 7,
    kFMSeedSize = 3, // upper three bits of TCID
    kFMITCSize = 4, // lower four bits of TCID
    // Bit size for full FullMatchMemory
    kFullMatchSize = kFMTCIDSize + kFMTrackletIndexSize + kFMStubIndexSize + kFMStubRSize + kFMPhiResSize + kFMZResSize
  };
};

template<>
class FullMatchBase<BARREL_FOR_MC>
{
public:
  enum BitWidths {
    // Bit size for FullMatchMemory fields
    kFMZResSize = 9,
    kFMPhiResSize = 12,
    kFMStubRSize = 0, // this field is currently missing in the test-vectors for the MC
    kFMStubIndexSize = 10,
    kFMStubPhiIDSize = 3,   // subdivision of StubIndex
    kFMStubIDSize = 7,      // subdivision of StubIndex
    kFMTrackletIndexSize = 7,
    kFMTCIDSize = 7,
    kFMSeedSize = 3, // upper three bits of TCID
    kFMITCSize = 4, // lower four bits of TCID
    // Bit size for full FullMatchMemory
    kFullMatchSize = kFMTCIDSize + kFMTrackletIndexSize + kFMStubIndexSize + kFMStubRSize + kFMPhiResSize + kFMZResSize
  };
};

template<>
class FullMatchBase<DISK_FOR_MC>
{
public:
  enum BitWidths {
    // Bit size for FullMatchMemory fields
    kFMZResSize = 7,
    kFMPhiResSize = 12,
    kFMStubRSize = 0, // this field is currently missing in the test-vectors for the MC
    kFMStubIndexSize = 10,
    kFMStubPhiIDSize = 3,   // subdivision of StubIndex
    kFMStubIDSize = 7,      // subdivision of StubIndex
    kFMTrackletIndexSize = 7,
    kFMTCIDSize = 7,
    kFMSeedSize = 3, // upper three bits of TCID
    kFMITCSize = 4, // lower four bits of TCID
    // Bit size for full FullMatchMemory
    kFullMatchSize = kFMTCIDSize + kFMTrackletIndexSize + kFMStubIndexSize + kFMStubRSize + kFMPhiResSize + kFMZResSize
  };
};

// Data object definition
template<int FMType>
class FullMatch : public FullMatchBase<FMType>
{
public:
  enum BitLocations {
    // The location of the least significant bit (LSB) and most significant bit (MSB) in the FullMatchMemory word for different fields
    kFMZResLSB = 0,
    kFMZResMSB = kFMZResLSB + FullMatchBase<FMType>::kFMZResSize - 1,
    kFMPhiResLSB = kFMZResMSB + 1,
    kFMPhiResMSB = kFMPhiResLSB + FullMatchBase<FMType>::kFMPhiResSize - 1,
    kFMStubRLSB = kFMPhiResMSB + 1,
    kFMStubRMSB = kFMStubRLSB + FullMatchBase<FMType>::kFMStubRSize - 1,
    kFMStubIndexLSB = kFMStubRMSB + 1,
    kFMStubIndexMSB = kFMStubIndexLSB + FullMatchBase<FMType>::kFMStubIndexSize - 1,
    kFMStubIDLSB = kFMStubRMSB + 1,
    kFMStubIDMSB = kFMStubIDLSB + FullMatchBase<FMType>::kFMStubIDSize - 1,
    kFMStubPhiIDLSB = kFMStubIDMSB + 1,
    kFMStubPhiIDMSB = kFMStubPhiIDLSB + FullMatchBase<FMType>::kFMStubPhiIDSize - 1,
    kFMTrackletIndexLSB = kFMStubIndexMSB + 1,
    kFMTrackletIndexMSB = kFMTrackletIndexLSB + FullMatchBase<FMType>::kFMTrackletIndexSize - 1,
    kFMTCIDLSB = kFMTrackletIndexMSB + 1,
    kFMTCIDMSB = kFMTCIDLSB + FullMatchBase<FMType>::kFMTCIDSize - 1
  };

  typedef ap_int<FullMatchBase<FMType>::kFMZResSize> FMZRES;
  typedef ap_int<FullMatchBase<FMType>::kFMPhiResSize> FMPHIRES;
  typedef ap_uint<FullMatchBase<FMType>::kFMStubRSize> FMSTUBR;
  typedef ap_uint<FullMatchBase<FMType>::kFMStubIndexSize> FMSTUBINDEX;
  typedef ap_uint<FullMatchBase<FMType>::kFMStubIDSize> FMSTUBID;        // subdivision of StubIndex 
  typedef ap_uint<FullMatchBase<FMType>::kFMStubPhiIDSize> FMSTUBPHIID;  // subdivision of StubIndex
  typedef ap_uint<FullMatchBase<FMType>::kFMTrackletIndexSize> FMTrackletIndex;
  typedef ap_uint<FullMatchBase<FMType>::kFMTCIDSize> FMTCID;
  typedef ap_uint<FullMatchBase<FMType>::kFMTCIDSize + FullMatchBase<FMType>::kFMTrackletIndexSize> FMTrackletID;

  typedef ap_uint<FullMatchBase<FMType>::kFullMatchSize> FullMatchData;

  // Constructors
  FullMatch(const FullMatchData& newdata):
    data_(newdata)
  {}

  FullMatch(const FMTCID tcid, const FMTrackletIndex trackletindex, const FMSTUBINDEX stub, const FMSTUBR stubr, const FMPHIRES phires, const FMZRES zres):
    data_( (((((tcid,trackletindex),stub),stubr),phires),zres) )
  {}

  FullMatch(const FMTCID tcid, const FMTrackletIndex trackletindex, const FMSTUBPHIID stubphiid, const FMSTUBID stubid, const FMSTUBR stubr, const FMPHIRES phires, const FMZRES zres):
	data_( ((((((tcid,trackletindex),stubphiid),stubid),stubr),phires),zres) )
  {}

  FullMatch():
    data_(0)
  {}

  #ifndef __SYNTHESIS__
  FullMatch(const char* datastr, int base = 16)
  {
    FullMatchData newdata(datastr, base);
    data_ = newdata;
  }
  #endif

  // Getter
  static constexpr int getWidth() {return FullMatchBase<FMType>::kFullMatchSize;}

  FullMatchData raw() const {return data_;}

  // TCID is a unique identifier assigned to each TC. It is a concatenation of
  // the seed and iTC numbers:
  // TCID = (seed<<4) + iTC
  // The seed number is assigned as follows:
  //   L1L2 -> 0
  //   L2L3 -> 1
  //   L3L4 -> 2
  //   L5L6 -> 3
  //   D1D2 -> 4
  //   D3D4 -> 5
  //   D1L1/L1D1 -> 6
  //   D1L2/L2D1 -> 7
  // The iTC number comes from the letter at the end of the TC name (i.e.,
  // TC_L1L2A and TC_L5L6A have the same iTC number); generally indicates the
  // region of the phi sector being processed:
  //   A -> 0
  //   B -> 1
  //   C -> 2
  //   ...
  FMTCID getTCID() const {
    return data_.range(kFMTCIDMSB,kFMTCIDLSB);
  }

  FMTrackletIndex getTrackletIndex() const {
    return data_.range(kFMTrackletIndexMSB,kFMTrackletIndexLSB);
  }

  FMTrackletID getTrackletID() const {
    return data_.range(kFMTCIDMSB,kFMTrackletIndexLSB);
  }

  FMSTUBINDEX getStubIndex() const {
    return data_.range(kFMStubIndexMSB,kFMStubIndexLSB);
  }

  FMSTUBID getStubID() const {
    return data_.range(kFMStubIDMSB,kFMStubIDLSB);
  }

  FMSTUBPHIID getStubPhiID() const {
    return data_.range(kFMStubPhiIDMSB,kFMStubPhiIDLSB);
  }

  FMSTUBR getStubR() const {
    return data_.range(kFMStubRMSB,kFMStubRLSB);
  }

  FMPHIRES getPhiRes() const {
    return data_.range(kFMPhiResMSB,kFMPhiResLSB);
  }

  FMZRES getZRes() const {
    return data_.range(kFMZResMSB,kFMZResLSB);
  }

  // Setter
  void setTCID(const FMTCID tcid) {
    data_.range(kFMTCIDMSB,kFMTCIDLSB) = tcid;
  }

  void setTrackletIndex(const FMTrackletIndex trackletindex) {
    data_.range(kFMTrackletIndexMSB,kFMTrackletIndexLSB) = trackletindex;
  }

  void setStubIndex(const FMSTUBINDEX stid) {
    data_.range(kFMStubIndexMSB,kFMStubIndexLSB) = stid;
  }

  void setStubPhiID(const FMSTUBPHIID stphiid) {
    data_.range(kFMStubPhiIDMSB,kFMStubPhiIDLSB) = stphiid;
  }

  void setStubID(const FMSTUBID stid) {
    data_.range(kFMStubIDMSB,kFMStubIDLSB) = stid;
  }

  void setStubR(const FMSTUBR stubr) {
    data_.range(kFMStubRMSB,kFMStubRLSB) = stubr;
  }

  void setPhiRes(const FMPHIRES phires) {
    data_.range(kFMPhiResMSB,kFMPhiResLSB) = phires;
  }

  void setZRes(const FMZRES zres) {
    data_.range(kFMZResMSB,kFMZResLSB) = zres;
  }

private:

  FullMatchData data_;

};

// Memory definition
template<int FMType> using FullMatchMemory = MemoryTemplate<FullMatch<FMType>, 1, kNBits_MemAddr>;

#endif
