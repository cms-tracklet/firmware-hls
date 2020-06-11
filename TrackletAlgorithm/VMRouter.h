// VMRouter
// Log
// -------
// First tracklet 2.0 version -- December 2018 -- wittich

#ifndef VMROUTER_H
#define VMROUTER_H

#include "ap_int.h"

#include "Constants.h"
#include "InputStubMemory.h"
#include "AllStubMemory.h"
#include "VMStubMEMemory.h"
#include "VMStubTEInnerMemory.h"
#include "VMStubTEOuterMemory.h"
#include <cassert>

// from Constants.hh -- needs a final home?
constexpr unsigned int nallstubslayers[6] = { 8, 4, 4, 4, 4, 4 }; // Number of AllStub memories, i.e. coarse phi regions, per sector
constexpr unsigned int nallstubsdisks[5] = { 4, 4, 4, 4, 4 };

constexpr unsigned int nvmmelayers[6] = { 4, 8, 8, 8, 8, 8 }; // Number of ME VM modules per coarse phi region
constexpr unsigned int nvmmedisks[5] = { 8, 4, 4, 4, 4 };

constexpr unsigned int nvmtelayers[6] = { 4, 8, 4, 8, 4, 8 }; // Number of TE VM modules per coarse phi region
constexpr unsigned int nvmtedisks[5] = { 4, 4, 4, 4, 4 };

constexpr unsigned int nvmteoverlaplayers[2] = {2, 2}; // Number of Overlap VM modules per coarse phi region

// Number of finephi bits used. Can't this be taken from the memories?
static const int nfinephibarrelinner = 2;
static const int nfinephibarrelouter = 3;

static const int nfinephidiskinner = 2;
static const int nfinephidiskouter = 3;

static const int nfinephioverlapinner = 2;
static const int nfinephioverlapouter = 3;

// Maximum number of stubs that can be processed (memory depth)
constexpr int MAXVMROUTER = kMaxProc;

// Number of bits used for the VMs for different layers and disks
// E.g. 32 VMs would use 5 vmbits
constexpr int vmbitslayer[6] = { 5, 5, 4, 5, 4, 5 }; // Could be computed using the number of VMs...
constexpr int vmbitsdisk[5] = { 4, 4, 4, 4, 4 };
constexpr int vmbitsoverlap[2] = { 4, 3 };

// Number of most significant bits (MSBs) of z and r used for index in finebin LUTs
constexpr int nbitszfinebintablelayer = 7;
constexpr int nbitsrfinebintablelayer = 4;

constexpr int nbitszfinebintabledisk = 3;
constexpr int nbitsrfinebintabledisk = 7;

constexpr int nzbitsinnertablelayer = 7;
constexpr int nrbitsinnertablelayer = 4;

constexpr int nzbitsinnertabledisk = 3;
constexpr int nrbitsinnertabledisk = 8;

constexpr int nzbitsoutertablelayer = 7;
constexpr int nrbitsoutertablelayer = 4;

constexpr int nzbitsoutertabledisk = 3;
constexpr int nrbitsoutertabledisk = 7;

constexpr int nzbitsrzbitsoverlaptable = 7;
constexpr int nrbitsrzbitsoverlaptable = 3;

// Number of MSBs used for r index in phicorr LUTs
constexpr int nrbitsphicorrtable = 3; // Found hardcoded in VMRouterPhiphicorrtable.h



// Some functions

// Returns top 5 bits of phi, i.e. max 31 in decimal
// TODO: this one isn't really necessary
template<regionType InType>
inline ap_uint<5> iphivmRaw(const typename AllStub<InType>::ASPHI phi) {
// TODO: get rid of hard-coded values
	ap_uint<5> iphivm = phi.range(phi.length() - 1, phi.length() - 5);
	return iphivm;
}

// Returns the bits of phi corresponding to finephi, i.e. phi regions within a VM
// vmbits is the number of bits for the VMs, i.e. coarse phi region. E.g. 32 VMs would use vmbits=5
// finebits is the number of bits within the VM
template<regionType InType>
inline int iphivmFineBins(const typename AllStub<InType>::ASPHI phi,
		const int vmbits, const int finebits) {

	auto v1 = (phi.range(phi.length() - 1 - vmbits,
			phi.length() - vmbits - finebits));
//auto v2 = (phi >> (phi.length() - vmbits - finebits)) & ((1 << finebits) - 1);
//std::cout << "PHI: " << phi << "   v1: " << v1 << "    v2: " << v2 << std::endl;
//assert(v1 == v2);
	return v1;
}

