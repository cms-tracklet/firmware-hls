#ifndef TrackletAlgorithm_MatchCalculator_h
#define TrackletAlgorithm_MatchCalculator_h

// MatchCalculator

#include "Constants.h"
#include "CandidateMatchMemory.h"
#include "AllStubMemory.h"
#include "AllProjectionMemory.h"
#include "FullMatchMemory.h"

//////////////////////////////////////////////////////////////

template<int layer, int part>
void merger(
     // inputs
     CandidateMatch inA,
     bool validA,
     CandidateMatch inB,
     bool validB,
     CandidateMatch out,
     bool vout,
     bool inread,
     CandidateMatch A,
     bool vA,
     bool sA,
     CandidateMatch B,
     bool vB,
     bool sB,
     // outputs
     CandidateMatch * Anext,
     bool * vAnext,
     bool * sAnext,
     CandidateMatch * Bnext,
     bool * vBnext,
     bool * sBnext,
     CandidateMatch * outnext,
     bool * voutnext,
     bool * readA,
     bool * readB
){

#pragma HLS inline 
#pragma HLS pipeline II=1 rewind
#pragma HLS interface ap_ctrl_none port=return 
#pragma HLS interface ap_ctrl_none port=inA,validA,inB,validB,out,vout,inread,A,vA,sA,B,vB,sB 
#pragma HLS interface ap_ctrl_none port=Anext,Bnext,vAnext,vBnext,sAnext,sBnext,voutnext 

    // Set read enables for A and B
    *readA = (((inread || !vout) && sA) || !vA) && validA;
    *readB = (((inread || !vout) && sB) || !vB) && validB;

    // Setup state machine
    static enum {HOLD, PROC_A, PROC_B, START, DONE} state;
    if (sA && (inread || !vout))                          state = PROC_A;
    else if (sB && (inread || !vout))                     state = PROC_B; 
    else if ((!sA && !sB) && (validA || validB) && !vout) state = START;
    else if (!sB && !sA && vout && inread)                state = DONE;              
    else                                                  state = HOLD; 

    //------------- Explanation of the states -------------
    // START:  Either inA or inB is valid & there is no valid output & neither sA or sB is set
    //         No output yet, but set the next sA and sB and pipeline the inputs  
    // PROC_A: sA is set & either there is an inread from the next layer or not valid output
    //         Output is the pipelined A, and set the next sA and sB   
    // PROC_B: sB is set & either there is an inread from the next layer or not valid output
    //         Output is the pipelined B, and set the next sA and sB
    // DONE:   There is an valid output & there is no inread from the next layer and neither sA or sB is set
    //         No output, and set all reads to false
    // HOLD:   In all other cases, pipeline everthing 
    //-----------------------------------------------------

    // Case statements
    switch(state)
    {
    case PROC_A: // just readA and compare inA with pipelined B
        *outnext  = A;      // output is A
        *voutnext = vA;     // output valid is vA
    	*Anext    = inA;    // pipeline inA
    	*vAnext   = validA; // pipeline inA valid
        *Bnext    = B;      // pipeline B
        *vBnext   = vB;     // pipeline vB
        *sAnext   = ((inA.getProjIndex() <= B.getProjIndex()) || !vB) && validA;  // sA=true if inA is valid and (inA <= B or B not valid)
    	*sBnext   = (!(inA.getProjIndex() <= B.getProjIndex()) || !validA) && vB; // sB=true if B is valid and (inA > B or inA not valid)
        break;
    case PROC_B: // just readB and compare inB with pipelined A
        *outnext  = B;      // output is B
    	*voutnext = vB;     // output valid is vB
    	*Bnext    = inB;    // pipeline inB
    	*vBnext   = validB; // pipeline inB valid
        *Anext    = A;      // pipeline A
        *vAnext   = vA;     // pipeline vA
        *sAnext   = ((A.getProjIndex() <= inB.getProjIndex()) || !validB) && vA;  // sA=true if A is valid and (A <= inB or inB not valid) 
        *sBnext   = (!(A.getProjIndex() <= inB.getProjIndex()) || !vA) && validB; // sB=true if inB is valid and (A > inB or A not valid)
        break;
    case START: // both in at same time
        *outnext  = CandidateMatch();
    	*voutnext = false;
    	*Anext    = inA;    // pipeline inA
    	*vAnext   = validA; // pipeline inA valid
    	*Bnext    = inB;    // pipeline inB
    	*vBnext   = validB; // pipeline inB valid
        *sAnext   = ((inA.getProjIndex() <= inB.getProjIndex()) || !validB) && validA;  // sA=true if inA is valid and (inA <= inB or inB not valid)
        *sBnext   = (!(inA.getProjIndex() <= inB.getProjIndex()) || !validA) && validB; // sB=true if inB is valid and (inA > inB or inA not valid)
        break;
    case DONE: // set everything to false 
        *outnext  = CandidateMatch();
        *voutnext = false;
        *Anext    = CandidateMatch();
        *vAnext   = false;
        *Bnext    = CandidateMatch();
        *vBnext   = false;
        *sAnext   = false;
        *sBnext   = false;
        break;
    case HOLD: // pipeline all 
        *outnext  = out;
        *voutnext = vout;
        *Anext    = A;
        *Bnext    = B;
        *vAnext   = vA;
        *vBnext   = vB;
        *sAnext   = sA;
        *sBnext   = sB; 
        break;
    }

}

//////////////////////////////////////////////////////////////

// Absolute value template

template< int width >
ap_uint<width> iabs( ap_int<width> value )
{
  ap_uint<width> absval;
  if (value < 0) absval = -value;
  else           absval = value;
  return absval;
};

//////////////////////////////////////////////////////////////

// Template to get look up tables

