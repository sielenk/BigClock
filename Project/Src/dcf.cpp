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

bool
dcfCheck(DCF dcf, int offset) {
  DCFCheck const &check { reinterpret_cast<DCFCheck&>(dcf) };

  return (offset == 59 || (dcf.leapSecComing && offset == 60))
      && (check.zero == 0) && (check.one == 1) && (check.leapSecZero == 0)
      && !parity(check.minute) && !parity(check.hour) && !parity(check.date);
}
