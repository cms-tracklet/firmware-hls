// InputRouter Test
//#include "InputStubs.h"
#include "FileReadUtility.h"
#include "InputRouterTop.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <map>



static const int kMaxNEvents = 100;  // max number of events to run

// tables now deduces from wire map
// will remove when I've got some link data to test with
// link assignment table 
// TO-BE fixed :  need this added to emData 
// 3 bits for layer/disk id  --> 3 bits 
// 1 bit for barrel/disk --> 4 bits  
// up-to 4 layers/disks per DTC
// 16 bits per link
// then 3 bits 
// 1 bit to assign whether link is PS/2S 
// 3 bits to encode the number of layers readout by this DTC 
// 20 bits in total 
// const ap_uint<kLINKMAPwidth> kLinkAssignmentTable[] =
// {
//   0x500b9, 0x3000b, 0x3000d, 0x5006d, 
//   0x50082, 0x500a4, 0x60843, 0x8a623, 
//   0x20005, 0x60a62, 0x40047, 0x40087
// };

// number of phi bins read out 
// by each link (kSizeBinWord per layer )
// upto 4 layers per link  
// const ap_uint<kBINMAPwidth> kLinkNPhiBns[] = 
// {
//   0x01B , 0x003 , 0x003 , 0x01B , 
//   0x01B , 0x01B , 0x0DF , 0x6DF , 
//   0x003 , 0x0DB , 0x01B , 0x01B 
// };

// total number of memoreis 
// readout b each link 
// const ap_uint<kNMEMwidth> kLinkNMemories[] = 
// {
//    8 ,  4 ,  4 ,  8 , 
//    8 ,  8 , 16 , 20 , 
//    4 , 12 ,  8 ,  8 
// };



// LUT with phi corrections to the nominal radius. Only used by layers.
// Values are determined by the radius and the bend of the stub.
const int kPhiCorrtable_L1[] =
#include "../emData/LUTs/VMPhiCorrL1.tab"
;
const int kPhiCorrtable_L2[] =
#include "../emData/LUTs/VMPhiCorrL2.tab"
;
const int kPhiCorrtable_L3[] =
#include "../emData/LUTs/VMPhiCorrL3.tab"
;
const int kPhiCorrtable_L4[] =
#include "../emData/LUTs/VMPhiCorrL4.tab"
;
const int kPhiCorrtable_L5[] =
#include "../emData/LUTs/VMPhiCorrL5.tab"
;
const int kPhiCorrtable_L6[] =
#include "../emData/LUTs/VMPhiCorrL6.tab"
;

// const int kIRNmemsLUT[] =
// #include "../emData/LUTs/IR_Nmems.tab"
// ;


// map of detector regions read out [ per DTC ]
using CablingMap = std::map<int, std::vector<uint8_t>> ;
// map of dtc names and link ids 
using DtcMap = std::map< std::string, std::vector<uint8_t>> ;
// map of input links  [per DTC ]
using LinkMap = std::map<int, std::pair<std::string ,std::vector<std::uint8_t>>> ; 
//map of input stubs [ per Bx ]
using InputStubs = std::map<int, std::vector<std::string>> ; 
//vector of stubs 
using Stubs = std::vector<std::bitset<kNBits_DTC>>; 
// DTC layer map 
typedef std::map<int, std::vector<int>> TkMap;


using namespace std;