// Table for phi or z cuts
template<bool phi, TF::layer L, int width, int depth>
void readTable_Cuts(ap_uint<width> table[depth]){
  if (phi){ // phi cuts
    if (L==TF::L1){
      ap_uint<width> tmp[depth] =
#include "../emData/MC/tables/MC_L1PHIC_phicut.tab"
      for (int i = 0; i < depth; i++) table[i] = tmp[i];
    }
    else if (L==TF::L2){
      ap_uint<width> tmp[depth] =
#include "../emData/MC/tables/MC_L2PHIC_phicut.tab"
      for (int i = 0; i < depth; i++) table[i] = tmp[i];
    }
    else if (L==TF::L3){
      ap_uint<width> tmp[depth] =
#include "../emData/MC/tables/MC_L3PHIC_phicut.tab"
      for (int i = 0; i < depth; i++) table[i] = tmp[i];
    }
    else if (L==TF::L4){
      ap_uint<width> tmp[depth] =
#include "../emData/MC/tables/MC_L4PHIC_phicut.tab"
      for (int i = 0; i < depth; i++) table[i] = tmp[i];
    }
    else if (L==TF::L5){
      ap_uint<width> tmp[depth] =
#include "../emData/MC/tables/MC_L5PHIC_phicut.tab"
      for (int i = 0; i < depth; i++) table[i] = tmp[i];
    }
    else if (L==TF::L6){
      ap_uint<width> tmp[depth] =
#include "../emData/MC/tables/MC_L6PHIC_phicut.tab"
      for (int i = 0; i < depth; i++) table[i] = tmp[i];
    }
    else {
      static_assert(true, "Only LAYERS 1 to 6 are valid");
    }
  } // end phi cuts
  else { // z cuts
    if (L==TF::L1){
      ap_uint<width> tmp[depth] =
#include "../emData/MC/tables/MC_L1PHIC_zcut.tab"
      for (int i = 0; i < depth; i++) table[i] = tmp[i];
    }
    else if (L==TF::L2){
      ap_uint<width> tmp[depth] =
#include "../emData/MC/tables/MC_L2PHIC_zcut.tab"
      for (int i = 0; i < depth; i++) table[i] = tmp[i];
    }
    else if (L==TF::L3){
      ap_uint<width> tmp[depth] =
#include "../emData/MC/tables/MC_L3PHIC_zcut.tab"
      for (int i = 0; i < depth; i++) table[i] = tmp[i];
    }
    else if (L==TF::L4){
      ap_uint<width> tmp[depth] =
#include "../emData/MC/tables/MC_L4PHIC_zcut.tab"
      for (int i = 0; i < depth; i++) table[i] = tmp[i];
    }
    else if (L==TF::L5){
      ap_uint<width> tmp[depth] =
#include "../emData/MC/tables/MC_L5PHIC_zcut.tab"
      for (int i = 0; i < depth; i++) table[i] = tmp[i];
    }
    else if (L==TF::L6){
      ap_uint<width> tmp[depth] =
#include "../emData/MC/tables/MC_L6PHIC_zcut.tab"
      for (int i = 0; i < depth; i++) table[i] = tmp[i];
    }
    else {
      static_assert(true, "Only LAYERS 1 to 6 are valid");
    }
 
  }

} // end readTable_Cuts


//////////////////////////////////////////////////////////////

// MatchCalculator
namespace MC {
  enum imc {UNDEF_ITC, A = 0, B = 1, C = 2, D = 3, E = 4, F = 5, G = 6, H = 7, I = 8, J = 9, K = 10, L = 11, M = 12, N = 13, O = 14};
}
template<TF::layer, MC::imc, TF::seed> constexpr bool FMMask();
#include "MatchCalculator_parameters.h"

