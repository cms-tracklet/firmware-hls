// Test bench for the MatchCalculator 
#include "MatchCalculatorTop.h"

#include "FileReadUtility.h"
#include "Constants.h"

#include "hls_math.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>


const int nevents = 100; // number of events to run
bool truncation = false; // compare results to truncated emulation

using namespace std;



int main() {
  // error counter
  int err_count = 0;

  // declare input memory arrays to be read from the emulation files
  static CandidateMatchMemory           match[maxMatchCopies];
  static AllStubMemory<BARRELPS>        allstub;
  static AllProjectionMemory<BARRELPS>  allproj;

  // declare output memory array to be filled by hls simulation
  static FullMatchMemory<BARREL> fullmatch[maxFullMatchCopies];

  // read in input files
  ifstream fin_as;
  ifstream fin_ap;
  ifstream fin_cm1;
  ifstream fin_cm2;
  ifstream fin_cm3;
  ifstream fin_cm4;
  ifstream fin_cm5;
  ifstream fin_cm6;
  ifstream fin_cm7;
  ifstream fin_cm8;

  if (not openDataFile(fin_as,"MC/MC_L3PHIC/AllStubs_AS_L3PHICn1_04.dat")) return -1;
  if (not openDataFile(fin_ap,"MC/MC_L3PHIC/AllProj_AP_L3PHIC_04.dat")) return -1;
  if (not openDataFile(fin_cm1,"MC/MC_L3PHIC/CandidateMatches_CM_L3PHIC17_04.dat")) return -1;
  if (not openDataFile(fin_cm2,"MC/MC_L3PHIC/CandidateMatches_CM_L3PHIC18_04.dat")) return -1;
  if (not openDataFile(fin_cm3,"MC/MC_L3PHIC/CandidateMatches_CM_L3PHIC19_04.dat")) return -1;
  if (not openDataFile(fin_cm4,"MC/MC_L3PHIC/CandidateMatches_CM_L3PHIC20_04.dat")) return -1;
  if (not openDataFile(fin_cm5,"MC/MC_L3PHIC/CandidateMatches_CM_L3PHIC21_04.dat")) return -1;
  if (not openDataFile(fin_cm6,"MC/MC_L3PHIC/CandidateMatches_CM_L3PHIC22_04.dat")) return -1;
  if (not openDataFile(fin_cm7,"MC/MC_L3PHIC/CandidateMatches_CM_L3PHIC23_04.dat")) return -1;
  if (not openDataFile(fin_cm8,"MC/MC_L3PHIC/CandidateMatches_CM_L3PHIC24_04.dat")) return -1;

  // open file(s) with reference results
  ifstream fout_fm1;
  ifstream fout_fm2;
  ifstream fout_fm3;
  ifstream fout_fm4;
  ifstream fout_fm5;
  ifstream fout_fm6;
  ifstream fout_fm7;
  ifstream fout_fm8;

  if (not openDataFile(fout_fm1,"MC/MC_L3PHIC/FullMatches_FM_L1L2_L3PHIC_04.dat")) return -1;
  //if (not openDataFile(fout_fm2,"MC/MC_L1PHIC/FullMatches_FM_L2L3_L1PHIC_04.dat")) return -1;
  //if (not openDataFile(fout_fm3,"MC/MC_L1PHIC/FullMatches_FM_L3L4_L1PHIC_04.dat")) return -1;
  if (not openDataFile(fout_fm4,"MC/MC_L3PHIC/FullMatches_FM_L5L6_L3PHIC_04.dat")) return -1;
  //if (not openDataFile(fout_fm4,"MC/MC_L1PHIC/FullMatches_FM_D1D2_L1PHIC_04.dat")) return -1;
  //if (not openDataFile(fout_fm5,"MC/MC_L1PHIC/FullMatches_FM_D3D4_L1PHIC_04.dat")) return -1;
  //if (not openDataFile(fout_fm6,"")) return -1;
  //if (not openDataFile(fout_fm7,"MC/MC_L1PHIC/FullMatches_FM_L2D1_L1PHIC_04.dat")) return -1;

  // loop over events
  for (int ievt = 0; ievt < nevents; ++ievt) {
    //cout << "Event: " << dec << ievt << endl;

    fullmatch[0].clear();
//    fullmatch[1].clear();
//    fullmatch[2].clear();
    fullmatch[3].clear();
//    fullmatch[4].clear();
//    fullmatch[5].clear();
//    fullmatch[6].clear();
//    fullmatch[7].clear();

    // make memories from the input text files
    writeMemFromFile<AllStubMemory<BARRELPS> >(allstub, fin_as, ievt);
    writeMemFromFile<AllProjectionMemory<BARRELPS> >(allproj, fin_ap, ievt);
    writeMemFromFile<CandidateMatchMemory>(match[0], fin_cm1, ievt);
    writeMemFromFile<CandidateMatchMemory>(match[1], fin_cm2, ievt);
    writeMemFromFile<CandidateMatchMemory>(match[2], fin_cm3, ievt);
    writeMemFromFile<CandidateMatchMemory>(match[3], fin_cm4, ievt);
    writeMemFromFile<CandidateMatchMemory>(match[4], fin_cm5, ievt);
    writeMemFromFile<CandidateMatchMemory>(match[5], fin_cm6, ievt);
    writeMemFromFile<CandidateMatchMemory>(match[6], fin_cm7, ievt);
    writeMemFromFile<CandidateMatchMemory>(match[7], fin_cm8, ievt);

    //set bunch crossing
    BXType bx = ievt;
    BXType bx_out;

    // Unit Under Test
    MatchCalculatorTop(
      bx, match, &allstub, &allproj, bx_out, fullmatch
    );

    // compare the computed outputs with the expected ones 
    //std::cout << "FM: L1L2 seeding" << std::endl;
    err_count += compareMemWithFile<FullMatchMemory<BARREL> >(fullmatch[0], fout_fm1, ievt, "FullMatch", truncation);
    //std::cout << "FM: L2L3 seeding" << std::endl;
    //err_count += compareMemWithFile<FullMatchMemory<BARREL> >(fullmatch[1], fout_fm2, ievt, "FullMatch", truncation);
    //std::cout << "FM: L3L4 seeding" << std::endl;
    //err_count += compareMemWithFile<FullMatchMemory<BARREL> >(fullmatch[2], fout_fm3, ievt, "FullMatch", truncation);
    //std::cout << "FM: L5L6 seeding" << std::endl;
    err_count += compareMemWithFile<FullMatchMemory<BARREL> >(fullmatch[3], fout_fm4, ievt, "FullMatch", truncation);
    //std::cout << "FM: D1D2 seeding" << std::endl;
    //err_count += compareMemWithFile<FullMatchMemory<BARREL> >(fullmatch[4], fout_fm5, ievt, "FullMatch", truncation);
    //std::cout << "FM: D3D4 seeding" << std::endl;
    //err_count += compareMemWithFile<FullMatchMemory<BARREL> >(fullmatch[5], fout_fm6, ievt, "FullMatch", truncation);
    //std::cout << "FM: L1D1 seeding" << std::endl;
    //err_count += compareMemWithFile<FullMatchMemory<BARREL> >(fullmatch[6], fout_fm7, ievt, "FullMatch", truncation);
    //std::cout << "FM: L2D1 seeding" << std::endl;
    //err_count += compareMemWithFile<FullMatchMemory<BARREL> >(fullmatch[7], fout_fm8, ievt, "FullMatch", truncation);

  }  // end of event loop
  
  // close files
  fin_as.close();
  fin_ap.close();
  fin_cm1.close();
  fin_cm2.close();
  fin_cm3.close();
  fin_cm4.close();
  fin_cm5.close();
  fin_cm6.close();
  fin_cm7.close();
  fin_cm8.close();
  fout_fm1.close();
  fout_fm2.close();
  fout_fm3.close();
  fout_fm4.close();
  fout_fm5.close();
  fout_fm6.close();
  fout_fm7.close();
  fout_fm7.close();

  // This is necessary because HLS seems to only return an 8-bit error count, so if err%256==0, the test bench can falsely pass
  if (err_count > 255) err_count = 255;
  return err_count;
}
