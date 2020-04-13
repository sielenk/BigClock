/*
 * 7segment.hpp
 *
 *  Created on: Apr 13, 2020
 *      Author: Marvin Sielenkemper
 */

#ifndef _7SEGMENT_HPP_
#define _7SEGMENT_HPP_

#include <cstdint>

enum SegmentMask : uint8_t {
  SEG_M01_MASK = 0x70,
  SEG_M10_MASK = 0xb0,
  SEG_H01_MASK = 0xd0,
  SEG_H10_MASK = 0xe0
};

void
writeSegment(SegmentMask mask, uint8_t value);

#endif /* _7SEGMENT_HPP_ */
