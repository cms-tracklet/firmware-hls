#ifndef TrackletAlgorithm_VMProjectionMemory_h
#define TrackletAlgorithm_VMProjectionMemory_h

#include "Constants.h"
#include "MemoryTemplate.h"

// VMProjectionBase is where we define the bit widths, which depend on the class template parameter.
template<int VMProjType> class VMProjectionBase {};

template<>
class VMProjectionBase<BARREL>
{
public:
  enum BitWidths {
    // Bit sizes for VMProjectionMemory fields
    kVMProjIsPSSeedSize = 1,
    kVMProjRinvSize = 5,
    kVMProjFinePhiSize = 3,
    kVMProjFineZSize = 4,
    kVMProjZBinSize = MEBinsBits+1,
    kVMProjIndexSize = 7,
    // Bit size for full VMProjectionMemory
    kVMProjectionSize = kVMProjIsPSSeedSize + kVMProjRinvSize + kVMProjFineZSize + kVMProjFinePhiSize + kVMProjZBinSize + kVMProjIndexSize
  };
};

template<>
class VMProjectionBase<DISK>
{
public:
  enum BitWidths {
    // Bit sizes for VMProjectionMemory fields
    kVMProjIsPSSeedSize = 0,
    kVMProjRinvSize = 5,
    kVMProjFinePhiSize = 3,
    kVMProjFineZSize = 4,
    kVMProjZBinSize = MEBinsBits+1+1,
    kVMProjIndexSize = 7,
    // Bit size for full VMProjectionMemory
    kVMProjectionSize = kVMProjIsPSSeedSize + kVMProjRinvSize + kVMProjFineZSize + kVMProjFinePhiSize + kVMProjZBinSize + kVMProjIndexSize
  };
};


// Data object definition
template<int VMProjType>
class VMProjection : public VMProjectionBase<VMProjType>
{
public:
  enum BitLocations {
    // The location of the least significant bit (LSB) and most significant bit (MSB) in the VMProjectionMemory word for different fields
    kVMProjIsPSSeedLSB = 0,
    kVMProjIsPSSeedMSB = kVMProjIsPSSeedLSB + VMProjectionBase<VMProjType>::kVMProjIsPSSeedSize - 1,
    
    kVMProjRinvLSB = kVMProjIsPSSeedMSB + 1,
    kVMProjRinvMSB = kVMProjRinvLSB + VMProjectionBase<VMProjType>::kVMProjRinvSize - 1,
    kVMProjFinePhiLSB = kVMProjRinvMSB + 1,
    kVMProjFinePhiMSB = kVMProjFinePhiLSB + VMProjectionBase<VMProjType>::kVMProjFinePhiSize - 1,
    kVMProjFineZLSB = kVMProjFinePhiMSB + 1,
    kVMProjFineZMSB = kVMProjFineZLSB + VMProjectionBase<VMProjType>::kVMProjFineZSize - 1,
    kVMProjZBinLSB = kVMProjFineZMSB + 1,
    kVMProjZBinMSB = kVMProjZBinLSB + VMProjectionBase<VMProjType>::kVMProjZBinSize - 1,
    kVMProjIndexLSB = kVMProjZBinMSB + 1,
    kVMProjIndexMSB = kVMProjIndexLSB + VMProjectionBase<VMProjType>::kVMProjIndexSize - 1
  };
  
  typedef ap_uint<VMProjectionBase<VMProjType>::kVMProjIndexSize> VMPID;
  typedef ap_uint<VMProjectionBase<VMProjType>::kVMProjZBinSize> VMPZBIN;
  typedef ap_uint<VMProjectionBase<VMProjType>::kVMProjFineZSize> VMPFINEZ;
  typedef ap_uint<VMProjectionBase<VMProjType>::kVMProjFinePhiSize> VMPFINEPHI;
  typedef ap_uint<VMProjectionBase<VMProjType>::kVMProjRinvSize> VMPRINV;
	
  typedef ap_uint<VMProjectionBase<VMProjType>::kVMProjectionSize> VMProjData;

  // Constructors
  VMProjection(const VMProjData& newdata):
    data_(newdata)
  {}

  // This constructor is only used for projections in BARREL
  VMProjection(const VMPID id, const VMPZBIN zbin, const VMPFINEZ finez, const VMPFINEPHI finephi, const VMPRINV rinv, const bool ps):
    data_( (id, zbin, finez, finephi, rinv, ps) )
  {
    static_assert(VMProjType == BARREL, "Constructor should only be used for BARREL projections");
  }

  // This constructor is only used for projections in DISK
  VMProjection(const VMPID id, const VMPZBIN zbin, const VMPFINEZ finez, const VMPFINEPHI finephi, const VMPRINV rinv):
    data_( (id, zbin ,finez , finephi, rinv) )
  {
    static_assert(VMProjType == DISK, "Constructor should only be used for DISK projections");
  }
  
  VMProjection():
    data_(0)
  {}

  #ifndef __SYNTHESIS__
  VMProjection(const char* datastr, int base=16)
  {
    VMProjData newdata(datastr, base);
    data_ = newdata;
  }
  #endif
  
  // Getter
  static constexpr int getWidth() {return VMProjectionBase<VMProjType>::kVMProjectionSize;}

  VMProjData raw() const {return data_;}
  
  VMPID getIndex() const {
    return data_.range(kVMProjIndexMSB,kVMProjIndexLSB);
  }

  VMPZBIN getZBin() const {
    return data_.range(kVMProjZBinMSB,kVMProjZBinLSB);
  }

  VMPFINEZ getFineZ() const {
    return data_.range(kVMProjFineZMSB,kVMProjFineZLSB);
  }

  VMPFINEPHI getFinePhi() const {
    return data_.range(kVMProjFinePhiMSB,kVMProjFinePhiLSB);
  }
  
  VMPRINV getRInv() const {
    return data_.range(kVMProjRinvMSB,kVMProjRinvLSB);
  }

  // This getter is only used for projections in BARREL
  bool getIsPSSeed() const {
    static_assert("VMProjType == BARREL", "Getter should only be used for BARREL projections");
    return data_.range(kVMProjIsPSSeedLSB,kVMProjIsPSSeedMSB);
  }
  
  // Setter
  void setIndex(const VMPID id) {
    data_.range(kVMProjIndexMSB,kVMProjIndexLSB) = id;
  }
  
  void setZBin(const VMPZBIN zbin) {
    data_.range(kVMProjZBinMSB,kVMProjZBinLSB) = zbin;
  }
  
  void setFineZ(const VMPFINEZ finez) {
    data_.range(kVMProjFineZMSB,kVMProjFineZLSB) = finez;
  }

  void setFinePhi(const VMPFINEPHI finephi) {
    data_.range(kVMProjFinePhiMSB,kVMProjFinePhiLSB) = finephi;
  }
  
  void setRInv(const VMPRINV rinv) {
    data_.range(kVMProjRinvMSB,kVMProjRinvLSB) = rinv;
  }
  
  // This setter is only used for projections in BARREL
  void setIsPSSeed(const bool psseed) {
    static_assert("VMProjType == BARREL", "Setter should only be used for BARREL projections");
    data_.range(kVMProjIsPSSeedLSB,kVMProjIsPSSeedMSB) = psseed;
  }
  
private:
  
  VMProjData data_;
  
};

// Memory definition
template<int VMProjType> using VMProjectionMemory = MemoryTemplate<VMProjection<VMProjType>, 1, kNBits_MemAddr>;
// FIXME: double check number of bits for bx and for memory address

#endif
