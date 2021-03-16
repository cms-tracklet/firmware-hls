#include "MatchEngine.h"

void readTable(ap_uint<1> table[LSIZE]){
	ap_uint<1> tmp[LSIZE]=
	#if LAYER == 1
#include "../emData/ME/tables/METable_L1.tab"
	#elif LAYER == 2
#include "../emData/ME/tables/METable_L2.tab"
	#elif LAYER == 3
#include "../emData/ME/tables/METable_L3.tab"
	#elif LAYER == 4
#include "../emData/ME/tables/METable_L4.tab"
	#elif LAYER == 5
#include "../emData/ME/tables/METable_L5.tab"
	#elif LAYER == 6
#include "../emData/ME/tables/METable_L6.tab"
	#else
{};
	#endif

	for (int i=0;i<LSIZE;i++){
		table[i]=tmp[i];
	}
}

//template<int L, regionType VMSMEType>
template<int L, int VMSMEType, int VMPMEType>
void MatchEngine(const BXType bx, BXType& bx_o,
				 const VMStubMEMemory<VMSMEType, NBITBIN>& inputStubData,
				 const VMProjectionMemory<VMPMEType>& inputProjectionData,
				 CandidateMatchMemory& outputCandidateMatch) {
#pragma HLS inline
	//
	//Initialize table for bend-rinv consistency
	//
	ap_uint<1> table[LSIZE];
	readTable(table);

	//
	// Set up a FIFO based on a circular buffer structure.
	// Projection memory is read and if projections points to nonempty zbin for the stubs it is stored on this buffer.
	// The projection reading will stop if buffer is full and continue after the buffer is drained.
	// Each element consists of
	//   * kNBits_BufferAddr is the number of bits to handle buffer index (i.e. buffer size will be 1<<kNBits_BufferAddr).
	//   * kBufferDataSize is the size of each element in the buffer. The element data consists of, in order of MSB to LSB:
	//       [# of stubs in z-bin][projection data][index of z-bin][z-bin flag]
	//
	ap_uint<kBufferDataSize> projectionBuffer[1<<kNBits_BufferAddr];
	#pragma HLS ARRAY_PARTITION variable=projectionBuffer complete dim=0
	ap_uint<kNBits_BufferAddr> head_writeindex = 0;	// handles current buffer index for writing
	ap_uint<kNBits_BufferAddr> tail_readindex = 0;	// handles current buffer index for reading

	// The next projection to read, the number of projections and flag if we have more projections to read
	ap_uint<kNBits_MemAddr> iprojection = 0;
	auto const nproj = inputProjectionData.getEntries(bx);
	bool moreProjectionsAvailable = iprojection < nproj;

	// Variables for the projection
	typename VMProjection<VMPMEType>::VMPID projindex;
	typename VMProjection<VMPMEType>::VMPFINEZ projfinez;
	typename VMProjection<VMPMEType>::VMPFINEPHI projfinephi;
	typename VMProjection<VMPMEType>::VMPRINV projrinv;
	ap_uint<MEBinsBits> zbin = 0;
	ap_uint<kNBits_MemAddr> ncmatch = 0;
	bool isPSseed;
	bool second;

	// Number of stubs for current zbin and the stub being processed on this clock
	ap_uint<kNBits_MemAddrBinned> nstubs=0;
	ap_uint<kNBits_MemAddrBinned> istub=0;
	#pragma HLS dependence variable=istub intra WAR true

#ifdef DEBUG
	std::cout << "ProjectionIndex\tStubIndex\t<=== (PASS/FAIL)" << std::endl;
#endif

	// Main processing loops starts here.
	// Seven iterations are subtracted so that the total latency is 108 clock
	// cycles. Pipeline rewinding does not currently work.
	STEP_LOOP: for (ap_uint<kNBits_MemAddr> istep=0; istep<kMaxProc - kMaxProcOffset(module::ME); istep++) {
		#pragma HLS PIPELINE II=1 rewind
		#pragma HLS DEPENDENCE variable=tail_readindex inter false

		// Pre-fetch an element from the buffer
		auto const qdata=projectionBuffer[tail_readindex];

		// The buffer is not full if 2 slots are available as we may write stubs for up to 2 z-bins
		ap_uint<kNBits_BufferAddr> head_writeindexplus     = head_writeindex+1;
		ap_uint<kNBits_BufferAddr> head_writeindexplusplus = head_writeindex+2;
		bool bufferNotFull = (head_writeindexplus!=tail_readindex) && (head_writeindexplusplus!=tail_readindex);

		// The buffer is not empty when current write index and read index are different
		// With this you have to assume the buffer will never be absolutely full
		bool bufferNotEmpty = head_writeindex != tail_readindex;
		
		// If we have more projections and the buffer is not full we read
		// next projection and put in buffer if there are stubs in the 
		// memory the to which the projection points
		if (moreProjectionsAvailable && bufferNotFull) {
			auto const iprojectiontmp=iprojection;
			auto const projectiondatatmp=inputProjectionData.read_mem(bx,iprojectiontmp);
			iprojection++;
			moreProjectionsAvailable=iprojection<nproj;

			// The first and last zbin the projection points to
			auto const projectionzbitstmp=projectiondatatmp.getZBin();
			ap_uint<MEBinsBits> zbinfirst=projectionzbitstmp.range(3,1);
			ap_uint<MEBinsBits> zbinlast=zbinfirst + projectionzbitstmp.range(0,0);

			// Check if there are stubs in the memory
			auto const nstubfirst = inputStubData.getEntries(bx,zbinfirst);
			auto const nstublast  = inputStubData.getEntries(bx,zbinlast);
			bool savefirst = (nstubfirst != 0);
			bool savelast  = (nstublast != 0) && projectionzbitstmp.range(0,0);
			auto const head_writeindex_tmp=head_writeindex;

			if (savefirst) {
				ap_uint<1> zero=0;
				ap_uint<MEBinsBits+1> tmp=zbinfirst.concat(zero);
				ap_uint<VMProjection<PROJECTIONTYPE>::kVMProjectionSize+MEBinsBits+1> tmp2=projectiondatatmp.raw().concat(tmp);
				projectionBuffer[head_writeindex_tmp] = nstubfirst.concat(tmp2);
			}
			if (savelast) {
				ap_uint<1> one=1;
				ap_uint<MEBinsBits+1> tmp=zbinlast.concat(one);
				ap_uint<VMProjection<PROJECTIONTYPE>::kVMProjectionSize+MEBinsBits+1> tmp2=projectiondatatmp.raw().concat(tmp);
				ap_uint<kNBits_BufferAddr> head_writeindex_tmp_last = head_writeindex_tmp+savefirst;
				projectionBuffer[head_writeindex_tmp_last] = nstublast.concat(tmp2);
			}
#ifdef DEBUG
			std::cout << "Writing " << savefirst+savelast << " z-bins ==> head_writeindex " << head_writeindex << "-->";
#endif
			head_writeindex = head_writeindex + savefirst + savelast;
#ifdef DEBUG
			std::cout << head_writeindex << std::endl;
#endif
		}

		// If the buffer is not empty we have a projection that we need to process ...
		if (bufferNotEmpty) {
			ap_uint<kNBits_MemAddrBinned> istubtmp=istub;

			//Need to read the information about the proj in the buffer
			second=qdata.range(ME::BitLocations::kVMMESecondMSB,ME::BitLocations::kVMMESecondLSB);
			nstubs=qdata.range(ME::BitLocations::kVMMENStubsMSB,ME::BitLocations::kVMMENStubsLSB);
			VMProjection<PROJECTIONTYPE> data(qdata.range(ME::BitLocations::kVMMEProjectionMSB,ME::BitLocations::kVMMEProjectionLSB));
			zbin=qdata.range(ME::BitLocations::kVMMEZBinMSB,ME::BitLocations::kVMMEZBinLSB);

			projindex=data.getIndex();
			projfinez=data.getFineZ();
			projfinephi=data.getFinePhi();
			projrinv=data.getRInv();
			isPSseed=data.getIsPSSeed();

			// Check if last stub, if so, go to next buffer entry 
			if (istub+1 >= nstubs){
			  istub = 0;
			  tail_readindex++;
			}
			else {
			  istub++;
			}

			// Read stub memory and extract data fields
			auto const stubadd     = zbin.concat(istubtmp);
			auto const stubdata    = inputStubData.read_mem(bx,stubadd);
			auto const stubindex   = stubdata.getIndex();
			auto const stubfinez   = stubdata.getFineZ();
			auto const stubfinephi = stubdata.getFinePhi();
			auto const stubbend    = stubdata.getBend();

			// Calculate fine z position
			ap_int<VMProjectionBase<PROJECTIONTYPE>::kVMProjFineZSize+1> projfinezadj = projfinez;
			if (second) projfinezadj = projfinezadj - kZAdjustment;
			ap_int<VMProjectionBase<PROJECTIONTYPE>::kVMProjFineZSize+1> idz          = stubfinez - projfinezadj;
			ap_int<VMProjectionBase<PROJECTIONTYPE>::kVMProjFinePhiSize+1> idphi      = projfinephi - stubfinephi;

			// Check if stub z position consistent
			bool pass = (isPSseed) ? (idz >= ME::StubZPositionBarrelConsistency::kPSMin && idz <= ME::StubZPositionBarrelConsistency::kPSMax)
								   : (idz >= ME::StubZPositionBarrelConsistency::k2SMin && idz <= ME::StubZPositionBarrelConsistency::k2SMax);

			// Check is stub phi positions are consistent
			bool passphi = ((idphi<ME::StubPhiPositionBarrelConsistency::kMin)&&(idphi>-ME::StubPhiPositionBarrelConsistency::kMin)) || 
			  ((idphi>ME::StubPhiPositionBarrelConsistency::kMax) || (idphi<-ME::StubPhiPositionBarrelConsistency::kMax));


			// Check if stub bend and proj rinv consistent
#ifdef DEBUG
			std::cout << projindex.to_string() << "\t" << stubindex.to_string() << "\t<=== ";
#endif
			auto const index=projrinv.concat(stubbend);
			if ( passphi && pass && table[index]) {
				CandidateMatch cmatch(projindex.concat(stubindex));
				outputCandidateMatch.write_mem(bx,cmatch,ncmatch);
				ncmatch++;
#ifdef DEBUG
				std::cout << "PASS -- " << ncmatch-1 << "\n";
#endif
			}
#ifdef DEBUG
			else {
				std::cout << "FAIL" << "\n";
			}
			std::cout << "\ttail_readindex: " << tail_readindex << "\n"
					  << "\thead_writeindex: " << head_writeindex << "\n"
					  << "\tsecond: " << second << "\n"
					  << "\tprojfinez: " << projfinez << "\n"
					  << "\tprojfinezadj: " << projfinezadj << "\n"
					  << "\tstubfinez: " << stubfinez << "\n"
					  << "\tidz: " << idz.to_string() << "\n"
					  << "\tisPSseed: " << isPSseed << "\n"
					  << "\tpass: " << pass << "\n"
					  << "\tindex: " << index.get().to_string() << "\n"
					  << "\ttable[index]: " << table[index] << "\n"
					  << "\tnstubs:" << nstubs << "\n"
					  << "\tistub:" << istub << std::endl;
#endif
		}
	}

	bx_o = bx;
}

void MatchEngineTop(const BXType bx, BXType& bx_o,
					const VMStubMEMemory<MODULETYPE, NBITBIN>& inputStubData,
					const VMProjectionMemory<PROJECTIONTYPE>& inputProjectionData,
					CandidateMatchMemory& outputCandidateMatch) {

#pragma HLS interface register port=bx_o
#pragma HLS resource variable=inputStubData->get_mem() latency=2
#pragma HLS resource variable=inputProjectionData->get_mem() latency=2

	MatchEngine<LAYER,MODULETYPE,PROJECTIONTYPE>(bx, bx_o, inputStubData, inputProjectionData, outputCandidateMatch); 
}