// get cabling map 
// return link names from 
// wire.dat 
DtcMap getCablingMap( std::string pInputWiresMap )
{
  std::ifstream fin_wiring_map;
  if (not openDataFile(fin_wiring_map,pInputWiresMap)) 
  {
    std::cout << "Could not find file " 
      << pInputWiresMap << std::endl;
  }

  DtcMap cDtcMap;
  std::vector<std::string> cDtcNames;
  for(std::string cInputLine; getline( fin_wiring_map, cInputLine ); )
  {
    auto cStream = std::istringstream{cInputLine};
    std::string cToken;
    std::string cPrevToken = "";
    std::vector<std::string> cTokens;
    // read line 
    // and look for connections where 
    // VMR is the output 
    bool cFoundInputLink=false;
    
    std::string cInputLink = "";
    std::string cVMRMemory = "";
    while (cStream >> cToken) 
    {
      if( cPrevToken.find("input") != std::string::npos 
        && cToken.find("output") != std::string::npos ) 
      {
        cFoundInputLink = true;
      }

      if( cPrevToken.find("output") != std::string::npos 
        && cToken.find("VMR") != std::string::npos ) 
      {
        cVMRMemory=cToken;
      }
      if( cInputLink.empty() ) cInputLink = cToken;
      cPrevToken = cToken; 
    }

    // extract DTC name 
    // and detector region 
    if( !cInputLink.empty() && !cVMRMemory.empty() )
    {
      std::string cDetectorRegion="";
      std::string cDtcName="";

      //std::cout << cInputLink << "\t\t" << cVMRMemory << "\n";
      std::stringstream cLinkNameSS(cInputLink);
      char cSplitToken='_';
      std::string cPrevSubStr="";
      for(std::string cSubStr; getline( cLinkNameSS, cSubStr , cSplitToken ); )  
      {

        //std::cout << "\t..." << cSubStr << "\n";
        if( cPrevSubStr.find("IL") != std::string::npos  && cDetectorRegion.empty() )
          cDetectorRegion = cSubStr;

        if( cPrevSubStr.find(cDetectorRegion) != std::string::npos  && cDtcName.empty() && !cDetectorRegion.empty() )
          cDtcName = cSubStr;

        if ( cPrevSubStr.find(cDtcName) != std::string::npos && !cDtcName.empty() )
          cDtcName += "_" + cSubStr;
        
        cPrevSubStr = cSubStr;
      }
      // figure out detector layer 
      std::string cLyrName  = cDetectorRegion.substr(0, cDetectorRegion.find("PHI")) ;
      uint8_t cLyr = std::stol(  cLyrName.substr(1, cLyrName.length()-1) , nullptr, 10 );
      uint8_t cLayerId = (cLyrName.substr(0,1).find("D") != std::string::npos)  ? (10 + cLyr): cLyr; 
      auto cDtcIter = std::find(cDtcNames.begin(), cDtcNames.end(), cDtcName ) ;
      // detector region printout   
      if( cDtcIter == cDtcNames.end() || cDtcNames.empty())
      {
        // push back dtc name
        cDtcNames.push_back( cDtcName );
        std::vector<uint8_t> cLyrs(0); cLyrs.push_back(cLayerId);
        cDtcMap.insert( std::make_pair( cDtcName , cLyrs) ); 
      }
      else
      {
          // map by name 
          auto cNameIterator =  cDtcMap.find( cDtcName );
          if( cNameIterator != cDtcMap.end() )
          {
            // check if layer is already there 
            auto cLyrIter = std::find(cNameIterator->second.begin(), cNameIterator->second.end(), cLayerId ); 
            if( cLyrIter == cNameIterator->second.end() )
            {
              cNameIterator->second.push_back( cLayerId );
            }
          }
      }
    }
  }

  // for(auto cMapItem : cDtcMap)
  // {
  //   auto cLyrs = cMapItem.second;
  //   std::cout << "DTC " << cMapItem.first << "\n";
  //   for(auto cLyr : cLyrs ) 
  //     std::cout << "\t...Layer : " << +cLyr << "\n";
  // }

  // return map
  return cDtcMap;
}

// return name of dtc based on link id 
std::string getDTCName( int pLinkId
  , std::string pInputFile_Wires = "emData/wires_hourglass.dat")
{
  auto cDtcMap = getCablingMap( pInputFile_Wires );
  auto cMapIter = *std::next(cDtcMap.begin(), pLinkId);
  return cMapIter.first;
}

// return layers based on link id 
std::vector<uint8_t> getLyrs( int pLinkId
  , std::string pInputFile_Wires = "emData/wires_hourglass.dat")
{
  auto cDtcMap = getCablingMap( pInputFile_Wires );
  auto cMapIter = *std::next(cDtcMap.begin(), pLinkId);
  return cMapIter.second;
}


// return name of link file based on link id 
// link 
std::string getLinkName( int pLinkId
  , int pDTCsplit = 0
  , int pNonant= 4 
  , std::string pInputFile_Wires = "emData/wires_hourglass.dat")
{
  std::string cDtcName = getDTCName( pLinkId, pInputFile_Wires ); 
  std::string cLinkName = "Link_DL_" + cDtcName + ((pDTCsplit==0)?"_A":"_B");
  cLinkName += "_0" + std::to_string(pNonant); 
  return cLinkName;
          
}