// Returns a number from 0 to 31. for both the plus and the minus:
// we add a small amount to the raw value; if it's not the same
// as the central value we copy the data to the adjacent memory as well.
template<regionType InType>
inline ap_uint<5> iphivmRawPlus(const typename AllStub<InType>::ASPHI phi) {
// // TODO: get rid of hard-coded values
// ap_uint<7> tmp = phi.range(phi.length()-1,phi.length()-7);
// auto iphivmp = ++tmp;
// return ap_uint<5>(iphivmp(2,6));
	ap_uint<7> tmp(phi.range(phi.length() - 1, phi.length() - 7));
	++tmp;
	ap_uint<5> plus(tmp.range(tmp.length() - 1, 2));
	return plus;
}

// See above
// TODO: make plus and minus use similar methods
template<regionType InType>
inline ap_uint<5> iphivmRawMinus(const typename AllStub<InType>::ASPHI phi) {
	ap_uint<7> tmp(phi.range(phi.length() - 1, phi.length() - 7));
	auto iphivmp = --tmp;
	return ap_uint<5>(iphivmp(6, 2));
//  // TODO: get rid of hard-coded values
//  auto iphivm=((phi-(1<<(phi.length()-7)))>>(phi.length()-5));
//  if (iphivm<0) iphivm=0;
//  else if (iphivm>31) iphivm=0;
//  return ap_uint<5>(iphivm);
}

// Get the corrected phi, i.e. phi at the average radius of the barrel
// Corrected phi is used by ME and TE memories in the barrel
template<regionType InType>
inline typename AllStub<InType>::ASPHI getPhiCorr(
		const typename AllStub<InType>::ASPHI phi,
		const typename AllStub<InType>::ASR r,
		const typename AllStub<InType>::ASBEND bend, const int phicorrtable[]) {

	if (InType == DISKPS || InType == DISK2S)
		return phi; // Do nothing if disks

	constexpr auto rbins = 1 << nrbitsphicorrtable; // The number of bins for r

	ap_uint<nrbitsphicorrtable> rbin = (r + (1 << (r.length() - 1)))
			>> (r.length() - nrbitsphicorrtable); // Which bin r belongs to. Note r = 0 is mid radius
	auto index = bend * rbins + rbin; // Index for where we find our correction value
	auto corrval = phicorrtable[index]; // The amount we need to correct our phi

	auto phicorr = phi - corrval; // the corrected phi
	
	// Check for overflow
	if (phicorr < 0)
		phicorr = 0; // can't be less than 0
	if (phicorr >= 1 << phi.length())
		phicorr = (1 << phi.length()) - 1;  // can't be more than the max value

	return phicorr;
}

// Get the number of the first ME/TE memory for the current VMRouter
// I.e. the position of the first non-zero bit in the mask
// L1PHIE17 would return 16
inline ap_uint<5> getFirstMemNumber(const ap_uint<32> mask) {
	ap_uint<5> i = 0;
	ap_uint<1> x = mask[i]; // Value of the i:th bit

// Stop counter when we have reached the first non-zero bit
	while (x == 0 && i < 31) {
		i++;
		x = mask[i];
	}

	return i;
}

/////////////////////////////////
// Main function

