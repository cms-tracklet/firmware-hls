#ifndef TrackletAlgorithm_TrackletCalculator_parameters_h
#define TrackletAlgorithm_TrackletCalculator_parameters_h

// This file contains numbers of memories and bit masks that are specific to
// each TrackletCalculator and that come directly from the wiring.
//
// The specific inner all-stub memory that the indices in a given stub-pair
// memory correspond to is determined by ASInnerMask. The bits of this mask,
// from least significant to most significant, represent the stub-pair memories
// in the order they are passed to TrackletCalculator; e.g., the LSB
// corresponds to stubPairs[0]. The bit for a given stub-pair memory is the
// index (0 or 1) of the inner all-stub memory that should be used. Likewise,
// the specific outer all-stub memories are determined by ASOuterMask.
//
// The validity of each of the barrel TPROJ memories is determined by
// TPROJMaskBarrel. The bits of this mask, from least significant to most
// significant, represent the memories in the order they are passed to
// TrackletCalculator; e.g., the LSB corresponds to
// projout_barrel_ps[TC::L1PHIA]. If a bit is set, the corresponding memory is
// valid, if it is not, the corresponding memory is not valid. Likewise, the
// validity of each of the disk TPROJ memories is determined by TPROJMaskDisk
// in the same way.

// magic numbers for TC_L1L2D
template<> constexpr uint8_t NASMemInner<TF::L1L2, TC::D>() {
  return 2;
}
template<> constexpr uint8_t NASMemOuter<TF::L1L2, TC::D>() {
  return 1;
}
//template<> constexpr uint8_t NSPMem<TC::L1L2, TC::E>() {
//  return 13;
//}
template<> constexpr uint16_t ASInnerMask<TF::L1L2, TC::D>() {
  return 0x1FE0;
}
template<> constexpr uint16_t ASOuterMask<TF::L1L2, TC::D>() {
  return 0x0000;
}
template<> constexpr uint32_t TPROJMaskBarrel<TF::L1L2, TC::D>() {
  return 0x7733000;
}
template<> constexpr uint32_t TPROJMaskDisk<TF::L1L2, TC::D>() {
  return 0xFFFF;
}

#endif