std::string getMemPrintName( std::string pDtcName
  , int pDTCsplit 
  , int pNonant  
  , int pLyr 
  , int pPhiBn  ) 
{
  std::string cDtcName = pDtcName; 
  int cLyrCorr = (pLyr<11)? 0 : 10; 
  std::string cLyrId = (pLyr<11) ? "L" : "D";
  cLyrId += std::to_string(pLyr-cLyrCorr); 
    
  cDtcName += ((pDTCsplit==0)?"_A":"_B");
  std::stringstream ss;
  ss << "PHI" ; 
  ss << static_cast<char>(int('A') + pPhiBn);
  ss << "_" << cDtcName << "_";
  ss << std::setw(2) << std::setfill('0') << pNonant;
  std::string cMemPrint = "InputStubs_IL_";
  cMemPrint += cLyrId + ss.str() + ".dat"; 
  return cMemPrint;
}

using DtcMemWrd = std::pair<int, ap_uint<kBINMAPwidth>>; 
// return number of phi bins 
// connectd to link 
DtcMemWrd getNMemories( int pLinkId
  , int pDTCsplit = 0
  , int pNonant = 4 
  , std::string pInputFile_Wires = "emData/wires_hourglass.dat")
{

  DtcMemWrd cDtcMemWrd;
  std::string cDtcName = getDTCName( pLinkId, pInputFile_Wires ); 
  std::vector<uint8_t> cLyrs = getLyrs( pLinkId , pInputFile_Wires );
  //std::cout << "Found " << +cLyrs.size() << " layer(s) readout out by [DTC]" << cDtcName << "\n";
  std::vector<int> cNmems(cLyrs.size(),0);
  ap_uint<kBINMAPwidth> cLinkNPhiBns=0;
  int cOffst=0; 
  for( auto cLyr : cLyrs )
  {
    //std::cout << "Lyr:" << +cLyr << "\n";
    int cNmemories=0;
    for( size_t cPhiBn=0; cPhiBn<kMaxPhiBnsPrLyr; cPhiBn++)
    {
      // re-construct file name 
      std::string cFileName = "emData/MemPrints/InputStubs/"; 
      cFileName += getMemPrintName( cDtcName,  pDTCsplit, pNonant, cLyr,  (int)cPhiBn);
      std::ifstream fin_mem_prints;
      bool cPrintError=false;
      bool cFileFnd = openDataFile(fin_mem_prints,cFileName,cPrintError);
      if( cFileFnd ){ 
        //std::cout << "\t\t.. " << cFileName << "\n";
        fin_mem_prints.close();
        cNmemories++;
      }//if file exists 
    }//all possible phi bins 
    cLinkNPhiBns.range(cOffst+kSizeBinWord-1,cOffst) = cNmemories-1;
    cOffst += kSizeBinWord;
    auto cIndx= std::distance(cLyrs.begin(), std::find(cLyrs.begin(),cLyrs.end(),cLyr));
    cNmems[cIndx]=cNmemories;
  }
  cDtcMemWrd.first = std::accumulate(cNmems.begin(), cNmems.end(), 0);
  cDtcMemWrd.second = cLinkNPhiBns;
  return cDtcMemWrd;
}

