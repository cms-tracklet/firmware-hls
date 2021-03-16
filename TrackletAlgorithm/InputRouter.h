#ifndef TrackletAlgorithm_InputRouter_h
#define TrackletAlgorithm_InputRouter_h


#include <cassert>
#include "Constants.h"
#include "AllStubMemory.h"
#include "DTCStubMemory.h"


// link map
static const int kNBitsLnkDsc = 4; 
static const int kNBitsNLnks = 6; 
static const int kLINKMAPwidth = 20;
static const int kSizeLinkWord = 4; 
static const int kBINMAPwidth = 12; 
static const int kSizeBinWord =  3; 
static const int kNMEMwidth = 5;
#ifndef __SYNTHESIS__
	#include <bitset> 
#endif



// // maximum number of IR memories 
// constexpr unsigned int kNIRMemories = 20;

// mxm number of coarse phi bins 
constexpr int kMaxPhiBnsPrLyr = 8; 

// mxmium number of layers readout by a DTC 
static const int kMaxLyrsPerDTC = 4; 

// Number of MSBs used for r index in phicorr LUTs
static const int kNBitsPhiCorrTableR = 3; // Found hardcoded in VMRouterphicorrtable.h
// size of link LUT 
static const int kSizeLinkTable = 12;
// size of phi correction table 
// for barrel PS
static const int kSizePhiCorrTablePS = 64;
// size of phi correction table 
// for barrel 2S
static const int kSizePhiCorrTable2S = 128;
// number of phi bins IR sorts 
// stubs into for first PS barrel 
static const int kNbitsPhiBinsPSL1 = 3; 
// number of phi bins IR sorts 
// stubs into for all regions of 
// the tracker 
// except for PS L1
static const int kNbitsPhiBinsTkr = 2; 

// typedefs
// encoded DTC layer id 
static const unsigned int kNBitsDTCLyrEnc=3;
typedef ap_uint<kNBitsDTCLyrEnc> EncodedDTCLyrId ;
// raw trakcker layer id 
static const unsigned int kNBitsLyrTk=3; 
typedef ap_uint<kNBitsLyrTk> TkLyrId ;
// barrel bit 
static const unsigned int kNBitsBrlBit=1;
typedef ap_uint<kNBitsBrlBit> BrlBit; 
//
static const unsigned int kNBitsPhi=3; 
typedef ap_uint<kNBitsPhi> LyrPhiBn; 
// link word 
typedef ap_uint<kSizeLinkWord> LnkWrd;

// 
static const int kNBrlLyrs = 3;
static const int kFrstPSBrlLyr = 1; 
static const int kScndPSBrlLyr = 2;
static const int kThrdPSBrlLyr = 3; 
static const int kFrst2SBrlLyr = 4; 
static const int kScnd2SBrlLyr = 5; 
static const int kThrd2SBrlLyr = 6; 
// valid bit in DTC stub 
// these can change when tarball is modified 
static const int kVldBitSize = 1 ; 
static const int kLSBVldBt = 0;
static const int kMSBVldBt = 0;
// lyr bit in DTC stub 
// these can change when tarball is modified 
static const int kLyBitsSize = 2; 
static const int kLSBLyrBts = 1;
static const int kMSBLyrBts = 2;

constexpr unsigned int kMaxNmemories = 20;  
#define IR_DEBUG false

// Get the corrected phi, bin
// i.e. phi at the average radius of the barrel
template<regionType InType>
void getPhiBinWithCorrection(ap_uint<kBRAMwidth> hStbWrd
		, const int phicorrtable[]
		, const int cNbits 
		, int &phiBn ) 
{
	#pragma HLS inline
  	#pragma HLS array_partition variable = phicorrtable complete

  	AllStub<InType> hAStub(hStbWrd);
  	auto hPhi = hAStub.getPhi();
  	auto hR =  hAStub.getR();
  	auto hBend = hAStub.getBend();

	constexpr auto rbins = 1 << kNBitsPhiCorrTableR; // The number of bins for r
	ap_uint<kNBitsPhiCorrTableR> rbin = (hR + (1 << (AllStubBase<InType>::kASRSize - 1)))
			>> (AllStubBase<InType>::kASRSize - kNBitsPhiCorrTableR); // Which bin r belongs to. Note r = 0 is mid radius
	auto index = hBend * rbins + rbin; // Index for where we find our correction value
	auto corrval = phicorrtable[index]; // The amount we need to correct our phi
 	
 	// internal storage of phi value 
	auto hPhiCorr = hPhi - corrval; // the corrected phi
	// Check for overflow
	if (hPhiCorr < 0)
		hPhiCorr = 0; // can't be less than 0
	if (hPhiCorr >= 1 << AllStubBase<InType>::kASPhiSize)
		hPhiCorr = (1 << AllStubBase<InType>::kASPhiSize) - 1;  // can't be more than the max value

	auto cBn = hPhiCorr.range(AllStubBase<InType>::kASPhiSize-1, AllStubBase<InType>::kASPhiSize-cNbits);
	phiBn = cBn;
}

