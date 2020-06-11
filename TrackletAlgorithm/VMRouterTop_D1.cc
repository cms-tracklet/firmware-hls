#include "VMRouterTop_D1.h"

// VMRouter Top Function for Disk 1, AllStub region A
void VMRouterTop(BXType bx,
// Input memories
		const InputStubMemory<DISK2S> *i0,
		const InputStubMemory<DISKPS> *i1,
		const InputStubMemory<DISKPS> *i2,
		const InputStubMemory<DISK2S> *i3,
		const InputStubMemory<DISKPS> *i4,
		const InputStubMemory<DISKPS> *i5,
// Output memories
AllStubMemory<DISK> allStub[6],
VMStubMEMemory<DISK> meMemories[8],
VMStubTEInnerMemory<DISK> teiMemories[4][3],
VMStubTEOuterMemory<DISK> teoMemories[4][5]
		)
{

	//////////////////////////////////
	// Variables for that are specified with regards to the test bench
	
	constexpr int layer(0); // Which barrel layer number the data is coming from, 0 if not barrel
	constexpr int disk(1); // Which disk number the data is coming from, 0 if not disk
	
	// Maximum number of memory "copies"
	constexpr int nall(6); // Allstub memory
	constexpr int ntei(3); // TE Inner memories
	constexpr int nteol(1); // Can't use 0 evne if we don't have any TE Inner Overlap memories
	constexpr int nteo(5); // TE Outer memories
	
	// Masks of which memories that are being used. The first memory is represented by the LSB
	static const ap_uint<6> imask(0x3F); // Input memories
	static const ap_uint<32> memask(0x000000FFUL); // ME memories
	static const ap_uint<32> teimask(0x0000000FUL); // TE Inner memories
	static const ap_uint<16> olmask(0x000); // TE Inner Overlap memories
	static const ap_uint<32> teomask(0x0000000FUL); // TE Outer memories


	///////////////////////////
	// Open Lookup tables

	// LUT with the corrected r/z. It is corrected for the average r (z) of the barrel (disk).
	// Includes both coarse r/z position (bin), and finer region each r/z bin is divided into.
	// Indexed using r and z position bits
	static const int finebintable[] =
#include "../emData/MemPrints/Tables/VMR_D1PHIA_finebin.tab"
	;

	// LUT with phi corrections to project the stub to the nominal radius. 
	// Only used by layers.
	// Indexed using phi and bend bits
	// 	static const int phicorrtable[] =
	// #include "../emData/MemPrints/Tables/VMPhiCorrL1.txt"
	// 	;
		
	// LUT with the Z/R bits for TE memories
	// Contain information about where in z to look for valid stub pairs
	// Indexed using z and r position bits
	static const int rzbitstable[2048] = // 11 bits used for LUT
#include "../emData/MemPrints/Tables/VMTableInnerD1D2.tab" // Only for Layer 1
	;

	// LUT with the Z/R bits for TE Overlap memories
	// Only used for Layer 1 and 2, and Disk 1
	// Indexed using z and r position bits
	static const int rzbitsextratable[] = // 10 bits used for LUT
#include "../emData/MemPrints/Tables/VMTableOuterD1.tab" // Only for Layer 1
	;


	// LUT with bend-cuts for the TE memories
	// The cuts are different depending on the memory version (nX)

	// TE Memory 1
	ap_uint<1> tmptable1_n1[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIA1n1_vmbendcut.tab"
	;
	ap_uint<1> tmptable1_n2[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIA1n2_vmbendcut.tab"
	;
	ap_uint<1> tmptable1_n3[8] = {0};
	
	// TE Memory 2
	ap_uint<1> tmptable2_n1[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIA2n1_vmbendcut.tab"
	;
	ap_uint<1> tmptable2_n2[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIA2n2_vmbendcut.tab"
	;
	ap_uint<1> tmptable2_n3[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIA2n3_vmbendcut.tab"
	;
	
	// TE Memory 3
	ap_uint<1> tmptable3_n1[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIA3n1_vmbendcut.tab"
	;
	ap_uint<1> tmptable3_n2[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIA3n2_vmbendcut.tab"
	;
	ap_uint<1> tmptable3_n3[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIA3n3_vmbendcut.tab"
	;

	// TE Memory 4
	ap_uint<1> tmptable4_n1[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIA4n1_vmbendcut.tab"
	;
	ap_uint<1> tmptable4_n2[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIA4n2_vmbendcut.tab"
	;
	ap_uint<1> tmptable4_n3[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIA4n3_vmbendcut.tab"
	;
	
	static const ap_uint<1>* bendtable[] = {
		tmptable1_n1, tmptable1_n2, tmptable1_n3, 
		tmptable2_n1, tmptable2_n2, tmptable2_n3, 
		tmptable3_n1, tmptable3_n2, tmptable3_n3,
		tmptable4_n1, tmptable4_n2, tmptable4_n3};


	// TE Overlap Memory 1
	ap_uint<1> tmpextratable1_n1[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX1n1_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable1_n2[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX1n2_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable1_n3[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX1n3_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable1_n4[8] = {0};
	
	ap_uint<1> tmpextratable1_n5[8] = {0};

	// TE Overlap Memory 2
	ap_uint<1> tmpextratable2_n1[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX2n1_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable2_n2[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX2n2_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable2_n3[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX2n3_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable2_n4[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX2n4_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable2_n5[8] = {0};

	// TE Overlap Memory 3
	ap_uint<1> tmpextratable3_n1[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX3n1_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable3_n2[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX3n2_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable3_n3[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX3n3_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable3_n4[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX3n4_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable3_n5[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX3n5_vmbendcut.tab"
	;

	// TE Overlap Memory 4
	ap_uint<1> tmpextratable4_n1[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX4n1_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable4_n2[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX4n2_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable4_n3[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX4n3_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable4_n4[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX4n4_vmbendcut.tab"
	;
	ap_uint<1> tmpextratable4_n5[] =
#include "../emData/MemPrints/Tables/VMSTE_D1PHIX4n5_vmbendcut.tab"
	;

	static const ap_uint<1>* bendextratable[] = {
		tmpextratable1_n1, tmpextratable1_n2, tmpextratable1_n3, tmpextratable1_n4, tmpextratable1_n5,
		tmpextratable2_n1, tmpextratable2_n2, tmpextratable2_n3, tmpextratable2_n4, tmpextratable2_n5,
		tmpextratable3_n1, tmpextratable3_n2, tmpextratable3_n3, tmpextratable3_n4, tmpextratable3_n5,
		tmpextratable4_n1, tmpextratable4_n2, tmpextratable4_n3, tmpextratable4_n4, tmpextratable4_n5};



// Takes 2 clock cycles before on gets data, used at high frequencies
#pragma HLS resource variable=i0->get_mem() latency=2
#pragma HLS resource variable=i1->get_mem() latency=2
#pragma HLS resource variable=i2->get_mem() latency=2
#pragma HLS resource variable=i3->get_mem() latency=2
#pragma HLS resource variable=i4->get_mem() latency=2
#pragma HLS resource variable=i5->get_mem() latency=2

#pragma HLS resource variable=finebintable latency=2
#pragma HLS resource variable=rzbitstable latency=2
#pragma HLS resource variable=rzbitsextratable latency=2
//#pragma HLS resource variable=bendtable latency=2
//#pragma HLS resource variable=bendextratable latency=2
//phicorrtable and bendtable seems to be using LUTs as they relatively small?


/////////////////////////
// Main function
	
	// template<regionType INTYPE, regionType INTYPE2, regionType OUTTYPE, int LAYER, int DISK, int MAXNALL, int MAXNTEI, int MAXNTEOL, int MAXNTEO>
	// Disks have two types of input
	VMRouter<DISKPS, DISK2S, DISK, layer, disk, nall, ntei, nteol, nteo>
	(bx, finebintable, nullptr, 
		rzbitstable, nullptr, rzbitsextratable, 
		bendtable, nullptr, bendextratable,
// Input memories
		imask, i0,i1,i2,i3,i4,i5,//,i6,i7,
// AllStub memories
		allStub,
// ME memories
		memask, meMemories,
// TEInner memories
		teimask, teiMemories,
// TEInner Overlap memories
		olmask, nullptr,
// TEOuter memories
		teomask, teoMemories
		 );

	return;
}
