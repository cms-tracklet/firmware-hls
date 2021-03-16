#ifndef TrackletAlgorithm_VMStubMEMemoryCM_h
#define TrackletAlgorithm_VMStubMEMemoryCM_h

#include "Constants.h"
#include "MemoryTemplateBinned.h"

// VMStubMECMBase is where we define the bit widths, which depend on the class template parameter.
template<int VMSMEType> class VMStubMECMBase {};

template<>
class VMStubMECMBase<BARRELPS>
{
public:
  enum BitWidths {
    // Bit sizes for VMStubMEMemoryCM fields
    kVMSMEFineZSize = 3,
    kVMSMEBendSize = 3,
    kVMSMEIDSize = 7,
    // Bit size for full VMStubMEMemoryCM
    kVMStubMECMSize = kVMSMEFineZSize + kVMSMEBendSize + kVMSMEIDSize
  };
};

template<>
class VMStubMECMBase<BARREL2S>
{
public:
  enum BitWidths {
    // Bit sizes for VMStubMEMemoryCM fields
    kVMSMEFineZSize = 3,
    kVMSMEBendSize = 4,
    kVMSMEIDSize = 7,
    // Bit size for full VMStubMEMemoryCM
    kVMStubMECMSize = kVMSMEFineZSize + kVMSMEBendSize + kVMSMEIDSize
  };
};

template<>
class VMStubMECMBase<DISK>
{
public:
  enum BitWidths {
    // Bit sizes for VMStubMEMemoryCM fields
    kVMSMEFineZSize = 3,
    kVMSMEBendSize = 3,
    kVMSMEIDSize = 7,
    // Bit size for full VMStubMEMemoryCM
    kVMStubMECMSize = kVMSMEFineZSize + kVMSMEBendSize + kVMSMEIDSize
  };
};

// Data object definition
template<int VMSMEType>
class VMStubMECM : public VMStubMECMBase<VMSMEType>
{
public:
  enum BitLocations {
    // The location of the least significant bit (LSB) and most significant bit (MSB) in the VMStubMEMemoryCM word for different fields
    kVMSMEFineZLSB = 0,
    kVMSMEFineZMSB = kVMSMEFineZLSB + VMStubMECMBase<VMSMEType>::kVMSMEFineZSize - 1,
    kVMSMEBendLSB = kVMSMEFineZMSB + 1,
    kVMSMEBendMSB = kVMSMEBendLSB + VMStubMECMBase<VMSMEType>::kVMSMEBendSize - 1,
    kVMSMEIDLSB = kVMSMEBendMSB + 1,
    kVMSMEIDMSB = kVMSMEIDLSB + VMStubMECMBase<VMSMEType>::kVMSMEIDSize - 1
  };

  typedef ap_uint<VMStubMECMBase<VMSMEType>::kVMSMEIDSize> VMSMEID;
  typedef ap_uint<VMStubMECMBase<VMSMEType>::kVMSMEBendSize> VMSMEBEND;
  typedef ap_uint<VMStubMECMBase<VMSMEType>::kVMSMEFineZSize> VMSMEFINEZ;

  typedef ap_uint<VMStubMECMBase<VMSMEType>::kVMStubMECMSize> VMStubMECMData;

  // Constructors
  VMStubMECM(const VMStubMECMData& newdata):
    data_(newdata)
  {}

  VMStubMECM(const VMSMEID id, const VMSMEBEND bend, const VMSMEFINEZ finez):
    data_( ((id,bend),finez) )
  {}

  VMStubMECM():
    data_(0)
  {}

  #ifndef __SYNTHESIS__
  VMStubMECM(const char* datastr, int base=16)
  {
    VMStubMECMData newdata(datastr, base);
    data_ = newdata;
  }
  #endif

  // Getter
  static constexpr int getWidth() {return VMStubMECMBase<VMSMEType>::kVMStubMECMSize;}
  
  VMStubMECMData raw() const {return data_;}

  VMSMEID getIndex() const {
    return data_.range(kVMSMEIDMSB,kVMSMEIDLSB);
  }

  VMSMEBEND getBend() const {
    return data_.range(kVMSMEBendMSB,kVMSMEBendLSB);
  }

  VMSMEFINEZ getFineZ() const {
    return data_.range(kVMSMEFineZMSB,kVMSMEFineZLSB);
  }

  // Setter
  void setIndex(const VMSMEID id) {
    data_.range(kVMSMEIDMSB,kVMSMEIDLSB) = id;
  }

  void setBend(const VMSMEBEND bend) {
    data_.range(kVMSMEBendMSB,kVMSMEBendLSB) = bend;
  }

  void setFineZ(const VMSMEFINEZ finez) {
    data_.range(kVMSMEFineZMSB,kVMSMEFineZLSB) = finez;
  }

private:

  VMStubMECMData data_;

};

// Memory definition
template<int VMSMEType, int RZSize, int PhiRegSize > using VMStubMEMemoryCM = MemoryTemplateBinned<VMStubMECM<VMSMEType>, 1, 4+RZSize+PhiRegSize, RZSize+PhiRegSize>;

#endif
