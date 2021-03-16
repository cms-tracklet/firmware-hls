#ifndef TrackletAlgorithm_VMStubTEOuterMemory_h
#define TrackletAlgorithm_VMStubTEOuterMemory_h

#include "Constants.h"
#include "MemoryTemplateBinnedCM.h"

// VMStubTEOuterBase is where we define the bit widths, which depend on the class template parameter.
template<int VMSTEOType> class VMStubTEOuterBase {};

template<>
class VMStubTEOuterBase<BARRELPS>
{
public:
  enum BitWidths {
    // Bit sizes for VMStubTEOuterMemory fields
    kVMSTEOFineZSize = 3,
    kVMSTEOFinePhiSize = 3,
    kVMSTEOBendSize = 3,
    kVMSTEOIDSize = 7,
    // Bit size for full VMStubTEOuterMemory
    kVMStubTEOuterSize = kVMSTEOFineZSize + kVMSTEOFinePhiSize + kVMSTEOBendSize + kVMSTEOIDSize
  };
};

template<>
class VMStubTEOuterBase<BARREL2S>
{
public:
  enum BitWidths {
    // Bit sizes for VMStubTEOuterMemory fields
    kVMSTEOFineZSize = 3,
    kVMSTEOFinePhiSize = 3,
    kVMSTEOBendSize = 4,
    kVMSTEOIDSize = 7,
    // Bit size for full VMStubTEOuterMemory
    kVMStubTEOuterSize = kVMSTEOFineZSize + kVMSTEOFinePhiSize + kVMSTEOBendSize + kVMSTEOIDSize
  };
};

template<>
class VMStubTEOuterBase<DISK>
{
public:
  enum BitWidths {
    // Bit sizes for VMStubTEOuterMemory fields
    kVMSTEOFineZSize = 3,
    kVMSTEOFinePhiSize = 3,
    kVMSTEOBendSize = 3,
    kVMSTEOIDSize = 7,
    // Bit size for full VMStubTEOuterMemory
    kVMStubTEOuterSize = kVMSTEOFineZSize + kVMSTEOFinePhiSize + kVMSTEOBendSize + kVMSTEOIDSize
  };
};

// Data object definition
template<int VMSTEOType>
class VMStubTEOuter : public VMStubTEOuterBase<VMSTEOType>
{
public:
  enum BitLocations {
    // The location of the least significant bit (LSB) and most significant bit (MSB) in the VMStubTEOuterMemory word for different fields
    kVMSTEOFineZLSB = 0,
    kVMSTEOFineZMSB = kVMSTEOFineZLSB + VMStubTEOuterBase<VMSTEOType>::kVMSTEOFineZSize - 1,
    kVMSTEOFinePhiLSB = kVMSTEOFineZMSB + 1,
    kVMSTEOFinePhiMSB = kVMSTEOFinePhiLSB + VMStubTEOuterBase<VMSTEOType>::kVMSTEOFinePhiSize - 1,
    kVMSTEOBendLSB = kVMSTEOFinePhiMSB + 1,
    kVMSTEOBendMSB = kVMSTEOBendLSB + VMStubTEOuterBase<VMSTEOType>::kVMSTEOBendSize - 1,
    kVMSTEOIDLSB = kVMSTEOBendMSB + 1,
    kVMSTEOIDMSB = kVMSTEOIDLSB + VMStubTEOuterBase<VMSTEOType>::kVMSTEOIDSize - 1
  };

  typedef ap_uint<VMStubTEOuterBase<VMSTEOType>::kVMSTEOIDSize> VMSTEOID;
  typedef ap_uint<VMStubTEOuterBase<VMSTEOType>::kVMSTEOBendSize> VMSTEOBEND;
  typedef ap_uint<VMStubTEOuterBase<VMSTEOType>::kVMSTEOFinePhiSize> VMSTEOFINEPHI;
  typedef ap_uint<VMStubTEOuterBase<VMSTEOType>::kVMSTEOFineZSize> VMSTEOFINEZ;

  typedef ap_uint<VMStubTEOuterBase<VMSTEOType>::kVMStubTEOuterSize> VMStubTEOuterData;

  // Constructors
  VMStubTEOuter(const VMStubTEOuterData& newdata):
    data_(newdata)
  {}

  VMStubTEOuter(const VMSTEOID id, const VMSTEOBEND bend, const VMSTEOFINEPHI finephi, const VMSTEOFINEZ finez):
    data_( (((id,bend),finephi),finez) )
  {}

  VMStubTEOuter():
    data_(0)
  {}

  #ifndef __SYNTHESIS__
  VMStubTEOuter(const char* datastr, int base=16)
  {
    VMStubTEOuterData newdata(datastr, base);
    data_ = newdata;
  }
  #endif

  // Getter
  static constexpr int getWidth() {return VMStubTEOuterBase<VMSTEOType>::kVMStubTEOuterSize;}
  
  VMStubTEOuterData raw() const {return data_;}

  VMSTEOID getIndex() const {
    return data_.range(kVMSTEOIDMSB,kVMSTEOIDLSB);
  }

  VMSTEOBEND getBend() const {
    return data_.range(kVMSTEOBendMSB,kVMSTEOBendLSB);
  }

  VMSTEOFINEPHI getFinePhi() const {
    return data_.range(kVMSTEOFinePhiMSB,kVMSTEOFinePhiLSB);
  }

  VMSTEOFINEZ getFineZ() const {
    return data_.range(kVMSTEOFineZMSB,kVMSTEOFineZLSB);
  }

  // Setter
  void setIndex(const VMSTEOID id) {
    data_.range(kVMSTEOIDMSB,kVMSTEOIDLSB) = id;
  }

  void setBend(const VMSTEOBEND bend) {
    data_.range(kVMSTEOBendMSB,kVMSTEOBendLSB) = bend;
  }

  void setFinePhi(const VMSTEOFINEPHI finephi) {
    data_.range(kVMSTEOFinePhiMSB,kVMSTEOFinePhiLSB) = finephi;
  }

  void setFineZ(const VMSTEOFINEZ finez) {
    data_.range(kVMSTEOFineZMSB,kVMSTEOFineZLSB) = finez;
  }

private:

  VMStubTEOuterData data_;

};

// Memory definition
template<int VMSTEOType, int RZSize, int PhiRegSize, unsigned int NCOPY > using VMStubTEOuterMemoryCM = MemoryTemplateBinnedCM<VMStubTEOuter<VMSTEOType>, 1, 4+RZSize+PhiRegSize, RZSize+PhiRegSize, NCOPY>;

#endif
