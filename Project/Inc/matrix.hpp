/*
 * matrix.hpp
 *
 *  Created on: Apr 5, 2020
 *      Author: Marvin H. Sielenkemper
 */

#ifndef MATRIX_HPP_
#define MATRIX_HPP_
#ifdef __cplusplus

#include <ctime>

void
matrixSetTime(std::tm const& current_time);

extern "C" {
#endif

  void
  matrixTaskFun(void *argument) __attribute__ ((noreturn));

#ifdef __cplusplus
}
#endif
#endif /* MATRIX_HPP_ */
