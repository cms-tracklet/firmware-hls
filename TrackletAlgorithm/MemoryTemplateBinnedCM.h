// Class template for binned memory module
#ifndef TrackletAlgorithm_MemoryTemplateBinnedCM_h
#define TrackletAlgorithm_MemoryTemplateBinnedCM_h

#ifndef __SYNTHESIS__
#include <iostream>
#include <sstream>
#include <vector>
#endif

template<class DataType, unsigned int NBIT_BX, unsigned int NBIT_ADDR,
  unsigned int NBIT_BIN, unsigned int NCOPY>
// DataType: type of data object stored in the array
// NBIT_BX: number of bits for BX;
// (1<<NBIT_BIN): number of BXs the memory is keeping track of
// NBIT_ADDR: number of bits for memory address space per BX
// (1<<NBIT_ADDR): depth of the memory for each BX
// NBIT_BIN: number of bits used for binning; (1<<NBIT_BIN): number of bins
// NCOPY: is the number of memory copies - same as numbr of TE used
class MemoryTemplateBinnedCM{

 public: 
  typedef ap_uint<NBIT_BX> BunchXingT;
  typedef ap_uint<NBIT_ADDR-NBIT_BIN> NEntryT;
  
 protected:
  enum BitWidths {
    kNBxBins = 1<<NBIT_BX,
    kNSlots = 1<<NBIT_BIN,
    kNMemDepth = 1<<NBIT_ADDR
  };

  DataType dataarray_[NCOPY][kNBxBins][kNMemDepth];  // data array

  ap_uint<8> binmask8_[kNBxBins][8];
  ap_uint<32> nentries8_[kNBxBins][8];

  
 public:

  MemoryTemplateBinnedCM() {
    clear();
  }
  
  ~MemoryTemplateBinnedCM() {}
  
  void clear() {
#pragma HLS inline
	
  clearloop1:for (size_t ibx=0; ibx<(kNBxBins); ++ibx) {
#pragma HLS UNROLL
      clear(ibx);
    }
  }

  void clear(BunchXingT bx) {
#pragma HLS inline
	
  clearloop2:for (unsigned int ibin = 0; ibin < 8; ++ibin) {
#pragma HLS UNROLL
      nentries8_[bx][ibin] = 0;
      binmask8_[bx][ibin] = 0;
    }
  }

  unsigned int getDepth() const {return kNMemDepth;}
  unsigned int getNBX() const {return kNBxBins;}
  unsigned int getNBins() const {return kNSlots;}

  NEntryT getEntries(BunchXingT bx, ap_uint<NBIT_BIN> slot) const {
    ap_uint<3> ibin,ireg;
    (ibin,ireg)=slot;
    return nentries8_[bx][ibin].range(ireg*4+3,ireg*4);
  }

  ap_uint<32> getEntries8(BunchXingT bx, ap_uint<3> ibin) const {
    #pragma HLS ARRAY_PARTITION variable=nentries8_ complete dim=0
    return nentries8_[bx][ibin];
  }

  ap_uint<8> getBinMask8(BunchXingT bx, ap_uint<3> ibin) const {
    #pragma HLS ARRAY_PARTITION variable=binmask8_ complete dim=0
    return binmask8_[bx][ibin];
  }

  NEntryT getEntries(BunchXingT bx) const {
    NEntryT val = 0;
    for ( auto i = 0; i < getDepth(); ++i ) {
      val += getEntries(bx, i);
    }
    return val;
  }


  const DataType (&get_mem() const)[NCOPY][1<<NBIT_BX][1<<NBIT_ADDR] {
    return dataarray_;
  }

  DataType read_mem(unsigned int icopy, BunchXingT ibx, ap_uint<NBIT_ADDR> index) const {
#pragma HLS ARRAY_PARTITION variable=dataarray_ dim=1
    // TODO: check if valid
    return dataarray_[icopy][ibx][index];
  }
  
