/*
 * 7segment.cpp
 *
 *  Created on: Apr 13, 2020
 *      Author: Marvin Sielenkemper
 */

#include "7segment.hpp"

#include "main.h"

constexpr uint8_t SEG_NONE_MASK { 0xf0 };

void
writeSegment(SegmentMask mask, uint8_t value) {
  GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK | value;
  GPIOA->ODR = (GPIOA->ODR & ~0xff) | mask | value;
  GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK | value;
  GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK;
}
