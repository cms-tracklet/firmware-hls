// Utilities used only in test bench for C simulation
#ifndef __SYNTHESIS__

#ifndef TestBenches_FileReadUtility_h
#define TestBenches_FileReadUtility_h

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <vector>
#include <bitset>

#include "../TrackletAlgorithm/Constants.h"

bool openDataFile(std::ifstream& file_in, const std::string& file_name)
{
  file_in.open(file_name);

  bool success = file_in.good();
  if (not success) {
    std::cerr << "Open of file " << file_name << " failed with error: ";
    std::cerr << std::strerror(errno);
    std::cerr << std::endl;
    std::cerr << "running from directory " << getcwd(nullptr,0) << std::endl;
  }

  return success;
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream sstream(s);
  
  while (getline(sstream, token, delimiter)) {
    if (token.empty()) continue;
    tokens.push_back(token);
  }
  
  return tokens;
}


// S.S. Storey 
// added because the IR 
// needs to fill a stream and not a memory 
// so writeMemFromFile does not work 
template<class DataType, int Base=2>
void writeArrayFromFile(DataType* hData, std::ifstream& pInputStream, int pEvent
, char pDelimeter = '|' , char pSplitToken = ' '){
  
  // check file is still good 
  assert(pInputStream.good());
  
  int cEventCounter=-1;
  int cCounter=0;
  do
  {
    std::string cInputLine="";
    getline( pInputStream, cInputLine );
    if( cInputLine.find("Event") != std::string::npos ) 
    {
      //std::cout << cInputLine << "\n";
      cEventCounter++;
    }
    else
    {
      if(cEventCounter != pEvent)
        continue;
      
      // split line 
      //std::cout << cInputLine << "\n";
      std::stringstream cLineContent(cInputLine);
      for(std::string cToken; getline( cLineContent, cToken , pSplitToken ); )
      {
        // look for binary representation of word  
        if( cToken.find('|') != std::string::npos )  
        {
          //remove delimeter
          cToken.erase( std::remove(cToken.begin(), cToken.end(), pDelimeter), cToken.end() );
          hData[cCounter] = DataType(std::stol( cToken , nullptr,Base ) );
          cCounter++; 
        }
      }
    }
  }while( pInputStream.good() && cEventCounter <= pEvent);
}

template<class MemType>
void writeMemFromFile(MemType& memory, std::ifstream& fin, int ievt, int base=16)
{
  std::string line;

  if (ievt==0) {
    getline(fin, line);
  }
  
  memory.clear();
  
  while (getline(fin, line)) {
    
    if (!fin.good()) {
      return;
    }
    
    if (line.find("Event") != std::string::npos) {
      return;
    } else {
      if (split(line,' ').size()==4) {
       memory.write_mem(ievt, line, base);
      } else {
	const std::string datastr = split(line, ' ').back();
	memory.write_mem(ievt, datastr, base);
      }
    }	
  }
  
}

template<class MemType, int InputBase=16, int OutputBase=16>
unsigned int compareMemWithFile(const MemType& memory, std::ifstream& fout,
                                int ievt, const std::string& label,
                                const bool truncated = false, int maxProc = kMaxProc)
{
  unsigned int err_count = 0;

  ////////////////////////////////////////
  // Read from file
  MemType memory_ref;
  writeMemFromFile<MemType>(memory_ref, fout, ievt, InputBase);

  for (int i = 0; i < memory_ref.getDepth(); ++i) {
    auto data_ref = memory_ref.read_mem(ievt,i).raw();
    auto data_com = memory.read_mem(ievt,i).raw();
    if (i==0) {
      // If both reference and computed memories are completely empty, skip it
      if (data_com == 0 && data_ref == 0) break;
      std::cout << label << ":" << std::endl;
      std::cout << "index" << "\t" << "reference" << "\t" << "computed" << std::endl;
    }
    // If have reached the end of valid entries in both computed and reference, don't bother printing further
    if (data_com == 0 && data_ref == 0) continue;

    std::cout << i << "\t";
    if (OutputBase == 2) std::cout << std::bitset<MemType::getWidth()>(data_ref) << "\t";
    else                 std::cout << std::hex << data_ref << "\t";
    
    if (OutputBase == 2) std::cout << std::bitset<MemType::getWidth()>(data_com);
    else                 std::cout << std::hex << data_com; // << std::endl;

    // If there is extra entries in reference
    if (data_com == 0) {
      std::cout << "\t" << "<=== missing";
      if (!truncated) err_count++;
    // If there is extra entries in computed
    } else if (data_ref == 0) {
      std::cout << "\t" << "<=== EXTRA";
      err_count++;
    // If reference and computed entry are inconsistent
    } else if (data_com != data_ref) {
      std::cout << "\t" << "<=== INCONSISTENT";
      err_count++;
    }

    std::cout << std::endl;
  }
  
  return err_count;
  
}

template<class MemType, int InputBase=16, int OutputBase=16>
unsigned int compareBinnedMemWithFile(const MemType& memory, 
                                      std::ifstream& fout,
                                      int ievt, const std::string& label,
                                      const bool truncated = false, int maxProc = kMaxProc)
{
  unsigned int err_count = 0;

  ////////////////////////////////////////
  // Read from file
  MemType memory_ref;
  writeMemFromFile<MemType>(memory_ref, fout, ievt, InputBase);

  ////////////////////////////////////////
  // compare expected data with those computed and stored in the output memory
  std::cout << label << ":" << std::endl;
  std::cout << "index" << "\t" << "reference" << "\t" << "computed" << std::endl;
  for ( int j = 0; j < memory_ref.getNBins(); ++j ) {
    std::cout << "Bin " << std::dec << j << std::endl;
    for (int i = 0; i < memory_ref.getNEntryPerBin() ; ++i) {
      auto data_ref = memory_ref.read_mem(ievt,j,i).raw();
      auto data_com = memory.read_mem(ievt,j,i).raw();

      // If have reached the end of valid entries in both computed and reference, don't bother printing further
      if (data_com == 0 && data_ref == 0) continue;

      std::cout << i << "\t";

      if (OutputBase == 2) std::cout << std::bitset<MemType::getWidth()>(data_ref) << "\t";
      else                 std::cout << std::hex << data_ref << "\t";
    
      if (OutputBase ==2) std::cout << std::bitset<MemType::getWidth()>(data_com);
      else                std::cout << std::hex << data_com; // << std::endl;

      // If there is extra entries in reference
      if (data_com == 0) {
        std::cout << "\t" << "<=== missing";
        if (!truncated) err_count++;
      // If there is extra entries in computed
      } else if (data_ref == 0) {
        std::cout << "\t" << "<=== EXTRA";
        err_count++;
      // If reference and computed entry are inconsistent
      } else if (data_com != data_ref) {
        std::cout << "\t" << "<=== INCONSISTENT";
        err_count++;
      }

      std::cout << std::endl;
    } // loop over entries in bin
  } // loop over bins

  return err_count;
  
}

#endif // TestBenches_FileReadUtility_h

#endif // __SYNTHESIS__
