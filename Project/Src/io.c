/*
 * io.cpp
 *
 *  Created on: 02.03.2020
 *      Author: Marv
 */

#include "usbd_cdc_if.h"

int
_write(int file, char *ptr, int len) {
  while (CDC_Transmit_FS((uint8_t*)ptr, len) == USBD_BUSY) {
  }
  return len;
}

int
__io_getchar(void) {
  return 0;
}