// Get the phi bin
// no correction needed [i.e. for diskPS/disk2S]
template<regionType InType>
void getPhiBin( ap_uint<kBRAMwidth> hStbWrd
		, const int cNbits 
		, int &phiBn ) 
{
	#pragma HLS inline
  	
	AllStub<InType> hAStub(hStbWrd);
	//typename AllStub<InType>::ASPHI
	auto cPhi =  hAStub.getPhi();
	auto cBn = cPhi.range(AllStubBase<InType>::kASPhiSize-1, AllStubBase<InType>::kASPhiSize-cNbits);
	phiBn = cBn;
}

// clear internal counters 
template<unsigned int nEntriesSize>
void ClearCounters(unsigned int nMemories
	, ap_uint<kNBits_MemAddr> nEntries[nEntriesSize]) 
{
  #pragma HLS inline
  #pragma HLS array_partition variable = nEntries complete
  LOOP_ClearCounters:
	for (int cIndx = 0; cIndx < nEntriesSize ; cIndx++) 
  {
    #pragma HLS unroll
    nEntries[cIndx]=0; 
  }
}

// internal memory counter
template<unsigned int nLyrs>
void CountMemories(const ap_uint<kBINMAPwidth> hPhBnWord 
	, unsigned int &nMems
	, unsigned int nMemsPerLyr[nLyrs]) 
{
  #pragma HLS inline
  #pragma HLS array_partition variable = nMemsPerLyr complete
  int cPrevSize=0; 
  LOOP_CountOutputMemories:
  for (int cLyr = 0; cLyr < kMaxLyrsPerDTC ; cLyr++) 
  {
     #pragma HLS unroll
  	 auto hBnWrd = hPhBnWord.range(kSizeBinWord * cLyr + (kSizeBinWord-1), kSizeBinWord * cLyr);
     nMemsPerLyr[cLyr] = (cLyr < nLyrs) ?  (1+(int)(hBnWrd)) : 0 ; 
     nMems = (cLyr < nLyrs) ? (cPrevSize +  (1+(int)(hBnWrd))) : cPrevSize; 
     cPrevSize = nMems;
  }
}

// internal calculation 
// of memory index 
template<unsigned int nLyrs>
void GetMemoryIndex(unsigned int nMemsPerLyr[nLyrs]
	, ap_uint<kLyBitsSize> hEncLyr 
	, unsigned int &hIndx ) 
{
  #pragma HLS inline
  #pragma HLS array_partition variable = nMemsPerLyr complete
	// update index
	int cPrevIndx=0; 
	LOOP_UpdateMemIndx:
	for (int cLyr = 0; cLyr < nLyrs; cLyr++) 
	{
		#pragma HLS unroll
		int cUpdate = (cLyr < hEncLyr) ? nMemsPerLyr[cLyr] : 0; 
		hIndx = cPrevIndx + cUpdate; 
		cPrevIndx = hIndx;
	}
}