template<regionType ASTYPE, regionType APTYPE, regionType FMTYPE, int MaxMatchCopies, int MaxFullMatchCopies, TF::layer LAYER=TF::L1, TF::disk DISK=TF::D1, MC::imc PHISEC=MC::A>
void MatchCalculator(BXType bx,
                     const CandidateMatchMemory match[MaxMatchCopies],
                     const AllStubMemory<ASTYPE>* allstub,
                     const AllProjectionMemory<APTYPE>* allproj,
                     BXType& bx_o,
                     FullMatchMemory<FMTYPE> fullmatch[MaxFullMatchCopies]
){
#pragma HLS inline
#pragma HLS array_partition variable=match complete dim=1
#pragma HLS array_partition variable=fullmatch complete dim=1

  // Initialization
 
  // Setup constants depending on which layer/disk working on
  // probably should move these to constants file
  const ap_uint<4> kNbitszprojL123 = 12; // nbitszprojL123 in emulation (defined in constants) 
  const ap_uint<4> kNbitszprojL456 = 8;  // nbitszprojL456 in emulation (defined in constants)
  const ap_uint<5> kNbitsdrinv = 19;     // idrinvbits     in emulation (defined in constants)
  const ap_uint<4> kShift_Rinv = 13;     // rinvbitshift   in emulation (defined in constants)
  const ap_uint<3> kShift_Phider = 7;    // phiderbitshift in emulation (defined in constants)
  const ap_uint<3> kNbitsrL123 = 7;      // nbitsrL123     in emulation (defined in constants)
  const ap_uint<3> kNbitsrL456 = 7;      // nbitsrL456     in emulation (defined in constants) 
  const ap_int<4>  kShift_PS_zderL = -7; // PS_zderL_shift in emulation (defined in constants)
  const ap_int<4>  kShift_2S_zderL = -7; // SS_zderL_shift in emulation (defined in constants)

  const auto kFact               = (LAYER < TF::L4)? 1 : (1<<(kNbitszprojL123-kNbitszprojL456)); // fact_ in emulation defined in MC
  const auto kPhi0_shift         = (LAYER < TF::L4)? 3 : 0;                                      // phi0shift_ in emulation defined in MC
  const auto kShift_phi0bit      = 1;                                                             // phi0bitshift in emulation defined in constants
  const ap_uint<10> kPhi_corr_shift_L123 = 7 + kNbitsdrinv + kShift_phi0bit - kShift_Rinv - kShift_Phider;                    // icorrshift for L123
  const ap_uint<10> kPhi_corr_shift_L456 = kPhi_corr_shift_L123 - 10 + kNbitsrL456;                                           // icorrshift for L456
  const auto kPhi_corr_shift     = (LAYER < TF::L4)? kPhi_corr_shift_L123 : kPhi_corr_shift_L456;                            // icorrshift_ in emulation
  const ap_uint<10> kZ_corr_shiftL123 = (-1-kShift_PS_zderL);                                                                 // icorzshift for L123 (6 in L3)
  const ap_uint<10> kZ_corr_shiftL456 = (-1-kShift_2S_zderL + kNbitszprojL123 - kNbitszprojL456 + kNbitsrL456 - kNbitsrL123); // icorzshift for L456
  const auto kZ_corr_shift       = (LAYER < TF::L4)? kZ_corr_shiftL123 : kZ_corr_shiftL456;                                  // icorzshift_ in emulation

  const auto LUT_matchcut_phi_width = 17;
  const auto LUT_matchcut_phi_depth = 12;
  const auto LUT_matchcut_z_width = 13;
  const auto LUT_matchcut_z_depth = 12;

  // Setup look up tables for match cuts
  ap_uint<LUT_matchcut_phi_width> LUT_matchcut_phi[LUT_matchcut_phi_depth];
  readTable_Cuts<true,LAYER,LUT_matchcut_phi_width,LUT_matchcut_phi_depth>(LUT_matchcut_phi);
  ap_uint<LUT_matchcut_z_width> LUT_matchcut_z[LUT_matchcut_z_depth];
  readTable_Cuts<false,LAYER,LUT_matchcut_z_width,LUT_matchcut_z_depth>(LUT_matchcut_z);

  // Initialize MC delta phi cut variables
  ap_uint<17> best_delta_phi;

  // Bool and ID needed for determining if processing a new tracklet
  CandidateMatch::CMProjIndex id;
  CandidateMatch::CMProjIndex id_next;

  ap_uint<kNBits_MemAddr> ncm1 = 0; 
  ap_uint<kNBits_MemAddr> ncm2 = 0;
  ap_uint<kNBits_MemAddr> ncm3 = 0;
  ap_uint<kNBits_MemAddr> ncm4 = 0;
  ap_uint<kNBits_MemAddr> ncm5 = 0;
  ap_uint<kNBits_MemAddr> ncm6 = 0;
  ap_uint<kNBits_MemAddr> ncm7 = 0;
  ap_uint<kNBits_MemAddr> ncm8 = 0;
  ap_uint<kNBits_MemAddr+1> total  = 0;
  ap_uint<kNBits_MemAddr> ncm = 0;

  // Initialize read addresses for candidate matches
  ap_uint<kNBits_MemAddr> addr1 = 0;  
  ap_uint<kNBits_MemAddr> addr2 = 0;  
  ap_uint<kNBits_MemAddr> addr3 = 0;  
  ap_uint<kNBits_MemAddr> addr4 = 0;  
  ap_uint<kNBits_MemAddr> addr5 = 0;  
  ap_uint<kNBits_MemAddr> addr6 = 0;  
  ap_uint<kNBits_MemAddr> addr7 = 0;  
  ap_uint<kNBits_MemAddr> addr8 = 0; 

  // Read signals for the input candidate matches
  bool read1 = false;
  bool read2 = false;
  bool read3 = false;
  bool read4 = false;
  bool read5 = false;
  bool read6 = false;
  bool read7 = false;
  bool read8 = false;

  // MC_L3PHIC mask {1: on, 0: off}
  //static const uint16_t FML1L2 = 1 << shift_L1L2;
  //static const uint16_t FML2L3 = 0 << shift_L2L3;
  //static const uint16_t FML3L4 = 0 << shift_L3L4;
  //static const uint16_t FML5L6 = 1 << shift_L5L6;
  //static const uint16_t FMD1D2 = 0 << shift_D1D2;
  //static const uint16_t FMD3D4 = 0 << shift_D3D4;
  //static const uint16_t FML1D1 = 0 << shift_L1D1;
  //static const uint16_t FML2D1 = 0 << shift_L2D1;

  // Variables for the merger
  // layer 1 variables
  bool read_L1_1 = false;
  bool read_L1_2 = false;
  bool read_L1_3 = false;
  bool read_L1_4 = false;
  CandidateMatch cm_L1_1 = CandidateMatch();
  CandidateMatch cm_L1_2 = CandidateMatch();
  CandidateMatch cm_L1_3 = CandidateMatch();
  CandidateMatch cm_L1_4 = CandidateMatch();
  CandidateMatch tmpA_L1_1 = CandidateMatch();
  CandidateMatch tmpA_L1_2 = CandidateMatch();
  CandidateMatch tmpA_L1_3 = CandidateMatch();
  CandidateMatch tmpA_L1_4 = CandidateMatch();
  CandidateMatch tmpB_L1_1 = CandidateMatch();
  CandidateMatch tmpB_L1_2 = CandidateMatch();
  CandidateMatch tmpB_L1_3 = CandidateMatch();
  CandidateMatch tmpB_L1_4 = CandidateMatch();
  bool valid_L1_1 = false; 
  bool valid_L1_2 = false; 
  bool valid_L1_3 = false; 
  bool valid_L1_4 = false; 
  bool vA_L1_1 = false; 
  bool vA_L1_2 = false; 
  bool vA_L1_3 = false; 
  bool vA_L1_4 = false; 
  bool vB_L1_1 = false; 
  bool vB_L1_2 = false; 
  bool vB_L1_3 = false; 
  bool vB_L1_4 = false; 
  bool sA_L1_1 = false; 
  bool sA_L1_2 = false; 
  bool sA_L1_3 = false; 
  bool sA_L1_4 = false; 
  bool sB_L1_1 = false; 
  bool sB_L1_2 = false; 
  bool sB_L1_3 = false; 
  bool sB_L1_4 = false; 

  // layer 2 variables
  bool read_L2_1 = false;
  bool read_L2_2 = false;
  CandidateMatch cm_L2_1 = CandidateMatch();
  CandidateMatch cm_L2_2 = CandidateMatch();
  CandidateMatch tmpA_L2_1 = CandidateMatch();
  CandidateMatch tmpA_L2_2 = CandidateMatch();
  CandidateMatch tmpB_L2_1 = CandidateMatch();
  CandidateMatch tmpB_L2_2 = CandidateMatch();
  bool valid_L2_1 = false; 
  bool valid_L2_2 = false; 
  bool vA_L2_1 = false; 
  bool vA_L2_2 = false; 
  bool vB_L2_1 = false; 
  bool vB_L2_2 = false; 
  bool sA_L2_1 = false; 
  bool sA_L2_2 = false; 
  bool sB_L2_1 = false; 
  bool sB_L2_2 = false; 

  // layer 3 variables
  CandidateMatch tmpA_L3;
  CandidateMatch tmpB_L3;
  bool valid_L3 = false;
  bool vA_L3 = false;
  bool vB_L3 = false;
  bool sA_L3 = false;
  bool sB_L3 = false;
  
  // Setup candidate match data stream that goes into match calculations
  CandidateMatch datastream = CandidateMatch();

  // Full match shift register to store best match
  typename AllProjection<APTYPE>::AProjTCSEED projseed;
  FullMatch<FMTYPE> bestmatch      = FullMatch<FMTYPE>();
  bool goodmatch                   = false;


  //-----------------------------------------------------------------------------------------------------------
  //-------------------------------- DATA PROCESSING STARTS ---------------------------------------------------
  //-----------------------------------------------------------------------------------------------------------
  // declare counters for each of the 8 output VMProj // !!!
  ap_uint<kNBits_MemAddr> nmcout1 = 0;
  ap_uint<kNBits_MemAddr> nmcout2 = 0;
  ap_uint<kNBits_MemAddr> nmcout3 = 0;
  ap_uint<kNBits_MemAddr> nmcout4 = 0;
  ap_uint<kNBits_MemAddr> nmcout5 = 0;
  ap_uint<kNBits_MemAddr> nmcout6 = 0;
  ap_uint<kNBits_MemAddr> nmcout7 = 0;
  ap_uint<kNBits_MemAddr> nmcout8 = 0;  
  MC_LOOP: for (ap_uint<kNBits_MemAddr> istep = 0; istep < kMaxProc - kMaxProcOffset(module::MC); istep++)
  {

#pragma HLS PIPELINE II=1 

    // Pick up number of candidate matches for each CM memory
    ncm1 = match[0].getEntries(bx);
    ncm2 = match[1].getEntries(bx);
    ncm3 = match[2].getEntries(bx);
    ncm4 = match[3].getEntries(bx);
    ncm5 = match[4].getEntries(bx);
    ncm6 = match[5].getEntries(bx);
    ncm7 = match[6].getEntries(bx);
    ncm8 = match[7].getEntries(bx);

    // Count up total number of CMs *and protect incase of overflow)
    total  = ncm1+ncm2+ncm3+ncm4+ncm5+ncm6+ncm7+ncm8; 
    ncm    = (total > kMaxProc)? kMaxProc : total.range(7,0);

    // pipeline variables
    bool read_L1_1_next = false;
    bool read_L1_2_next = false;
    bool read_L1_3_next = false;
    bool read_L1_4_next = false;
    CandidateMatch cm_L1_1_next = CandidateMatch();
    CandidateMatch cm_L1_2_next = CandidateMatch();
    CandidateMatch cm_L1_3_next = CandidateMatch();
    CandidateMatch cm_L1_4_next = CandidateMatch();
    CandidateMatch tmpA_L1_1_next = CandidateMatch();
    CandidateMatch tmpA_L1_2_next = CandidateMatch();
    CandidateMatch tmpA_L1_3_next = CandidateMatch();
    CandidateMatch tmpA_L1_4_next = CandidateMatch();
    CandidateMatch tmpB_L1_1_next = CandidateMatch();
    CandidateMatch tmpB_L1_2_next = CandidateMatch();
    CandidateMatch tmpB_L1_3_next = CandidateMatch();
    CandidateMatch tmpB_L1_4_next = CandidateMatch();
    bool valid_L1_1_next = false;
    bool valid_L1_2_next = false;
    bool valid_L1_3_next = false;
    bool valid_L1_4_next = false;
    bool vA_L1_1_next = false; 
    bool vA_L1_2_next = false; 
    bool vA_L1_3_next = false; 
    bool vA_L1_4_next = false; 
    bool vB_L1_1_next = false; 
    bool vB_L1_2_next = false; 
    bool vB_L1_3_next = false; 
    bool vB_L1_4_next = false; 
    bool sA_L1_1_next = false; 
    bool sA_L1_2_next = false; 
    bool sA_L1_3_next = false; 
    bool sA_L1_4_next = false; 
    bool sB_L1_1_next = false; 
    bool sB_L1_2_next = false; 
    bool sB_L1_3_next = false; 
    bool sB_L1_4_next = false; 

    bool read_L2_1_next = false;
    bool read_L2_2_next = false;
    CandidateMatch cm_L2_1_next = CandidateMatch();
    CandidateMatch cm_L2_2_next = CandidateMatch();
    CandidateMatch tmpA_L2_1_next = CandidateMatch();
    CandidateMatch tmpA_L2_2_next = CandidateMatch();
    CandidateMatch tmpB_L2_1_next = CandidateMatch();
    CandidateMatch tmpB_L2_2_next = CandidateMatch();
    bool valid_L2_1_next = false; 
    bool valid_L2_2_next = false; 
    bool vA_L2_1_next = false; 
    bool vA_L2_2_next = false; 
    bool vB_L2_1_next = false; 
    bool vB_L2_2_next = false; 
    bool sA_L2_1_next = false; 
    bool sA_L2_2_next = false; 
    bool sB_L2_1_next = false; 
    bool sB_L2_2_next = false; 

    CandidateMatch cm_L3_next;
    CandidateMatch tmpA_L3_next;
    CandidateMatch tmpB_L3_next; 
    bool valid_L3_next = false;
    bool vA_L3_next = false;
    bool vB_L3_next = false;
    bool sA_L3_next = false;
    bool sB_L3_next = false;
    bool read1_next = false;
    bool read2_next = false;
    bool read3_next = false;
    bool read4_next = false;
    bool read5_next = false;
    bool read6_next = false;
    bool read7_next = false;
    bool read8_next = false;


    //-----------------------------------------------------------------------------------------------------------
    //-------------------------------- MERGE INPUT CANDIDATE MATCHES --------------------------------------------
    //-----------------------------------------------------------------------------------------------------------

    // Increment the read addresses for the candidate matches

    if (read1) addr1++;
    if (read2) addr2++;
    if (read3) addr3++;
    if (read4) addr4++;
    if (read5) addr5++;
    if (read6) addr6++;
    if (read7) addr7++;
    if (read8) addr8++;

    // Read in each candidate match
    const CandidateMatch &cm1 = match[0].read_mem(bx,addr1);
    const CandidateMatch &cm2 = match[1].read_mem(bx,addr2);
    const CandidateMatch &cm3 = match[2].read_mem(bx,addr3); 
    const CandidateMatch &cm4 = match[3].read_mem(bx,addr4); 
    const CandidateMatch &cm5 = match[4].read_mem(bx,addr5); 
    const CandidateMatch &cm6 = match[5].read_mem(bx,addr6); 
    const CandidateMatch &cm7 = match[6].read_mem(bx,addr7); 
    const CandidateMatch &cm8 = match[7].read_mem(bx,addr8); 

    // Valid signal for the candidate match
    bool valid1 = (addr1 < ncm1) && (ncm1 > 0);
    bool valid2 = (addr2 < ncm2) && (ncm2 > 0);
    bool valid3 = (addr3 < ncm3) && (ncm3 > 0);
    bool valid4 = (addr4 < ncm4) && (ncm4 > 0);
    bool valid5 = (addr5 < ncm5) && (ncm5 > 0);
    bool valid6 = (addr6 < ncm6) && (ncm6 > 0);
    bool valid7 = (addr7 < ncm7) && (ncm7 > 0);
    bool valid8 = (addr8 < ncm8) && (ncm8 > 0);

    // merger Layer 1 Part 1
    merger<1,1>(
      cm1, valid1, cm2, valid2,                      // inputs: inA, validA, inB, validB
      cm_L1_1, valid_L1_1, read_L1_1,                // inputs: out, vout, inread from L2_1
      tmpA_L1_1, vA_L1_1, sA_L1_1,                   // tmp variables internal to L1_1 merger
      tmpB_L1_1, vB_L1_1, sB_L1_1,                   // tmp variables internal to L1_1 merger
      &tmpA_L1_1_next, &vA_L1_1_next, &sA_L1_1_next, // tmp variables internal to L1_1 merger
      &tmpB_L1_1_next, &vB_L1_1_next, &sB_L1_1_next, // tmp variables internal to L1_1 merger
      &cm_L1_1_next, &valid_L1_1_next,               // outputs: out, vout
      &read1_next, &read2_next                       // outputs: read1, read2 
    );

    // merger Layer 1 Part 2
    merger<1,2>(
      cm3, valid3, cm4, valid4,                      // inputs: inA, validA, inB, validB
      cm_L1_2, valid_L1_2, read_L1_2,                // inputs: out, vout, inread from L2_1
      tmpA_L1_2, vA_L1_2, sA_L1_2,                   // tmp variables internal to L1_2 merger
      tmpB_L1_2, vB_L1_2, sB_L1_2,                   // tmp variables internal to L1_2 merger
      &tmpA_L1_2_next, &vA_L1_2_next, &sA_L1_2_next, // tmp variables internal to L1_2 merger
      &tmpB_L1_2_next, &vB_L1_2_next, &sB_L1_2_next, // tmp variables internal to L1_2 merger
      &cm_L1_2_next, &valid_L1_2_next,               // outputs: out, vout
      &read3_next, &read4_next                       // outputs: read3, read4 
    );

    // merger Layer 1 Part 3 
    merger<1,3>(
      cm5, valid5, cm6, valid6,                      // inputs: inA, validA, inB, validB
      cm_L1_3, valid_L1_3, read_L1_3,                // inputs: out, vout, inread from L2_1
      tmpA_L1_3, vA_L1_3, sA_L1_3,                   // tmp variables internal to L1_3 merger
      tmpB_L1_3, vB_L1_3, sB_L1_3,                   // tmp variables internal to L1_3 merger
      &tmpA_L1_3_next, &vA_L1_3_next, &sA_L1_3_next, // tmp variables internal to L1_3 merger
      &tmpB_L1_3_next, &vB_L1_3_next, &sB_L1_3_next, // tmp variables internal to L1_3 merger
      &cm_L1_3_next, &valid_L1_3_next,               // outputs: out, vout
      &read5_next, &read6_next                       // outputs: read5, read6 
    );

    // merger Layer 1 Part 4  
    merger<1,4>(
      cm7, valid7, cm8, valid8,                      // inputs: inA, validA, inB, validB
      cm_L1_4, valid_L1_4, read_L1_4,                // inputs: out, vout, inread from L2_1
      tmpA_L1_4, vA_L1_4, sA_L1_4,                   // tmp variables internal to L1_4 merger
      tmpB_L1_4, vB_L1_4, sB_L1_4,                   // tmp variables internal to L1_4 merger
      &tmpA_L1_4_next, &vA_L1_4_next, &sA_L1_4_next, // tmp variables internal to L1_4 merger
      &tmpB_L1_4_next, &vB_L1_4_next, &sB_L1_4_next, // tmp variables internal to L1_4 merger
      &cm_L1_4_next, &valid_L1_4_next,               // outputs: out, vout
      &read7_next, &read8_next                       // outputs: read7, read8 
    );

    // merger Layer 2 Part 1
    merger<2,1>(
      cm_L1_1_next, valid_L1_1_next,                 // inputs: inA, validA
      cm_L1_2_next, valid_L1_2_next,                 // inputs: inB, validB
      cm_L2_1, valid_L2_1, read_L2_1,                // inputs: out, vout, inread from L3_1
      tmpA_L2_1, vA_L2_1, sA_L2_1,                   // tmp variables internal to L2_1 merger
      tmpB_L2_1, vB_L2_1, sB_L2_1,                   // tmp variables internal to L2_1 merger
      &tmpA_L2_1_next, &vA_L2_1_next, &sA_L2_1_next, // tmp variables internal to L2_1 merger
      &tmpB_L2_1_next, &vB_L2_1_next, &sB_L2_1_next, // tmp variables internal to L2_1 merger
      &cm_L2_1_next, &valid_L2_1_next,               // outputs: out, vout
      &read_L1_1_next, &read_L1_2_next               // outputs: read_L1_1, read_L1_2
    );

    // merger Layer 2 Part 2
    merger<2,2>(
      cm_L1_3_next, valid_L1_3_next,                 // inputs: inA, validA 
      cm_L1_4_next, valid_L1_4_next,                 // inputs: inB, validB
      cm_L2_2, valid_L2_2, read_L2_2,                // inputs: out, vout, inread from L3_1
      tmpA_L2_2, vA_L2_2, sA_L2_2,                   // tmp variables internal to L2_2 merger
      tmpB_L2_2, vB_L2_2, sB_L2_2,                   // tmp variables internal to L2_2 merger
      &tmpA_L2_2_next, &vA_L2_2_next, &sA_L2_2_next, // tmp variables internal to L2_2 merger
      &tmpB_L2_2_next, &vB_L2_2_next, &sB_L2_2_next, // tmp variables internal to L2_2 merger
      &cm_L2_2_next, &valid_L2_2_next,               // outputs: out, vout
      &read_L1_3_next, &read_L1_4_next               // outputs: read_L1_3, read_L1_4
    );

    // merger Layer 3 Part 1
    merger<3,1>(
      cm_L2_1_next, valid_L2_1_next,           // inputs: inA, validA 
      cm_L2_2_next, valid_L2_2_next,           // inputs: inB, validB
      datastream, valid_L3, true,              // inputs: out, vout, true inread because last layer
      tmpA_L3, vA_L3, sA_L3,                   // tmp variables internal to L3_1 merger
      tmpB_L3, vB_L3, sB_L3,                   // tmp variables internal to L3_1 merger
      &tmpA_L3_next, &vA_L3_next, &sA_L3_next, // tmp variables internal to L3_1 merger
      &tmpB_L3_next, &vB_L3_next, &sB_L3_next, // tmp variables internal to L3_1 merger
      &cm_L3_next, &valid_L3_next,             // outputs: out, vout
      &read_L2_1_next, &read_L2_2_next         // outputs: read_L2_1, read_L2_2
    );

    // pipeline the variables
    // set up the inputs for the next iteration of the loop

    read1      = read1_next;
    read2      = read2_next;
    read3      = read3_next;
    read4      = read4_next;
    read5      = read5_next;
    read6      = read6_next;
    read7      = read7_next;
    read8      = read8_next;

    read_L1_1  = read_L1_1_next;
    read_L1_2  = read_L1_2_next;
    read_L1_3  = read_L1_3_next;
    read_L1_4  = read_L1_4_next;
    read_L2_1  = read_L2_1_next;
    read_L2_2  = read_L2_2_next;

    cm_L1_1    = cm_L1_1_next;
    cm_L1_2    = cm_L1_2_next;
    cm_L1_3    = cm_L1_3_next;
    cm_L1_4    = cm_L1_4_next;
    cm_L2_1    = cm_L2_1_next;
    cm_L2_2    = cm_L2_2_next;
    datastream = cm_L3_next;

    valid_L1_1 = valid_L1_1_next;
    valid_L1_2 = valid_L1_2_next;
    valid_L1_3 = valid_L1_3_next;
    valid_L1_4 = valid_L1_4_next;
    valid_L2_1 = valid_L2_1_next;
    valid_L2_2 = valid_L2_2_next;
    valid_L3   = valid_L3_next;

    tmpA_L1_1 = tmpA_L1_1_next;
    tmpA_L1_2 = tmpA_L1_2_next;
    tmpA_L1_3 = tmpA_L1_3_next;
    tmpA_L1_4 = tmpA_L1_4_next;
    tmpB_L1_1 = tmpB_L1_1_next;
    tmpB_L1_2 = tmpB_L1_2_next;
    tmpB_L1_3 = tmpB_L1_3_next;
    tmpB_L1_4 = tmpB_L1_4_next;

    tmpA_L2_1 = tmpA_L2_1_next;
    tmpA_L2_2 = tmpA_L2_2_next;
    tmpB_L2_1 = tmpB_L2_1_next;
    tmpB_L2_2 = tmpB_L2_2_next;
    tmpA_L3   = tmpA_L3_next;
    tmpB_L3   = tmpB_L3_next;
    
    vA_L1_1   = vA_L1_1_next; 
    vA_L1_2   = vA_L1_2_next; 
    vA_L1_3   = vA_L1_3_next; 
    vA_L1_4   = vA_L1_4_next; 
    vB_L1_1   = vB_L1_1_next; 
    vB_L1_2   = vB_L1_2_next; 
    vB_L1_3   = vB_L1_3_next; 
    vB_L1_4   = vB_L1_4_next; 
    vA_L2_1   = vA_L2_1_next; 
    vA_L2_2   = vA_L2_2_next; 
    vB_L2_1   = vB_L2_1_next; 
    vB_L2_2   = vB_L2_2_next;
    vA_L3     = vA_L3_next;
    vB_L3     = vB_L3_next; 

    sA_L1_1   = sA_L1_1_next; 
    sA_L1_2   = sA_L1_2_next; 
    sA_L1_3   = sA_L1_3_next; 
    sA_L1_4   = sA_L1_4_next; 
    sB_L1_1   = sB_L1_1_next; 
    sB_L1_2   = sB_L1_2_next; 
    sB_L1_3   = sB_L1_3_next; 
    sB_L1_4   = sB_L1_4_next; 
    sA_L2_1   = sA_L2_1_next; 
    sA_L2_2   = sA_L2_2_next; 
    sB_L2_1   = sB_L2_1_next; 
    sB_L2_2   = sB_L2_2_next;
    sA_L3     = sA_L3_next;
    sB_L3     = sB_L3_next; 

    //-----------------------------------------------------------------------------------------------------------
    //-------------------------------------- MATCH CALCULATION STEPS --------------------------------------------
    //-----------------------------------------------------------------------------------------------------------

    // Extract the stub and projection indices from the candidate match
    auto projid = datastream.getProjIndex();
    auto stubid = datastream.getStubIndex();
    // Use the stub and projection indices to pick up the stub and projection
    AllProjection<APTYPE> proj = allproj->read_mem(bx,projid);
    AllStub<ASTYPE>       stub = allstub->read_mem(bx,stubid);

    // Check if processing a new tracklet or not 
    // Later we only want to store the single best match per tracklet
    id      = id_next; // pipelined id
    id_next = projid;  
    bool newtracklet = (istep==0 || (id_next != id))? true : false;

    // Stub parameters
    typename AllStub<ASTYPE>::ASR    stub_r    = stub.getR();
    typename AllStub<ASTYPE>::ASZ    stub_z    = stub.getZ();
    typename AllStub<ASTYPE>::ASPHI  stub_phi  = stub.getPhi();
    typename AllStub<ASTYPE>::ASBEND stub_bend = stub.getBend();       

    // Projection parameters
    typename AllProjection<APTYPE>::AProjTCID          proj_tcid = proj.getTCID();
    typename AllProjection<APTYPE>::AProjTrackletIndex proj_tkid = proj.getTrackletIndex();
    typename AllProjection<APTYPE>::AProjTCSEED        proj_seed = proj.getSeed();
    typename AllProjection<APTYPE>::AProjPHI           proj_phi  = proj.getPhi();
    typename AllProjection<APTYPE>::AProjRZ            proj_z    = proj.getRZ();
    typename AllProjection<APTYPE>::AProjPHIDER        proj_phid = proj.getPhiDer();
    typename AllProjection<APTYPE>::AProjRZDER         proj_zd   = proj.getRZDer(); 

    // Calculate residuals
    // Get phi and z correction
    ap_int<22> full_phi_corr = stub_r * proj_phid; // full corr has enough bits for full multiplication
    ap_int<18> full_z_corr   = stub_r * proj_zd;   // full corr has enough bits for full multiplication
    ap_int<11> phi_corr      = full_phi_corr >> kPhi_corr_shift;                        // only keep needed bits
    ap_int<12> z_corr        = (full_z_corr + (1<<(kZ_corr_shift-1))) >> kZ_corr_shift; // only keep needed bits
     
    // Apply the corrections
    const int kProj_phi_len = (APTYPE == BARRELPS) ? 15 : 18;
    ap_int<kProj_phi_len> proj_phi_corr = proj_phi + phi_corr;  // original proj phi plus phi correction
    ap_int<13> proj_z_corr   = proj_z + z_corr;      // original proj z plus z correction

    // Get phi and z difference between the projection and stub
    ap_int<10> delta_z         = stub_z - proj_z_corr;
    ap_int<14> delta_z_fact   = delta_z * kFact;
    ap_int<18> stub_phi_long  = stub_phi;         // make longer to allow for shifting
    ap_int<18> proj_phi_long  = proj_phi_corr;    // make longer to allow for shifting
    ap_int<18> shiftstubphi   = stub_phi_long << kPhi0_shift;                        // shift
    ap_int<18> shiftprojphi   = proj_phi_long << (kShift_phi0bit - 1 + kPhi0_shift); // shift
    ap_int<17> delta_phi      = shiftstubphi - shiftprojphi;
    ap_uint<13> abs_delta_z   = iabs<13>( delta_z_fact ); // absolute value of delta z
    ap_uint<17> abs_delta_phi = iabs<17>( delta_phi );    // absolute value of delta phi

    // Full match parameters
    const typename FullMatch<FMTYPE>::FMTCID          &fm_tcid  = proj_tcid;
    const typename FullMatch<FMTYPE>::FMTrackletIndex &fm_tkid  = proj_tkid;
    const typename FullMatch<FMTYPE>::FMSTUBPHIID     fm_asphi = PHISEC;
    const typename FullMatch<FMTYPE>::FMSTUBID        &fm_asid  = stubid;
    const typename FullMatch<FMTYPE>::FMSTUBR         &fm_stubr = stub_r;
    const typename FullMatch<FMTYPE>::FMPHIRES        fm_phi   = delta_phi;
    const typename FullMatch<FMTYPE>::FMZRES          fm_z     = delta_z;

    // Full match  
    FullMatch<FMTYPE> fm(fm_tcid,fm_tkid,fm_asphi,fm_asid,fm_stubr,fm_phi,fm_z);

    //-----------------------------------------------------------------------------------------------------------
    //-------------------------------------- BEST MATCH LOGIC BLOCK ---------------------------------------------
    //-----------------------------------------------------------------------------------------------------------
 
    typename AllProjection<APTYPE>::AProjTCSEED projseed_next;
    FullMatch<FMTYPE> bestmatch_next = FullMatch<FMTYPE>();
    bool goodmatch_next              = false;

    // For first tracklet, pick up the phi cut value
    best_delta_phi = (newtracklet)? LUT_matchcut_phi[proj_seed] : best_delta_phi;
  
    // Check that matches fall within the selection window of the projection 
    if ((abs_delta_z <= LUT_matchcut_z[proj_seed]) && (abs_delta_phi <= best_delta_phi)){
      // Update values of best phi parameters, so that the next match
      // will be compared to this value instead of the original selection cut
      best_delta_phi = abs_delta_phi;

      // Store bestmatch
      bestmatch_next = fm;
      goodmatch_next = true;
      projseed_next  = proj_seed;
    }
    else if (newtracklet){ // if is a new tracklet, do not make a match because it didn't pass the cuts
      bestmatch_next = FullMatch<FMTYPE>();
      goodmatch_next = false;
      projseed_next  = -1;
    }
    else { // if current match did not pass, but it is not a new tracklet, keep the previous best match for that tracklet
      bestmatch_next = bestmatch;
      goodmatch_next = goodmatch;
      projseed_next  = projseed;
    }

    if(newtracklet && goodmatch==true) { // Write out only the best match, based on the seeding 
      switch (projseed) {
        case 0:
        if(FMMask<LAYER, PHISEC, TF::L1L2>()) {
          fullmatch[0].write_mem(bx,bestmatch,nmcout1); // L1L2 seed
          nmcout1++;
        }
        break;
        case 1:
        if(FMMask<LAYER, PHISEC, TF::L2L3>()) {
          fullmatch[1].write_mem(bx,bestmatch,nmcout2); // L2L3 seed
          nmcout2++;
        }
        break;
        case 2:
        if(FMMask<LAYER, PHISEC, TF::L3L4>()) {
          fullmatch[2].write_mem(bx,bestmatch,nmcout3); // L3L4 seed
          nmcout3++;
        }
        break;
        case 3:
        if(FMMask<LAYER, PHISEC, TF::L5L6>()) {
          fullmatch[3].write_mem(bx,bestmatch,nmcout4); // L5L6 seed
          nmcout4++;
        }
        break;
        case 4:
        if(FMMask<LAYER, PHISEC, TF::D1D2>()) {
          fullmatch[4].write_mem(bx,bestmatch,nmcout5); // D1D2 seed
          nmcout5++;
        }
        break;
        case 5:
        if(FMMask<LAYER, PHISEC, TF::D3D4>()) {
          fullmatch[5].write_mem(bx,bestmatch,nmcout6); // D3D4 seed
          nmcout6++;
        }
        break;
        case 6:
        if(FMMask<LAYER, PHISEC, TF::L1D1>()) {
          fullmatch[6].write_mem(bx,bestmatch,nmcout7); // L1D1 seed
          nmcout7++;
        }
        break;
        case 7:
        if(FMMask<LAYER, PHISEC, TF::L2D1>()) {
          fullmatch[7].write_mem(bx,bestmatch,nmcout8); // L2D1 seed
          nmcout8++;
        }
        break;
      }
    }

    // pipeline the bestmatch registers
    bestmatch      = bestmatch_next;
    goodmatch      = goodmatch_next;
    projseed       = projseed_next;

  }// end MC_LOOP 

  bx_o = bx;

  /*
  // finish by writing out the last match made (to minimize truncation) -- can't do this because not in the loop 
  fullmatch[0].write_mem(bx,bestmatch,(goodmatch=true && projseed==0)); // L1L2 seed
  fullmatch[1].write_mem(bx,bestmatch,(goodmatch=true && projseed==1)); // L3L4 seed
  fullmatch[2].write_mem(bx,bestmatch,(goodmatch=true && projseed==2)); // L5L6 seed
  fullmatch[3].write_mem(bx,bestmatch,(goodmatch=true && projseed==3)); // D1D2 seed
  fullmatch[4].write_mem(bx,bestmatch,(goodmatch=true && projseed==4)); // D3D4 seed
  fullmatch[5].write_mem(bx,bestmatch,(goodmatch=true && projseed==5)); // L1D1 seed
  fullmatch[6].write_mem(bx,bestmatch,(goodmatch=true && projseed==6)); // L2D1 seed
  */

}// end MatchCalculator

#endif