// 3 bits for layer/disk id  --> 3 bits 
// 1 bit for barrel/disk --> 4 bits  
// up-to 4 layers/disks per DTC
// 16 bits per link
// then 3 bits 
// 1 bit to assign whether link is PS/2S 
// 3 bits to encode the number of layers readout by this DTC 
// 20 bits in total 
ap_uint<kLINKMAPwidth> getLnkWrd(int pLinkId
  , std::string pInputFile_Wires = "emData/wires_hourglass.dat")
{
  std::string cDtcName = getDTCName( pLinkId, pInputFile_Wires ); 
  std::vector<uint8_t> cLyrs = getLyrs( pLinkId , pInputFile_Wires );
  int cOffst=0; 
  ap_uint<kLINKMAPwidth> cLnkWrd;
  for( auto cLyr : cLyrs )
  {
    bool cIsBrl = (cLyr<11);
    int cLyrCorr = cIsBrl ? 0 : 10; 
    ap_uint<kSizeLinkWord> cWrd; 
    cWrd.range(0,0) =  (int)cIsBrl;
    cWrd.range(3,1) = cLyr-cLyrCorr;
    cLnkWrd.range(cOffst+kSizeLinkWord-1, cOffst) = (int)cWrd;
    cOffst+= kSizeLinkWord;
    //std::cout << "Lyr" << +cLyr << " isBrl " << (int)cIsBrl << " layer corr" << cLyrCorr << "\n";
  }
  bool cIs2S = ( cDtcName.find("2S") != std::string::npos  );
  cLnkWrd.range( kLINKMAPwidth-4,kLINKMAPwidth-4 ) = cIs2S ? 1 : 0 ;
  cLnkWrd.range( kLINKMAPwidth-1, kLINKMAPwidth-3) = cLyrs.size();
  return cLnkWrd;
}
void decodeLnkWrd(ap_uint<kLINKMAPwidth> pLnkWrd)
{
  int cNlyrs = pLnkWrd.range( kLINKMAPwidth-1, kLINKMAPwidth-1-(3-1));
  int cIs2S = pLnkWrd.range(kLINKMAPwidth-4,kLINKMAPwidth-4);
  std::cout << "LnkWrd 0x" 
    << std::hex  << int(pLnkWrd) << std::dec 
    << "\t..."
    << cNlyrs << " layers read-out...\t"
    << "Is2S bit is : " << cIs2S << "\n";
  int cOffst=0;
  for(size_t cLyrIndx=0; cLyrIndx < kMaxLyrsPerDTC; cLyrIndx++)
  {
    uint8_t cWrd=pLnkWrd.range(cOffst+kSizeLinkWord-1, cOffst);
    uint8_t cIsBrl=(cWrd&(0x1<<0))>>0;
    uint8_t cLyr=(cWrd&(0x7<<1))>>1;
    if( cLyrIndx < cNlyrs ) 
    {
      std::cout << "\t..LyrWrd#"
        << +cLyrIndx
        << " : 0x"
        << std::hex << +cWrd  << std::dec 
        << "\t\t..Lyr#" << +cLyr 
        << " , isBrl bit " << +cIsBrl << "\n";
    }
    cOffst+=kSizeLinkWord;
  }
}


// prepare file streams 
// containing data from 
// reference memories 
// produced by emulation 
void prepareInputStreams( ifstream * pInputStreams
  , int pLinkId
  , int pDTCsplit = 0 
  , int pNonant = 4 
  , std::string pInputFile_Wires = "emData/wires_hourglass.dat" )
{
  // dtc name
  std::string cDtcName = getDTCName( pLinkId, pInputFile_Wires );
  std::cout << "DTC " << cDtcName << " is link#" <<  pLinkId << std::endl;
  
  auto hLinkWord = getLnkWrd(pLinkId,pInputFile_Wires);
  auto cMemWrd = getNMemories( pLinkId, pDTCsplit, pNonant, pInputFile_Wires);
  ap_uint<kBINMAPwidth> hPhBnWord = cMemWrd.second; 
  int cMemIndx=0;
  std::vector<uint8_t> cLyrs = getLyrs( pLinkId , pInputFile_Wires );
  for(int cLyrIndx=0; cLyrIndx< cLyrs.size(); cLyrIndx++)
  {
    ap_uint<kSizeLinkWord> hWrd = hLinkWord.range(kSizeLinkWord*cLyrIndx+kSizeLinkWord-1,kSizeLinkWord*cLyrIndx);
    if( hWrd == 0) continue;
    ap_uint<1> hIsBrl = hWrd.range(0, 0);
    ap_uint<3> hLyrId = hWrd.range(3, 1);
    // get phi bin 
    ap_uint<kSizeBinWord> hBnWrd = hPhBnWord.range(kSizeBinWord * cLyrIndx + (kSizeBinWord-1), kSizeBinWord * cLyrIndx);
    auto cNPhiBns =  (1+(unsigned int)(hBnWrd));
    std::cout << "Layer# " << hLyrId 
      << "[Index=" << hLyrId << "]"
      << " " << cNPhiBns 
      << " phi bins"
      << " barrel bit is set to "
      << +hIsBrl
      << "\n";
    int cLyrCorr = (hIsBrl==1) ? 0 : 10; 
    for( int cPhiBn=0; cPhiBn<cNPhiBns; cPhiBn++)
    {
      std::string cFileName = "emData/MemPrints/InputStubs/"; 
      std::string cMemPrint = getMemPrintName( cDtcName, pDTCsplit, pNonant, hLyrId + cLyrCorr, cPhiBn ); 
      cFileName += cMemPrint;

      std::ifstream fin_mem_prints;
      bool cPrintError=false;
      bool cFileFnd = openDataFile(fin_mem_prints,cFileName,cPrintError);
      if( cFileFnd ){ 
        fin_mem_prints.close();
        std::cout << "\t..Loading stubs from .. " << cFileName 
          << " [Mem# " << +cMemIndx << "]\n";
        openDataFile(pInputStreams[cMemIndx],cFileName); 
        cMemIndx++;
      }
    }
  }
}

