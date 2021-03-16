#ifndef TrackletAlgorithm_MatchEngine_h
#define TrackletAlgorithm_MatchEngine_h

// cms-tracklet/firmware-hls Headers
#include "Constants.h"
#include "VMProjectionMemory.h"
#include "VMStubMEMemory.h"
#include "CandidateMatchMemory.h"

// HLS Headers
#include "hls_math.h"

// STL Headers
#include <iostream>
#include <fstream>

/////////////////////////////
// -- PREPROCESSOR FUNCTIONS
#define IS_REPRESENTIBLE_IN_D_BITS(D, N)                \
   (((unsigned long) N >= (1UL << (D - 1)) && (unsigned long) N < (1UL << D)) ? D : -1)
#define BITS_TO_REPRESENT(N)                             \
   (N == 0 ? 1 : (31                                     \
				  + IS_REPRESENTIBLE_IN_D_BITS( 1, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS( 2, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS( 3, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS( 4, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS( 5, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS( 6, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS( 7, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS( 8, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS( 9, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(10, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(11, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(12, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(13, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(14, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(15, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(16, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(17, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(18, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(19, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(20, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(21, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(22, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(23, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(24, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(25, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(26, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(27, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(28, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(29, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(30, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(31, N)    \
				  + IS_REPRESENTIBLE_IN_D_BITS(32, N)    \
				  )                                      \
	  )
#define LAYER 3
//Options: BARREL and DISK
#define PROJECTIONTYPE BARREL
#if (LAYER >= 1) && (LAYER <= 3)
	#define MODULETYPE BARRELPS
#elif (LAYER >= 4) && (LAYER <= 6)
	#define MODULETYPE BARREL2S
#endif
#if LAYER
  #define NBITBIN 3
#else
  #define NBITBIN 4
#endif

#define RINVSTEPS 32
//BARRELPS=256 and BARREL2S=512
#define LSIZE RINVSTEPS*(1<<VMStubME<MODULETYPE>::kVMSMEBendSize)
#define BUFFERSIZE 8
constexpr unsigned int kNBits_BufferAddr=BITS_TO_REPRESENT(BUFFERSIZE-1);
constexpr int kBufferDataSize = VMStubMEMemory<MODULETYPE,NBITBIN>::kNBitDataAddr   // number of bits for stubs array size
							  + VMProjection<PROJECTIONTYPE>::kVMProjectionSize	// projection data size
							  + MEBinsBits										// number of bits for index of z-bin
							  + 1;												// z-bin flag (0 => first bin, 1 => second bin)
namespace ME {
	enum BitLocations {
		// The location of the least significant bit (LSB) and most significant bit (MSB) in the ME buffer word for different fields
		kVMMESecondLSB = 0,
		kVMMESecondMSB = 0,
		kVMMEZBinLSB = kVMMESecondMSB + 1,
		kVMMEZBinMSB = kVMMEZBinLSB + MEBinsBits - 1,
		kVMMEProjectionLSB = kVMMEZBinMSB + 1,
		kVMMEProjectionMSB = kVMMEProjectionLSB + VMProjection<PROJECTIONTYPE>::kVMProjectionSize - 1,
		kVMMENStubsLSB = kVMMEProjectionMSB + 1,
		kVMMENStubsMSB = kVMMENStubsLSB + VMStubMEMemory<MODULETYPE,NBITBIN>::kNBitDataAddr - 1
	};
	enum StubZPositionBarrelConsistency {
		kPSMin = -1,
		kPSMax = 1,
		k2SMin = -5,
		k2SMax = 5
	};
	enum StubPhiPositionBarrelConsistency {
		kMin = 3,
		kMax = 5
	};
}
constexpr unsigned int kZAdjustment = 8;

/////////////////////////////
// -- MATCH ENGINE FUNCTIONS
void readTable(bool table[LSIZE]);

//template<int L, regionType VMSMEType>
template<int L, int VMSMEType>
void MatchEngine(const BXType bx, BXType& bx_o,
		 		 const VMStubMEMemory<VMSMEType, NBITBIN>& inputStubData,
		 		 const VMProjectionMemory<PROJECTIONTYPE>& inputProjectionData,
		 		 CandidateMatchMemory& outputCandidateMatch);

void MatchEngineTop(const BXType bx, BXType& bx_o,
					const VMStubMEMemory<MODULETYPE, NBITBIN>& inputStubData,
					const VMProjectionMemory<PROJECTIONTYPE>& inputProjectionData,
					CandidateMatchMemory& outputCandidateMatch);

#endif