// InType InType2 - Two input region types InType and InType2 due to the disks having both 2S and PS inputs.
// 		According to wiring script for disks, the 0th and 2th input are different for Disks.
// 		The last three inputs are also for the negative disks.
// Layer Disk - Specifies the layer or disk number
// MAXN - The maximum number of copies of a memory type
template<regionType InType, regionType InType2, regionType OutType, int Layer, int Disk, int MaxAllCopies, int MaxTEICopies, int MaxOLCopies, int MaxTEOCopies>
void VMRouter(const BXType bx, const int finebintable[], const int phicorrtable[], 
		// rzbitstables (binlookup in emulation)
		const int rzbitsinnertable[], const int rzbitsoverlaptable[], const int rzbitsoutertable[],
		// bendcut tables
		const ap_uint<1>* bendtable[], const ap_uint<1>* bendoverlaptable[], const ap_uint<1>* bendoutertable[],
		// Input memories
		const ap_uint<6>& imask, 
		const InputStubMemory<InType2>* const i0,
		const InputStubMemory<InType>* const i1,
		const InputStubMemory<InType>* const i2,
		const InputStubMemory<InType2>* const i3,
		const InputStubMemory<InType>* const i4,
		const InputStubMemory<InType>* const i5,
		// AllStub memory
		AllStubMemory<OutType> allStub[],
		// ME memories
		const ap_uint<32>& memask, VMStubMEMemory<OutType> meMemories[],
		// // Inner TE memories, non-overlap
		const ap_uint<32>& teimask, VMStubTEInnerMemory<OutType> teiMemories[][MaxTEICopies],
		// // TE Inner memories, overlap
		const ap_uint<16>& olmask, VMStubTEInnerMemory<BARRELOL> olMemories[][MaxOLCopies],
		// // TE Outer memories
		const ap_uint<32>& teomask, VMStubTEOuterMemory<OutType> teoMemories[][MaxTEOCopies]) {

#pragma HLS inline
#pragma HLS array_partition variable=bendtable complete dim=1
#pragma HLS array_partition variable=bendoverlaptable complete dim=1
#pragma HLS array_partition variable=bendoutertable complete dim=1
//#pragma HLS interface ap_bus port=bendtable depth=8
#pragma HLS array_partition variable=allstub complete
#pragma HLS array_partition variable=meMemories complete
#pragma HLS array_partition variable=teiMemories complete dim=2
#pragma HLS array_partition variable=olMemories complete dim=2
#pragma HLS array_partition variable=teoMemories complete dim=2


	// Some variables
	constexpr int nbitszfinebintable =
			(Layer) ? nbitszfinebintablelayer : nbitszfinebintabledisk; // Number of MSBs of z used in finebintable
	constexpr int nbitsrfinebintable =
			(Layer) ? nbitsrfinebintablelayer : nbitsrfinebintabledisk; // Number of MSBs of r used in finebintable
	
	// The first memory numbers, the position of the first non-zero bit in the mask
	static const ap_uint<5> firstme = getFirstMemNumber(memask); // ME memory
	static const ap_uint<5> firstte = (teimask) ? getFirstMemNumber(teimask) : getFirstMemNumber(teomask); // TE memory
	static const ap_uint<5> firstol = (olmask) ? static_cast<int>(getFirstMemNumber(olmask)) : 0; // TE Overlap memory

	// Number of memories/VMs for one coarse phi region
	constexpr int nvmme = (Layer) ? nvmmelayers[Layer-1] : nvmmedisks[Disk-1]; // ME memories
	constexpr int nvmte = (Layer) ? nvmtelayers[Layer-1] : nvmtedisks[Disk-1]; // TE memories
	constexpr int nvmol = ((Layer == 1) || (Layer == 2)) ? nvmteoverlaplayers[Layer-1] : 0; // TE Overlap memories

	// Total number of VMs for ME in a whole sector
	constexpr int ntotvmme =
			Layer != 0 ?
					nallstubslayers[Layer - 1]
							* nvmmelayers[Layer - 1] :
					nallstubsdisks[Disk - 1] * nvmmedisks[Disk - 1];
	// Total number of VMs for TE in a whole sector
	constexpr int ntotvmte =
			Layer != 0 ?
					nallstubslayers[Layer - 1] * nvmtelayers[Layer - 1] :
					nallstubsdisks[Disk - 1] * nvmtedisks[Disk - 1];

	// Some sort of normalisation thing used for determining which VM the stub belongs to
	static const ap_ufixed<5, 4> d_me = ntotvmme / 32.0;
	static const ap_ufixed<5, 4> d_te = ntotvmte / 32.0;

	// Reset address counters in output memories
	// Only clear if the masks says that memory is used
	ALLSTUB_CLEAR:	for (int i = 0; i < MaxAllCopies; i++) {
		#pragma HLS UNROLL
		allStub[i].clear(bx);
	}

	if (memask) {
		ME_CLEAR:	for (int i = 0; i < nvmme; i++) {
			#pragma HLS UNROLL
			if (memask[i + firstme]) meMemories[i].clear(bx);
		}
	}

	if (teimask) {
		TEI_CLEAR:	for (int i = 0; i < nvmte; i++) {
			#pragma HLS UNROLL
			if (teimask[i + firstte]) {
				for (int j = 0; j < MaxTEICopies; j++) {
					#pragma HLS UNROLL
					teiMemories[i][j].clear(bx);
				}
			}
		}
	}

	if (teomask) {
		TEO_CLEAR:	for (int i = 0; i < nvmte; i++) {
			#pragma HLS UNROLL
			if (teomask[i + firstte]) {
				for (int j = 0; j < MaxTEOCopies; j++) {
					#pragma HLS UNROLL
					teoMemories[i][j].clear(bx);
				}
			}
		}
	}

	if (olmask) {
	OL_CLEAR:	for (int i = 0; i < nvmol; i++) {
			#pragma HLS UNROLL
			if (olmask[i + firstol]) {
				for (int j = 0; j < MaxOLCopies; j++) {
					#pragma HLS UNROLL
					olMemories[i][j].clear(bx);
			}
			}
		}
	}
	
	
	// Number of data in each input memory
	
	const typename InputStubMemory<InType>::NEntryT zero(0);

	auto n_i0 = imask[0] != 0 ? i0->getEntries(bx) : zero;
	auto n_i1 = imask[1] != 0 ? i1->getEntries(bx) : zero;
	auto n_i2 = imask[2] != 0 ? i2->getEntries(bx) : zero;
	auto n_i3 = imask[3] != 0 ? i3->getEntries(bx) : zero;
	auto n_i4 = imask[4] != 0 ? i4->getEntries(bx) : zero;
	auto n_i5 = imask[5] != 0 ? i5->getEntries(bx) : zero;

	int n_tot = n_i0 + n_i1 + n_i2 + n_i3 + n_i4 + n_i5;
	//const int maxLoop = (kMaxProc < n_tot) ? kMaxProc : n_tot;


// Create variables that keep track of which memory address to read and write to

	ap_uint<kNBits_MemAddr> read_addr(0); // Reading of input stubs
	
	int addrCountTEI[nvmte][MaxTEICopies]; // Writing of TE Inner stubs
	if (teimask) {
		#pragma HLS array_partition variable = addrCountTEI complete dim=0
		ADDR_TEI:	for (int i = 0; i < nvmte; i++) {
			#pragma HLS UNROLL
			for (int j = 0; j < MaxTEICopies; j++) {
				#pragma HLS UNROLL
					addrCountTEI[i][j] = 0;
			}
		}
	}

	int addrCountOL[nvmol][MaxOLCopies]; // Writing of TE Overlap stubs
	if (olmask) {
	#pragma HLS array_partition variable=addrCountOL complete dim=0
	ADDR_OL:	for (int i = 0; i < nvmol; i++) {
			#pragma HLS UNROLL
			for (int j = 0; j < MaxOLCopies; j++) {
				#pragma HLS UNROLL
				addrCountOL[i][j] = 0;
			}
		}
}


/////////////////////////////////////
// Main Loop 

	TOPLEVEL: for (auto i = 0; i < kMaxProc; ++i) {
#pragma HLS PIPELINE II=1

		// const bool haveData = (n_i0 > 0) || (n_i1 > 0) || (n_i2 > 0)
		// 		|| (n_i3 > 0) || (n_i4 > 0) || (n_i5 > 0);
		
// Stop processing stubs if we have looped over the maximum number
// that can be processed or if we have gone through all data
		if ((i > MAXVMROUTER) || !n_tot) // use haveData or n_tot?
			continue;

		bool resetNext = false; // Used to reset read_addr
		bool isSpecialStub = false; // Used to determine if first or fourth input memory, which are different for disks
		bool disk2S = false; // Used to determine if DISK2S
		bool negdisk = false; // Used to determine if it's negative disk, the last 3 inputs memories
		
		InputStub<InType> stub;
		InputStub<InType2> stubspec; // Used for input 1 and 4 since they can be of different type in disks. find a better way to do this...
		
// Read stub from memory in turn
		if (n_i0) {
			stubspec = i0->read_mem(bx, read_addr);
			isSpecialStub = true;
			--n_i0;
			if (n_i0 == 0)
				resetNext = true;
		} else if (n_i1) {
			stub = i1->read_mem(bx, read_addr);
			--n_i1;
			if (n_i1 == 0)
				resetNext = true;
		} else if (n_i2) {
			stub = i2->read_mem(bx, read_addr);
			--n_i2;
			if (n_i2 == 0)
				resetNext = true;
		} else if (n_i3) {
			stubspec = i3->read_mem(bx, read_addr);
			isSpecialStub = true;
			negdisk = (Disk) ? true : false;
			--n_i3;
			if (n_i3 == 0)
				resetNext = true;
		} else if (n_i4) {
			stub = i4->read_mem(bx, read_addr);
			negdisk = (Disk) ? true : false;
			--n_i4;
			if (n_i4 == 0)
				resetNext = true;
		} else {
			stub = i5->read_mem(bx, read_addr);
			negdisk = (Disk) ? true : false;
			--n_i5;
			if (n_i5 == 0)
				resetNext = true;
		}
		
		--n_tot;

// If the stub is from a DISK2S module
		if (isSpecialStub && Disk) {
			disk2S = true;
		}

// Increment the read address, or reset it to zero when all stubs in a memory has been read
		if (resetNext)
			read_addr = 0;
		else
			++read_addr;


		// Add stub to all AllStub memory copies
		AllStub<OutType> allstubd =
				(isSpecialStub) ? stubspec.raw() : stub.raw();
		for (int n = 0; n < MaxAllCopies; n++) {
			#pragma HLS UNROLL
			allStub[n].write_mem(bx, allstubd, i);
		}
		// For debugging
		#ifndef __SYNTHESIS__
				std::cout << "Out put stub: " << std::hex << allstubd.raw() << std::dec
						<< std::endl;
		#endif // DEBUG





// Variables that are going to be used by ME and TE memories
		auto z = (isSpecialStub) ? stubspec.getZ() : stub.getZ();
		int r = (isSpecialStub) ? stubspec.getR().range() : stub.getR().range();
		int bend =
				(isSpecialStub) ?
						stubspec.getBend().range() : stub.getBend().range();
		auto stubPhi_uncorr =
				(isSpecialStub) ? stubspec.getPhi() : stub.getPhi(); // Original phi, uncorred_ted.
		auto stubPhi = getPhiCorr<InType>(stubPhi_uncorr, r, bend, phicorrtable); // Corred_ted phi, i.e. phi at nominal radius (what about disks?)

		int nrbits =
				(isSpecialStub) ?
						stubspec.getR().length() : stub.getR().length(); // Number of bits for r
		int nzbits =
				(isSpecialStub) ?
						stubspec.getZ().length() : stub.getZ().length(); // Number of bits for z
		int nbendbits =
				(isSpecialStub) ?
						stubspec.getBend().length() : stub.getBend().length();


/////////////////////////////////////////////
// ME memories

		if (memask != 0) {

			VMStubME<OutType> stubme;
			stubme.setBend(bend);
			stubme.setIndex(typename VMStubME<OutType>::VMSMEID(i));

			auto iphiRaw = iphivmRaw<InType>(stubPhi); // Top 5 bits of phi
			auto iphiRawPlus = iphivmRawPlus<InType>(stubPhi); // Top 5 bits of phi after adding a small number
			auto iphiRawMinus = iphivmRawMinus<InType>(stubPhi); // Top 5 bits of phi after subtracting a small number

			int ivm = iphiRaw * d_me; // The VM number
			int ivmPlus = iphiRawPlus * d_me;
			int ivmMinus = iphiRawMinus * d_me;

// To avoid overflow
			if (ivmMinus > ivm)
				ivmMinus = 0;
			if (ivmPlus < ivm)
				ivmPlus = ntotvmme - 1;

// Stubs can only end up in the neighbouring VM after calculating iphivmrawplus/minus
			assert(std::abs(ivm - ivmPlus) <= 1);
			assert(std::abs(ivm - ivmMinus) <= 1);

// Indices used to find the rzfine value in finebintable
// finebintable returns the top 6 bits of a corrected z
// Note: not the index that is being saved to the stub
			ap_uint<nbitszfinebintable + nbitsrfinebintable> indexz = ((z
					+ (1 << (nzbits - 1))) >> (nzbits - nbitszfinebintable)); // Make z unsigned and take the top "nbitszfinebintable" bits
			ap_uint<nbitsrfinebintable> indexr = 0;

			if (Disk) {
				if (negdisk) {
					indexz = (1 << nbitszfinebintable) - indexz;
				}
				indexr = r >> (nrbits - nbitsrfinebintable); // Take the top "nbitsrfinebintable" bits
			} else { // Layer
				indexr = ((r + (1 << (nrbits - 1)))
						>> (nrbits - nbitsrfinebintable)); // Make r unsigned and take the top "nbitsrfinebintable" bits
			}

// The index for finebintable
			ap_uint<nbitszfinebintable + nbitsrfinebintable> index = (indexz
					<< nbitsrfinebintable) + indexr;

// Get the corrected r/z position
			ap_uint<6> rzcorr = finebintable[index];

// Coarse z. The bin the stub is going to be put it in the memory
			ap_uint<MEBinsBits> bin = rzcorr >> 3; // 3 bits, i.e. max 8 bins within each VM
			if (negdisk)
				bin += 8; // Only needed if 16 ME bins

// Set rzfine, i.e. the r/z bits within a coarse r/z region
			ap_uint<3> rzfine = rzcorr & 7; // the 3 LSB as rzfine
			stubme.setFineZ(rzfine);

			assert(rzfine >= 0);

// For debugging
#ifndef __SYNTHESIS__
			std::cout << "ME stub " << std::hex << stubme.raw() << std::endl;
			std::cout << "ivm,Minus,Plus = " << std::dec << ivm << " "
					<< ivmMinus << " " << ivmPlus << " " << "\t0x"
					<< std::setfill('0') << std::setw(4) << std::hex
					<< stubme.raw().to_int() << std::dec << ", to bin " << bin
					<< std::endl;
			if (!memask[ivm]) {
				std::cerr << "Trying to write to non-existent memory for ivm = "
						<< ivm << std::endl;
			}
#endif // DEBUG


		// Write the ME stub to the correct memory.
		// If stub is close to a border (ivmPlus/ivmMinus != ivm)
		// write it to the adjacent memory as well
		// Note that at least one of ivmPlus/Minus == ivm
		#pragma HLS dependence variable=meMemories intra false
		for (int n = 0; n < 32; n++) {
			#pragma HLS UNROLL
			if (memask[n]) {
					if ((ivmMinus == n) || (ivmPlus == n))
						meMemories[n-firstme].write_mem(bx, bin, stubme);
				}
		}
	}
		//executeME() END   ------------------------------



//////////////////////////////////
// TE Memories

		auto iphiRaw = iphivmRaw<InType>(stubPhi); // Top 5 bits of phi.
		
		int ivm = iphiRaw * d_te; // Which VM
		
	// TE Inner
	// No stubs for DISK2S
		if ((teimask != 0) && (!disk2S)) {

			VMStubTEInner<OutType> stubTeInner;

			stubTeInner.setBend(bend);
			stubTeInner.setIndex(typename VMStubTEInner<OutType>::VMSTEIID(i));

			constexpr auto vmbits =
					(Layer) ? vmbitslayer[Layer - 1] : vmbitsdisk[Disk - 1]; // Number of bits for VMs
			constexpr auto finephibits =
					(Layer) ? nfinephibarrelinner : nfinephidiskinner; // Number of bits for finephi

	// The z/r information bits saved for TE Inner memories.
	// Which VMs to look at in the outer layer.
	// Note: not the z/r coordinate for the inner stub
	// Called binlookup in emulation
			int rzbits;

	// Layer
			if (Layer != 0) {

				constexpr auto zbins = (1 << nzbitsinnertablelayer); // Number of bins in z
				constexpr auto rbins = (1 << nrbitsinnertablelayer); // Number of bins in r
				ap_uint<nzbitsinnertablelayer> zbin = (z + (1 << (nzbits - 1)))
						>> (nzbits - nzbitsinnertablelayer); // Make z positive and take the 7 (nzbitsinnertablelayer) MSBs
				ap_uint<nrbitsinnertablelayer> rbin = (r + (1 << (nrbits - 1)))
						>> (nrbits - nrbitsinnertablelayer);

				int index = zbin * rbins + rbin; // Index for rzbits LUT

				rzbits = rzbitsinnertable[index];

				stubTeInner.setZBits(rzbits);
				stubTeInner.setFinePhi(
						iphivmFineBins<InType>(stubPhi, vmbits, finephibits));

			} else { // Disk
				assert(Disk != 0);

				constexpr int zbins = (1 << nzbitsinnertabledisk); // Number of bins in z
				constexpr int rbins = (1 << nrbitsinnertabledisk); // Number of bins in r
				ap_uint<nzbitsinnertabledisk> zbin = (z + (1 << (nzbits - 1)))
						>> (nzbits - nzbitsinnertabledisk); // Make z positive and take the 7 (nzbitsinnertablelayer) MSBs
				ap_uint<nrbitsinnertabledisk> rbin = r
						>> (nrbits - nrbitsinnertabledisk);

	// Special case if negative disk
				if (negdisk)
					zbin = 7 - zbin;

				int index = rbin * zbins + zbin; // Index for rzbits LUT

				rzbits = rzbitsinnertable[index];

				stubTeInner.setZBits(rzbits);
				stubTeInner.setFinePhi(
						iphivmFineBins<InType>(stubPhi, vmbits, finephibits));
			}
			// For debugging
			#ifndef __SYNTHESIS__
						std::cout << "TEInner stub " << std::hex << stubTeInner.raw()
								<< std::endl;
						std::cout << "ivm: " << std::dec << ivm <<std::endl
								<< std::endl;
			#endif // DEBUG



			// Write the TE Inner stub to the correct memory
			// Only if it has a valid rzbits/binlookup value, less than 1024 (table uses 1048575 as "-1"),
			// and a valid bend
			// TODO: remove hardcoded value
			if (rzbits <= 1024 && teimask[ivm]) {
				ap_uint<3> memindex = ivm-firstte; // Index for the correct memory in memory array
				ap_uint<6> bendindex = memindex*MaxTEICopies; // Index for bendcut LUTs
				for (int n = 0; n < MaxTEICopies; n++) {
					#pragma HLS UNROLL
					bool passbend = bendtable[bendindex][bend];
					if (passbend) {
						teiMemories[memindex][n].write_mem(bx, stubTeInner, addrCountTEI[memindex][n]);
						addrCountTEI[memindex][n] += 1; // Count the memory addresses we have written to
					}
					bendindex++; // Use next bencut table for the next memory "copy"
				}
			}
		} 
// END TE INNER



// OVERLAP
		if (olmask != 0) {

			assert(Layer == 1 || Layer == 2); // Make sure that only run layer 1 and 2

			constexpr auto zbins = (1 << nzbitsrzbitsoverlaptable); // Number of bins in z
			constexpr auto rbins = (1 << nrbitsrzbitsoverlaptable); // Number of bins in r
			ap_uint<nzbitsrzbitsoverlaptable> zbin = (z + (1 << (nzbits - 1)))
					>> (nzbits - nzbitsrzbitsoverlaptable); // Make z positive and take the 7 MSBs
			ap_uint<nrbitsrzbitsoverlaptable> rbin = (r + (1 << (nrbits - 1)))
					>> (nrbits - nrbitsrzbitsoverlaptable);

			int index = zbin * rbins + rbin; // Index for rzbitsoverlaptable

			int overlap = rzbitsoverlaptable[index];

			if (overlap != 1023) { // which is like "-1" if we had signed stuff...

				constexpr auto ntotvmol =
						(Layer) ? nallstubslayers[Layer - 1] * 2 : 0; // Always 2 overlap vms?

				VMStubTEInner<BARRELOL> stubOL;

// 16 overlap vms per layer
				auto iphiRawOl = iphivmRaw<InType>(stubPhi) >> 1; // Top 4 bits of phi
				const ap_ufixed<4, 3> d2 = ntotvmol / 16.; // Some normalisation thing
				int ivmol = iphiRawOl * d2; // Which VM it belongs to

				constexpr auto vmbits = (Layer) ? vmbitsoverlap[Layer - 1] : 0; // Number of bits used for VMs

				stubOL.setBend(bend);
				stubOL.setIndex(typename VMStubTEInner<BARRELOL>::VMSTEIID(i));
				stubOL.setZBits(overlap);
				stubOL.setFinePhi(
						iphivmFineBins<InType>(stubPhi, vmbits,
								nfinephioverlapinner));


// For debugging
#ifndef __SYNTHESIS__
				std::cout << "Overlap stub " << overlap << " " << std::hex
						<< stubOL.raw() << std::endl;
				std::cout << "ivm: " << std::dec << ivmol << std::endl
						<< std::endl;
#endif // DEBUG

				// Save stub to Overlap memories
				if (olmask[ivmol]) {
					int memindex = ivmol - firstol; // The memory index in array and addrcount
					int bendindex = memindex*MaxOLCopies; // Index for bendcut LUTs
					for (int n = 0; n < MaxOLCopies; n++) {
						#pragma HLS UNROLL
						bool passbend = bendoverlaptable[bendindex+n][bend];
						if (passbend) {
							olMemories[memindex][n].write_mem(bx, stubOL, addrCountOL[memindex][n]);
							addrCountOL[memindex][n] += 1;
						}
					}
				}
			} else {
				#ifndef __SYNTHESIS__
				std::cout << "NO OVERLAP" << std::endl << std::endl;
				#endif // DEBUG
			}
		}



// TE Outer
		if ((teomask != 0) && (!disk2S)) {
			VMStubTEOuter<OutType> stubTeOuter;

			stubTeOuter.setBend(bend);
			stubTeOuter.setIndex(typename VMStubTEOuter<OutType>::VMSTEOID(i));

			ap_uint<TEBinsBits> bin; // 3 bits, i.e. max 8 bins within each VM

			constexpr auto vmbits =
					(Layer) ? vmbitslayer[Layer - 1] : vmbitsdisk[Disk - 1]; // Number of bits for VMs
			constexpr auto finephibits =
					(Layer) ? nfinephibarrelouter : nfinephidiskouter; // Number of bits for finephi

// Layer
			if (Layer != 0) {

				stubTeOuter.setFinePhi(
						iphivmFineBins<InType>(stubPhi, vmbits, finephibits)); // is this the right vmbits

				constexpr auto zbins = (1 << nzbitsoutertablelayer); // Number of bins in z
				constexpr auto rbins = (1 << nrbitsoutertablelayer); // Number of bins in r
				ap_uint<nzbitsoutertablelayer> zbin = (z + (1 << (nzbits - 1)))
						>> (nzbits - nzbitsoutertablelayer); // Make z positive and take the 7 MSBs
				ap_uint<nrbitsoutertablelayer> rbin = (r + (1 << (nrbits - 1)))
						>> (nrbits - nrbitsoutertablelayer);

// Find the VM bit information in rzbits LUT
// First 3 MSB is the binning in z, and the 3 LSB is the fine z within a bin
				auto index = zbin * rbins + rbin; // number of bins
				ap_uint<6> rzbits = rzbitsoutertable[index];

				bin = rzbits >> 3; // Remove the 3 LSBs, i.e. the finebin bits

// Set fine z
				ap_uint<3> zfine = rzbits & 7;
				stubTeOuter.setFineZ(zfine);

			} else { // DiskS
				assert(Disk != 0);

				stubTeOuter.setFinePhi(
						iphivmFineBins<InType>(stubPhi, vmbits, finephibits));

				constexpr auto zbins = (1 << nzbitsoutertabledisk); // Number of bins in z
				constexpr auto rbins = (1 << nrbitsoutertabledisk); // Number of bins in r
				ap_uint<nzbitsoutertabledisk> zbin = (z + (1 << (nzbits - 1)))
						>> (nzbits - nzbitsoutertabledisk); // Make z positive and take the 7 MSBs
				ap_uint<nrbitsoutertabledisk> rbin = r
						>> (nrbits - nrbitsoutertabledisk);

// Special case if negative disk
				if (negdisk) {
					zbin = 7 - zbin;
				}

// Find the VM bit information in rzbits LUT
// First 2 MSB is the binning in r, and the 3 LSB is the fine r within a bin
				auto index = rbin * zbins + zbin; // Index for LUT
				ap_uint<5> rzbits = rzbitsoutertable[index];

				bin = rzbits >> 3; // Remove the 3 LSBs, i.e. the finebin bits

// Bin 4-7 is used for negative disks
				if (negdisk)
					bin += 4;

// Set fine r
				ap_uint<3> rfine = rzbits & 7;
				stubTeOuter.setFineZ(rfine);
			}

// For debugging
#ifndef __SYNTHESIS__
			std::cout << "TEOuter stub " << std::hex << stubTeOuter.raw()
					<< std::endl;
			std::cout << "ivm: " << std::dec << ivm << "       to bin " << bin << std::endl
					<< std::endl;
#endif // DEBUG

			// Write the TE Outer stub to the correct memory
			// Only if it has a valid bend
			// TODO: implement VMR to write to the n memory copies, which are different depending on the bendcuts
			if (teomask[ivm]) {
				int memindex = ivm-firstte; // Index for the correct memory in memory array and address
				int bendindex = memindex*MaxTEOCopies; // Index for bendcut LUTs
					for (int n = 0; n < MaxTEOCopies; n++) {
						#pragma HLS UNROLL
						bool passbend = bendoutertable[bendindex+n][bend]; // Check if stub passes bend cut
						if (passbend) {
							teoMemories[memindex][n].write_mem(bx, bin, stubTeOuter);
						}
					}
				}
		}

// executeTE() END   ------------------------------
 
	} // outside loop
	
} // VMRouter

#endif //VMROUTER_H