// for now .. create LUT for IR 
// this can eventually be produced 
// by either the wiring script 
// or the emulation 
void prepareLUTs( int pDTCsplit = 0
  , int pNonant = 4 
  , std::string pInputFile_Wires = "emData/wires_hourglass.dat" )
{
  auto cDtcMap = getCablingMap( pInputFile_Wires );
  auto cMapIter = cDtcMap.begin(); 
  uint8_t cLinkId=0;
  ofstream cOstream_LUT;
  cOstream_LUT.open ("emData/LUTs/IR_Nmems.tab");
  cOstream_LUT << "{\n";
  do
  {
    auto cDtcName = cMapIter->first;
    auto cLyrs = cMapIter->second; 
    auto cNmems = getNMemories( cLinkId, pDTCsplit, pNonant, pInputFile_Wires);
    std::cout << "For link# " << +cLinkId
      << " found  " << +cNmems.first 
      << " memories connected to this link\t"
      << "Word encoding number of bins readout per link is 0x"
      <<  std::hex << int(cNmems.second)  << std::dec
    <<  "\n";
    cOstream_LUT << std::to_string(+cNmems.first) << ",\n";
    cLinkId++;  
    cMapIter++;
  }while( cMapIter != cDtcMap.end() );
  cOstream_LUT << "};";
  cOstream_LUT.close();
}

