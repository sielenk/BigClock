/**
 ******************************************************************************
 * File Name          : dcf.cpp
 * Description        : This file provides code for the DCF decoder.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 Marvin Sielenkemper.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

#include "dcf.hpp"

namespace {
  typedef struct {
    unsigned int zero :1;
    unsigned int :19;
    unsigned int one :1;
    unsigned int minute :8;
    unsigned int hour :7;
    unsigned int date :23;
    unsigned int leapSecZero :1;
  } __attribute__((packed)) DCFCheck;

  int
  parity(int n) {
    n ^= n >> 1;
    n ^= n >> 2;
    n ^= n >> 4;
    n ^= n >> 8;
    n ^= n >> 16;

    return n & 1;
  }
}

void
dcf_addBit(int secondStart, unsigned short pulse, unsigned short delta) {
  static union {
    unsigned long long bits;
    const DCFCheck check;
    const DCF dcf;
  } buffer = { bits: 0 };

  static signed char offset = -1;

  const bool pulseOk = (500 <= pulse && pulse <= 2500);
  const bool deltaOk1 = (9500 <= delta && delta <= 10500);
  const bool deltaOk2 = (19500 <= delta && delta <= 20500);
  const bool deltaOk = deltaOk1 | deltaOk2;
  const bool isLast = deltaOk2;

  if (pulseOk && deltaOk) {
    if (0 <= offset && offset <= 60) {
      if (pulse > 1500) {
        buffer.bits |= 1ull << offset;
      }
      ++offset;
    } else {
      offset = -1;
      buffer.bits = 0;
    }

    if (isLast) {
      if ((offset == 59 || (buffer.dcf.leapSecComing && offset == 60))
          && (buffer.check.zero == 0) && (buffer.check.one == 1)
          && (buffer.check.leapSecZero == 0) && !parity(buffer.check.minute)
          && !parity(buffer.check.hour) && !parity(buffer.check.date)) {
        dcf_handleTelegram(secondStart, &buffer.dcf);
      } else {
        dcf_handleTelegram(secondStart, nullptr);
      }

      offset = 0;
      buffer.bits = 0;
    }
  } else {
    offset = -1;
    buffer.bits = 0;
  }
}