  DataType read_mem(unsigned int icopy, BunchXingT ibx, ap_uint<NBIT_BIN> slot,
		    ap_uint<NBIT_ADDR> index) const {
#pragma HLS ARRAY_PARTITION variable=dataarray_ dim=1
    // TODO: check if valid
    return dataarray_[icopy][ibx][(1<<(NBIT_ADDR-NBIT_BIN))*slot+index];
  }
  
  bool write_mem(BunchXingT ibx, ap_uint<NBIT_BIN> slot, DataType data) {
#pragma HLS ARRAY_PARTITION variable=dataarray_ dim=1
#pragma HLS inline

    ap_uint<3> ibin,ireg;    
    (ibin,ireg)=slot;

    NEntryT nentry_ibx = nentries8_[ibx][ibin].range(ireg*4+3,ireg*4);

    if (nentry_ibx < (1<<(NBIT_ADDR-NBIT_BIN))) {
      // write address for slot: 1<<(NBIT_ADDR-NBIT_BIN) * slot + nentry_ibx
  
    writememloop:for (unsigned int icopy=0;icopy<NCOPY;icopy++) {
#pragma HLS unroll
	dataarray_[icopy][ibx][(1<<(NBIT_ADDR-NBIT_BIN))*slot+nentry_ibx] = data;
      }

      binmask8_[ibx][ibin].set_bit(ireg,true);
      
      nentries8_[ibx][ibin].range(ireg*4+3,ireg*4)=nentry_ibx+1;
      
      return true;
    }
    else {
#ifndef __SYNTHESIS__
      std::cout << "Warning out of range. nentry_ibx = "<<nentry_ibx<<" NBIT_ADDR-NBIT_BIN = "<<NBIT_ADDR-NBIT_BIN << std::endl;
#endif
      return false;
    }
  }
  

  // Methods for C simulation only
#ifndef __SYNTHESIS__
  
  ///////////////////////////////////
  std::vector<std::string> split(const std::string& s, char delimiter)
  {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream sstream(s);
    while (getline(sstream, token, delimiter))
      {
	tokens.push_back(token);
      }
    return tokens;
  }

  // write memory from text file
  bool write_mem(BunchXingT bx, const std::string& line, int base=16)
  {

    std::string datastr = split(line, ' ').back();

    int slot = (int)strtol(split(line, ' ').front().c_str(), nullptr, base); // Convert string (in hexadecimal) to int
    // Originally: atoi(split(line, ' ').front().c_str()); but that didn't work for disks with 16 bins

    //change order HACK...
    ap_uint<3> ireg,bin;
    (ireg,bin)=ap_uint<6>(slot);
    int newslot=(bin,ireg);

    DataType data(datastr.c_str(), base);
    return write_mem(bx, newslot, data);
  }


  // print memory contents
  void print_data(const DataType data) const
  {
	std::cout << std::hex << data.raw() << std::endl;
	// TODO: overload '<<' in data class
  }

  void print_entry(BunchXingT bx, ap_uint<NBIT_ADDR> index) const
  {
	print_data(dataarray_[bx][index]);
  }

  void print_mem(BunchXingT bx) const
  {
	for(int slot=0;slot<8;slot++) {
      //std::cout << "slot "<<slot<<" entries "
      //		<<nentries_[bx%NBX].range((slot+1)*4-1,slot*4)<<endl;
      for (int i = 0; i < nentries8_[bx][slot]; ++i) {
		std::cout << bx << " " << i << " ";
		print_entry(bx, i + slot*(1<<(NBIT_ADDR-NBIT_BIN)) );
      }
    }
  }

  void print_mem() const
  {
	for (int ibx = 0; ibx < kNBxBins; ++ibx) {
	  for (int i = 0; i < 8; ++i) {
		std::cout << ibx << " " << i << " ";
		print_entry(ibx,i);
	  }
	}
  }

  static constexpr int getWidth() {return DataType::getWidth();}
  
#endif
  
};

#endif