// test bench starts here 
int main(int argc, char * argv[])
{
  // default values for test bench 
  int cLinkId = 6; 
  int cDTCsplit=0;
  int cNonant=4;
  // if cmd line args are passed 
  // parse them and change defaults 
  if( argc > 1 )
  {
    std::cout << "Have passed " << +argc << " arguments to the IR test bench.\n";
    for( size_t cArgIndx=1; cArgIndx < (size_t)argc ; cArgIndx++)
    {
      std::string cArg = argv[cArgIndx];
      if( cArg.find("link") != std::string::npos ) 
      {
        cLinkId  = std::stol( cArg.substr( cArg.find(",")+1 , cArg.length()-1) , nullptr, 10 );
        std::cout << "Link id passed from cmd line is " << cLinkId << "\n";
      }
      if( cArg.find("dtcSplit") != std::string::npos ) 
      {
        cDTCsplit  = std::stol( cArg.substr( cArg.find(",")+1 , cArg.length()-1) , nullptr, 10 );
        std::cout << "DTC split passed from cmd line is " << cDTCsplit << "\n";
      }
      if( cArg.find("tkNonant") != std::string::npos ) 
      {
        cNonant  = std::stol( cArg.substr( cArg.find(",")+1 , cArg.length()-1) , nullptr, 10 );
        std::cout << "Tk nonant passed from cmd line is " << cDTCsplit << "\n";
      }
    }
  }
  int cTotalErrCnt=0;

  // allow for truncation memory check [i.e. missing entries can pass check]
  bool cTruncation=false;
  std::string cInputFile_Wires = "emData/wires_hourglass.dat";
  // to be moved to emulation/wiring script 
  prepareLUTs(cDTCsplit,  cNonant, cInputFile_Wires ); 

  //
  int cFirstBx = 0 ;
  int cLastBx = 0;
  
  // dtc name
  std::string cDtcName = getDTCName( cLinkId, cInputFile_Wires );
  std::cout << "DTC " << cDtcName << " is link#" <<  cLinkId << std::endl;
  // link name
  std::string cLinkName = getLinkName( cLinkId, cDTCsplit ,cNonant, cInputFile_Wires); 
  std::string cBaseName  = "emData/MemPrints/InputStubs/";
  std::string cInputFile_Link = cBaseName + cLinkName + ".dat";
  std::cout << "Input file for Link# " << cLinkId << " is " <<  cInputFile_Link << std::endl;
  // link word
  // auto cLnkWrd = kLinkAssignmentTable[cLinkId%12];
  auto cLnkWrd = getLnkWrd(cLinkId,cInputFile_Wires);
  std::cout << "Word encoding number of layers readout per link is 0x"
    <<  std::hex << int(cLnkWrd)  << std::dec
    <<  "\n";
  // print decoded word to screen 
  decodeLnkWrd(cLnkWrd); 
  

  auto cMemWrd = getNMemories( cLinkId, cDTCsplit, cNonant, cInputFile_Wires);
  std::cout << "Found " << +cMemWrd.first 
    << " memories connected to this link\t"
    << "Word encoding number of bins readout per link is 0x"
    <<  std::hex << int(cMemWrd.second)  << std::dec
    <<  "\n";

  // int cNmemories = kLinkNMemories[cLinkId%12]
  const unsigned int cNmemories = cMemWrd.first ;
  // ap_uint<kBINMAPwidth> cPhBnWord = kLinkNPhiBns[cLinkId%12];
  ap_uint<kBINMAPwidth> cPhBnWord = cMemWrd.second; 

  
  // prepare memories that will be filled 
  DTCStubMemory hMemories[cNmemories];
  
  // prepare input streams 
  // those are used to read 
  // reference memory prints 
  // from emulation 
  ifstream cInputStreams[cNmemories];
  prepareInputStreams( cInputStreams
    , cLinkId
    , cDTCsplit
    , cNonant 
    , cInputFile_Wires );

  // prepare input stream 
  // used to read stubs arriving 
  // from the DTC on the link 
  // LINK_ file from 
  // emulation 
  std::ifstream cLinkDataStream;
  // check file exists 
  if( !openDataFile(cLinkDataStream,cInputFile_Link ) ) 
    return 1; 

  // now prepare inputs for IR 
  ap_uint<kNBitsNLnks> hLinkId = cLinkId;
  const ap_uint<kLINKMAPwidth> &hLinkWord = cLnkWrd;
  const ap_uint<kBINMAPwidth> &hPhBnWord = cPhBnWord;
  const ap_uint<kNMEMwidth> hNmemories = cNmemories;
  ap_uint<1> hIs2S = hLinkWord.range(kLINKMAPwidth-3,kLINKMAPwidth-4);
  
  
  for( int cEvId=0; cEvId < kMaxNEvents; cEvId++)
  {
    // fill reference output memories 
    // these will be used to compare the 
    // output of the emulation to 
    // the output of the HLS top level 
    // DTCStubMemory hRefMems[cNMemories]; 
    // for( size_t cMemIndx = 0; cMemIndx < cNMemories; cMemIndx++)
    // {
    //   writeMemFromFile<DTCStubMemory>(hRefMems[cMemIndx], cInputStreams[cMemIndx], cEvId);
    // }
    
    
    // only compare the ones I want 
    if( cEvId < cFirstBx || cEvId > cLastBx ) continue;
    
    std::cout << "Event#" << +cEvId << "\n";
    // prepare input stub stream 
    ap_uint<kNBits_DTC> hInputStubs[kMaxStubsFromLink];
    for( size_t cStubIndx=0; cStubIndx < kMaxStubsFromLink; cStubIndx++)
      hInputStubs[cStubIndx]=ap_uint<kNBits_DTC>(0);
    writeArrayFromFile<ap_uint<kNBits_DTC>>(hInputStubs , cLinkDataStream, cEvId);
    // // look at stubs 
    // for( unsigned int cIndx=0; cIndx < (unsigned int)kMaxStubsFromLink ; cIndx++)
    // { 
    //    if( hInputStubs[cIndx] == 0 ) continue;

    //    auto hVldBt = hInputStubs[cIndx].range( kMSBVldBt ,  kLSBVldBt);
    //    auto hEncLyr = hInputStubs[cIndx].range(kMSBLyrBts, kLSBLyrBts);
    //    std::cout << "Stub " << std::bitset<kNBits_DTC>(hInputStubs[cIndx])
    //     << " -- " << std::hex << hInputStubs[cIndx] << std::dec 
    //     << " valid bit " << (int)hVldBt 
    //     << " encoded layer " << (int)hEncLyr
    //     << "\n";
    // }

    // clear memories 
    for( unsigned int cIndx=0; cIndx < (unsigned int)hNmemories ; cIndx++)
    { 
       hMemories[cIndx].clear();
    }
    std::cout << "IR Module for link#" 
      << +hLinkId
      << " Link Word is " 
      << std::bitset<kLINKMAPwidth>(hLinkWord)
      << "\t"
      << std::hex
      << hLinkWord 
      << std::dec
      << "\t connected to "
      << hNmemories 
      << " memories."
      << "\n";

    // decode link wrd for this layer
    // figure out which of the LUTs I need 
    static const int* cLUT_L1 = (  hIs2S == 1 )  ? kPhiCorrtable_L4 : kPhiCorrtable_L1; 
    static const int* cLUT_L2 = (  hIs2S == 1 )  ? kPhiCorrtable_L5 : kPhiCorrtable_L2; 
    static const int* cLUT_L3 = (  hIs2S == 1 )  ? kPhiCorrtable_L6 : kPhiCorrtable_L3; 

    BXType hBx = cEvId&0x7;
    BXType hBx_o; 
    InputRouterTop( hBx
      , cNmemories // number of memories 
      , hLinkWord // input link LUT 
      , hPhBnWord  // n phi bins LUT 
      , cLUT_L1// corrections frst brl lyr  
      , cLUT_L2 // corrections scnd brl lyr  
      , cLUT_L3 // corrections thrd brl lyr  
      , hInputStubs // input stub stream 
      , hBx_o // output bx 
      , hMemories); 

    // // compare memories 
    for( size_t cMemIndx = 0; cMemIndx < (unsigned int)hNmemories; cMemIndx++)
    {
      // // for now I exclude the last phi bin 
      // // of L1 
      // // remove this when modified 
      // // emulation data is available 
      // if( cLinkId%12 == 6 || cLinkId%12 ==7 ) 
      //   if( cMemIndx >= 3 && cMemIndx < 8 ) // for now do not compare edges of L1 
      //     continue;

      // std::cout << "Memory#" 
      //   << cMemIndx 
      //   << " depth is "
      //   << hMemories[cMemIndx].getDepth()
      //   << "\n";
      // for( size_t cIndx=0; cIndx < hMemories[cMemIndx].getDepth(); cIndx++)
      // {
      //   auto cEntry = hMemories[cMemIndx].read_mem(hBx,cIndx).raw();
      //   if( cEntry== 0 ) continue;
      //   std::cout << "\t..#" << +cIndx 
      //     << " entry " << std::bitset<kBRAMwidth>(cEntry)
      //     << " --- " << std::hex << cEntry << std::dec 
      //     << " --- "
      //     << "\n";
      // }
      int cErCnt = compareMemWithFile<DTCStubMemory>(hMemories[cMemIndx], cInputStreams[cMemIndx], cEvId, "DTCStubMemory", cTruncation);
      cTotalErrCnt += cErCnt;
    }

    // reset input file stream back to the start 
    cLinkDataStream.clear();
    cLinkDataStream.seekg (0, ios::beg);
  }

  // place point back to start 
  for( size_t cMemIndx = 0; cMemIndx < (unsigned int)hNmemories; cMemIndx++)
  {
    cInputStreams[cMemIndx].close();
  }
  cLinkDataStream.close();

  // // remove temp LUT 
  // if( remove( "emData/LUTs/IR_Nmems.tab" ) == 0 )
  // {
  //   std::cout << "Temperorary LUT created by IR test bench deleted.\n";
  // }

  return cTotalErrCnt;
}
