// Test bench for TrackletCalculator
#include "TrackletCalculatorTop.h"

#include <algorithm>
#include <iterator>
#include <cstring>

#include "FileReadUtility.h"
#include "Constants.h"

const int nevents = 50;  //number of events to run

using namespace std;

int main()
{
  // error counts
  int err = 0;

  ///////////////////////////
  // input memories
  static AllStubMemory<BARRELPS> innerStubs[1];
  static AllStubMemory<BARRELPS> outerStubs[2];
  static StubPairMemory stubPairs[13];

  // output memories
  static TrackletParameterMemory tpar;
  static TrackletProjectionMemory<BARRELPS> tproj_barrel_ps[TC::N_PROJOUT_BARRELPS];
  static TrackletProjectionMemory<BARREL2S> tproj_barrel_2s[TC::N_PROJOUT_BARREL2S];
  static TrackletProjectionMemory<DISK> tproj_disk[TC::N_PROJOUT_DISK];

  ///////////////////////////
  // open input files
  cout << "Open files..." << endl;

  const string dir = "TC_L1L2G";

  ifstream fin_innerStubs;
  if (not openDataFile(fin_innerStubs, dir + "/AllStubs_AS_L1PHIEn2_04.dat")) return -1;

  ifstream fin_outerStubs0;
  if (not openDataFile(fin_outerStubs0, dir + "/AllStubs_AS_L2PHIBn6_04.dat")) return -1;

  ifstream fin_outerStubs1;
  if (not openDataFile(fin_outerStubs1, dir + "/AllStubs_AS_L2PHICn3_04.dat")) return -1;

  ifstream fin_stubPairs0;
  if (not openDataFile(fin_stubPairs0, dir + "/StubPairs_SP_L1PHIE17_L2PHIB15_04.dat")) return -1;

  ifstream fin_stubPairs1;
  if (not openDataFile(fin_stubPairs1, dir + "/StubPairs_SP_L1PHIE17_L2PHIB16_04.dat")) return -1;

  ifstream fin_stubPairs2;
  if (not openDataFile(fin_stubPairs2, dir + "/StubPairs_SP_L1PHIE17_L2PHIC17_04.dat")) return -1;

  ifstream fin_stubPairs3;
  if (not openDataFile(fin_stubPairs3, dir + "/StubPairs_SP_L1PHIE17_L2PHIC18_04.dat")) return -1;

  ifstream fin_stubPairs4;
  if (not openDataFile(fin_stubPairs4, dir + "/StubPairs_SP_L1PHIE17_L2PHIC19_04.dat")) return -1;

  ifstream fin_stubPairs5;
  if (not openDataFile(fin_stubPairs5, dir + "/StubPairs_SP_L1PHIE18_L2PHIB16_04.dat")) return -1;

  ifstream fin_stubPairs6;
  if (not openDataFile(fin_stubPairs6, dir + "/StubPairs_SP_L1PHIE18_L2PHIC17_04.dat")) return -1;

  ifstream fin_stubPairs7;
  if (not openDataFile(fin_stubPairs7, dir + "/StubPairs_SP_L1PHIE18_L2PHIC18_04.dat")) return -1;

  ifstream fin_stubPairs8;
  if (not openDataFile(fin_stubPairs8, dir + "/StubPairs_SP_L1PHIE18_L2PHIC19_04.dat")) return -1;

  ifstream fin_stubPairs9;
  if (not openDataFile(fin_stubPairs9, dir + "/StubPairs_SP_L1PHIE18_L2PHIC20_04.dat")) return -1;

  ifstream fin_stubPairs10;
  if (not openDataFile(fin_stubPairs10, dir + "/StubPairs_SP_L1PHIE19_L2PHIC17_04.dat")) return -1;

  ifstream fin_stubPairs11;
  if (not openDataFile(fin_stubPairs11, dir + "/StubPairs_SP_L1PHIE19_L2PHIC18_04.dat")) return -1;

  ifstream fin_stubPairs12;
  if (not openDataFile(fin_stubPairs12, dir + "/StubPairs_SP_L1PHIE19_L2PHIC19_04.dat")) return -1;

  ///////////////////////////
  // open output files
  ifstream fout_tpar;
  if (not openDataFile(fout_tpar, dir + "/TrackletParameters_TPAR_L1L2G_04.dat")) return -1;

  ifstream fout_tproj0;
  if (not openDataFile(fout_tproj0, dir + "/TrackletProjections_TPROJ_L1L2G_L3PHIB_04.dat")) return -1;

  ifstream fout_tproj1;
  if (not openDataFile(fout_tproj1, dir + "/TrackletProjections_TPROJ_L1L2G_L3PHIC_04.dat")) return -1;

  ifstream fout_tproj2;
  if (not openDataFile(fout_tproj2, dir + "/TrackletProjections_TPROJ_L1L2G_L4PHIB_04.dat")) return -1;

  ifstream fout_tproj3;
  if (not openDataFile(fout_tproj3, dir + "/TrackletProjections_TPROJ_L1L2G_L4PHIC_04.dat")) return -1;

  ifstream fout_tproj4;
  if (not openDataFile(fout_tproj4, dir + "/TrackletProjections_TPROJ_L1L2G_L5PHIB_04.dat")) return -1;

  ifstream fout_tproj5;
  if (not openDataFile(fout_tproj5, dir + "/TrackletProjections_TPROJ_L1L2G_L5PHIC_04.dat")) return -1;

  ifstream fout_tproj6;
  if (not openDataFile(fout_tproj6, dir + "/TrackletProjections_TPROJ_L1L2G_L5PHID_04.dat")) return -1;

  ifstream fout_tproj7;
  if (not openDataFile(fout_tproj7, dir + "/TrackletProjections_TPROJ_L1L2G_L6PHIB_04.dat")) return -1;

  ifstream fout_tproj8;
  if (not openDataFile(fout_tproj8, dir + "/TrackletProjections_TPROJ_L1L2G_L6PHIC_04.dat")) return -1;

  ifstream fout_tproj9;
  if (not openDataFile(fout_tproj9, dir + "/TrackletProjections_TPROJ_L1L2G_L6PHID_04.dat")) return -1;

  ifstream fout_tproj10;
  if (not openDataFile(fout_tproj10, dir + "/TrackletProjections_TPROJ_L1L2G_D1PHIA_04.dat")) return -1;

  ifstream fout_tproj11;
  if (not openDataFile(fout_tproj11, dir + "/TrackletProjections_TPROJ_L1L2G_D1PHIB_04.dat")) return -1;

  ifstream fout_tproj12;
  if (not openDataFile(fout_tproj12, dir + "/TrackletProjections_TPROJ_L1L2G_D1PHIC_04.dat")) return -1;

  ifstream fout_tproj13;
  if (not openDataFile(fout_tproj13, dir + "/TrackletProjections_TPROJ_L1L2G_D1PHID_04.dat")) return -1;

  ifstream fout_tproj14;
  if (not openDataFile(fout_tproj14, dir + "/TrackletProjections_TPROJ_L1L2G_D2PHIA_04.dat")) return -1;

  ifstream fout_tproj15;
  if (not openDataFile(fout_tproj15, dir + "/TrackletProjections_TPROJ_L1L2G_D2PHIB_04.dat")) return -1;

  ifstream fout_tproj16;
  if (not openDataFile(fout_tproj16, dir + "/TrackletProjections_TPROJ_L1L2G_D2PHIC_04.dat")) return -1;

  ifstream fout_tproj17;
  if (not openDataFile(fout_tproj17, dir + "/TrackletProjections_TPROJ_L1L2G_D2PHID_04.dat")) return -1;

  ifstream fout_tproj18;
  if (not openDataFile(fout_tproj18, dir + "/TrackletProjections_TPROJ_L1L2G_D3PHIA_04.dat")) return -1;

  ifstream fout_tproj19;
  if (not openDataFile(fout_tproj19, dir + "/TrackletProjections_TPROJ_L1L2G_D3PHIB_04.dat")) return -1;

  ifstream fout_tproj20;
  if (not openDataFile(fout_tproj20, dir + "/TrackletProjections_TPROJ_L1L2G_D3PHIC_04.dat")) return -1;

  ifstream fout_tproj21;
  if (not openDataFile(fout_tproj21, dir + "/TrackletProjections_TPROJ_L1L2G_D3PHID_04.dat")) return -1;

  ifstream fout_tproj22;
  if (not openDataFile(fout_tproj22, dir + "/TrackletProjections_TPROJ_L1L2G_D4PHIA_04.dat")) return -1;

  ifstream fout_tproj23;
  if (not openDataFile(fout_tproj23, dir + "/TrackletProjections_TPROJ_L1L2G_D4PHIB_04.dat")) return -1;

  ifstream fout_tproj24;
  if (not openDataFile(fout_tproj24, dir + "/TrackletProjections_TPROJ_L1L2G_D4PHIC_04.dat")) return -1;

  ifstream fout_tproj25;
  if (not openDataFile(fout_tproj25, dir + "/TrackletProjections_TPROJ_L1L2G_D4PHID_04.dat")) return -1;


  ///////////////////////////
  // loop over events
  cout << "Start event loop ..." << endl;
  for (unsigned int ievt = 0; ievt < nevents; ++ievt) {
    cout << "Event: " << dec << ievt << endl;

    // Clear all output memories before starting.
    tpar.clear();
    for (unsigned i = 0; i < TC::N_PROJOUT_BARRELPS; i++)
      tproj_barrel_ps[i].clear();
    for (unsigned i = 0; i < TC::N_PROJOUT_BARREL2S; i++)
      tproj_barrel_2s[i].clear();
    for (unsigned i = 0; i < TC::N_PROJOUT_DISK; i++)
      tproj_disk[i].clear();    

    // read event and write to memories
    writeMemFromFile<AllStubMemory<BARRELPS> >(innerStubs[0], fin_innerStubs, ievt);
    writeMemFromFile<AllStubMemory<BARRELPS> >(outerStubs[0], fin_outerStubs0, ievt);
    writeMemFromFile<AllStubMemory<BARRELPS> >(outerStubs[1], fin_outerStubs1, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[0], fin_stubPairs0, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[1], fin_stubPairs1, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[2], fin_stubPairs2, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[3], fin_stubPairs3, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[4], fin_stubPairs4, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[5], fin_stubPairs5, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[6], fin_stubPairs6, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[7], fin_stubPairs7, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[8], fin_stubPairs8, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[9], fin_stubPairs9, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[10], fin_stubPairs10, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[11], fin_stubPairs11, ievt);
    writeMemFromFile<StubPairMemory>(stubPairs[12], fin_stubPairs12, ievt);

    // bx
    BXType bx = ievt;
    BXType bx_o;

    // Unit Under Test
    TrackletCalculator_L1L2G(bx, innerStubs, outerStubs, stubPairs,
       bx_o,
       &tpar,
       tproj_barrel_ps,
       tproj_barrel_2s,
       tproj_disk
    );

    bool truncation = false;

    // compare the computed outputs with the expected ones
    err += compareMemWithFile<TrackletParameterMemory>(tpar, fout_tpar, ievt,
                                                   "\nTrackletParameter", truncation);
    err += compareMemWithFile<TrackletProjectionMemory<BARRELPS> >(tproj_barrel_ps[TC::L3PHIB], fout_tproj0, ievt,
                                                   "\nTrackletProjection (L3PHIB)", truncation);
    err += compareMemWithFile<TrackletProjectionMemory<BARRELPS> >(tproj_barrel_ps[TC::L3PHIC], fout_tproj1, ievt,
                                                   "\nTrackletProjection (L3PHIC)", truncation);
    err += compareMemWithFile<TrackletProjectionMemory<BARREL2S> >(tproj_barrel_2s[TC::L4PHIB], fout_tproj2, ievt,
                                                   "\nTrackletProjection (L4PHIB)", truncation);
    err += compareMemWithFile<TrackletProjectionMemory<BARREL2S> >(tproj_barrel_2s[TC::L4PHIC], fout_tproj3, ievt,
                                                   "\nTrackletProjection (L4PHIC)", truncation);
    err += compareMemWithFile<TrackletProjectionMemory<BARREL2S> >(tproj_barrel_2s[TC::L5PHIB], fout_tproj4, ievt,
                                                   "\nTrackletProjection (L5PHIB)", truncation);
    err += compareMemWithFile<TrackletProjectionMemory<BARREL2S> >(tproj_barrel_2s[TC::L5PHIC], fout_tproj5, ievt,
                                                   "\nTrackletProjection (L5PHIC)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<BARREL2S> >(tproj_barrel_2s[TC::L5PHID], fout_tproj6, ievt,
                                                   "\nTrackletProjection (L5PHID)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<BARREL2S> >(tproj_barrel_2s[TC::L6PHIB], fout_tproj7, ievt,
                                                   "\nTrackletProjection (L6PHIB)", truncation);
    err += compareMemWithFile<TrackletProjectionMemory<BARREL2S> >(tproj_barrel_2s[TC::L6PHIC], fout_tproj8, ievt,
                                                   "\nTrackletProjection (L6PHIC)", truncation);
    err += compareMemWithFile<TrackletProjectionMemory<BARREL2S> >(tproj_barrel_2s[TC::L6PHID], fout_tproj9, ievt,
                                                   "\nTrackletProjection (L6PHID)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D1PHIA], fout_tproj10, ievt,
                                                   "\nTrackletProjection (D1PHIA)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D1PHIB], fout_tproj11, ievt,
                                                   "\nTrackletProjection (D1PHIB)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D1PHIC], fout_tproj12, ievt,
                                                   "\nTrackletProjection (D1PHIC)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D1PHID], fout_tproj13, ievt,
                                                   "\nTrackletProjection (D1PHID)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D2PHIA], fout_tproj14, ievt,
                                                   "\nTrackletProjection (D2PHIA)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D2PHIB], fout_tproj15, ievt,
                                                   "\nTrackletProjection (D2PHIB)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D2PHIC], fout_tproj16, ievt,
                                                   "\nTrackletProjection (D2PHIC)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D2PHID], fout_tproj17, ievt,
                                                   "\nTrackletProjection (D2PHID)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D3PHIA], fout_tproj18, ievt,
                                                   "\nTrackletProjection (D3PHIA)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D3PHIB], fout_tproj19, ievt,
                                                   "\nTrackletProjection (D3PHIB)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D3PHIC], fout_tproj20, ievt,
                                                   "\nTrackletProjection (D3PHIC)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D3PHID], fout_tproj21, ievt,
                                                   "\nTrackletProjection (D3PHID)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D4PHIA], fout_tproj22, ievt,
                                                   "\nTrackletProjection (D4PHIA)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D4PHIB], fout_tproj23, ievt,
                                                   "\nTrackletProjection (D4PHIB)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D4PHIC], fout_tproj24, ievt,
                                                   "\nTrackletProjection (D4PHIC)", truncate);
    err += compareMemWithFile<TrackletProjectionMemory<DISK> >(tproj_disk[TC::D4PHID], fout_tproj25, ievt,
                                                   "\nTrackletProjection (D4PHID)", truncate);
    cout << endl;

  } // end of event loop

  // This is necessary because HLS seems to only return an 8-bit error count, so if err%256==0, the test bench can falsely pass
  if (err > 255) err = 255;
  return err;

}