template<unsigned int nOMems, unsigned int nLUTEntries>
void InputRouter( const BXType bx
	, const ap_uint<kLINKMAPwidth> hLinkWord
	, const ap_uint<kBINMAPwidth> hPhBnWord 
	, const int hPhiCorrtable_L1[nLUTEntries]
	, const int hPhiCorrtable_L2[nLUTEntries]
	, const int hPhiCorrtable_L3[nLUTEntries]
	, ap_uint<kNBits_DTC>* hInputStubs
	, BXType & bx_o // output bx 
	, DTCStubMemory hOutputStubs[nOMems])
{
	
	#pragma HLS inline
	#pragma HLS interface ap_memory port = hPhiCorrtable_L1
  	#pragma HLS interface ap_memory port = hPhiCorrtable_L2
  	#pragma HLS interface ap_memory port = hPhiCorrtable_L3
  	#pragma HLS interface register port = bx_o
  
  	ap_uint<1> hIs2S= hLinkWord.range(kLINKMAPwidth-4,kLINKMAPwidth-4);

	// count memories 
	unsigned int nMems=0;
	unsigned int nMemsPerLyr[kMaxLyrsPerDTC]; 
	CountMemories<kMaxLyrsPerDTC>(hPhBnWord, nMems, nMemsPerLyr);
	// clear stub counters
	ap_uint<kNBits_MemAddr> hNStubs[nOMems];
	ClearCounters<nOMems>(nMems,  hNStubs);

	LOOP_ProcessIR:
	for (int cStubCounter = 0; cStubCounter < kMaxStubsFromLink; cStubCounter++) 
	{
	#pragma HLS pipeline II = 1
	#pragma HLS PIPELINE rewind
	  // decode stub
	  auto hStub = hInputStubs[cStubCounter];
	  // add check of valid bit here 
	  if (hStub == 0)   continue;
	  // check valid bit
	  auto hVldBt = hStub.range( kMSBVldBt ,  kLSBVldBt);
	  if( hVldBt == 0 ){ 
	  	#ifndef __SYNTHESIS__
	  		#if IR_DEBUG
	  			std::cout << "\t.. Invalid Stub : " << std::hex << hStub << std::dec << "\n";
	  		#endif
	  	#endif
	  	continue;
	  }
	  // check which memory
	  // needs to be filled 
	  // encoded layer 
	  auto hEncLyr = hStub.range(kMSBLyrBts, kLSBLyrBts);
	  // get memory word
	  // discard LSBs
	  auto hStbWrd = ap_uint<kBRAMwidth>(hStub.range(kNBits_DTC - 1, kLyBitsSize + kVldBitSize));	
	  // old data format 
	  // discard MSBs
	  //auto hStbWrd = ap_uint<kBRAMwidth>(hStub.range(kBRAMwidth - 1, 0));
	  DTCStub hMemWord(hStbWrd);
	    
	  // decode link wrd for this layer
	  auto hIsBrl= hLinkWord.range((kNBitsBrlBit-1) + kSizeLinkWord * hEncLyr, kSizeLinkWord * hEncLyr);
	  auto hLyrId= hLinkWord.range((kNBitsLyrTk-1) + kSizeLinkWord * hEncLyr + kNBitsBrlBit, kNBitsBrlBit + kSizeLinkWord * hEncLyr);
	  
	  // point to the correct 
	  // LUT with the phi 
	  // corrections 
	  static const int* cLUT; 
	  if( hLyrId == kFrstPSBrlLyr || hLyrId == kFrst2SBrlLyr ) 
	  	cLUT = hPhiCorrtable_L1;
	  else if( hLyrId == kScndPSBrlLyr || hLyrId == kScnd2SBrlLyr ) 
	  	cLUT = hPhiCorrtable_L2;
	  else if( hLyrId == kThrdPSBrlLyr || hLyrId == kThrd2SBrlLyr )
	  	cLUT = hPhiCorrtable_L3;
	  
	  // update index
	  unsigned int cIndx = 0;
	  GetMemoryIndex<kMaxLyrsPerDTC>( nMemsPerLyr, hEncLyr, cIndx);
	  // get phi bin
	  int cIndxThisBn = 0;
	  // for the barrel need to correct 
	  if( hIsBrl == 1 )
	  {
	  	int cOffsetBrl = (hLyrId == kFrstPSBrlLyr) ? kNbitsPhiBinsPSL1 : kNbitsPhiBinsTkr;  
	  	if( hIs2S == 0 ) getPhiBinWithCorrection<BARRELPS>(hStbWrd, cLUT, cOffsetBrl, cIndxThisBn);
	  	else getPhiBinWithCorrection<BARREL2S>(hStbWrd, cLUT, cOffsetBrl, cIndxThisBn);
	  }
	  else
	  {
	  	int cOffsetDsk = kNbitsPhiBinsTkr;
	  	if( hIs2S == 0 ) getPhiBin<DISKPS>( hStbWrd , cOffsetDsk , cIndxThisBn ) ;
	  	else getPhiBin<DISK2S>( hStbWrd , cOffsetDsk , cIndxThisBn ) ;
	  }

	  // assign memory index
	  auto cMemIndx = cIndx+cIndxThisBn;
	  assert(cMemIndx < nMems);
	  #ifndef __SYNTHESIS__
	  	#if IR_DEBUG
		   std::cout << "\t.. Stub : " << std::hex << hStub << std::dec 
		   				<< " rel. parts : " << std::hex << hStbWrd << std::dec
			            << " [ EncLyrId " << hEncLyr << " ] "
						<< "[ LyrId " << hLyrId << " ] "
			            << "[ IsBrl " << +hIsBrl << " ] "
						<< " [ Nmemories " << nMems << " ] "
			            << " [ IndxThisBn " << cIndxThisBn << " ] "
			            << " [ Indx " << cIndx << " ] "
			            << " [ memIndx " << cMemIndx << " ] "
			            << "\n";
		#endif
	#endif
	  
	  // update  counters 
	  auto hEntries = hNStubs[cMemIndx];
	  hNStubs[cMemIndx] = hEntries + 1;
	  // fill memory 
	  (&hOutputStubs[cMemIndx])->write_mem(bx, hMemWord, hEntries);
	}
	// update output bx port 
	bx_o = bx;
}


#endif


