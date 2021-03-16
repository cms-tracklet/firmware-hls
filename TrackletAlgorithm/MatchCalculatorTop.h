#ifndef TrackletAlgorithm_MatchCalculatorTop_h
#define TrackletAlgorithm_MatchCalculatorTop_h

#include "MatchCalculator.h"

constexpr int maxMatchCopies(8);
constexpr int maxFullMatchCopies(8);

void MatchCalculatorTop(BXType bx,
                        const CandidateMatchMemory match[maxMatchCopies],
                        const AllStubMemory<BARRELPS>*,
                        const AllProjectionMemory<BARRELPS>*,
                        BXType&,
                        FullMatchMemory<BARREL> fullmatch[maxFullMatchCopies]
                       );

#endif
