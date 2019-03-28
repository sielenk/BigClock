/**
 ******************************************************************************
 * File Name          : dcf.c
 * Description        : This file provides code for the DCF decoder.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 Marvin Sielenkemper.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

#include "dcf.h"

typedef struct {
	unsigned int zero :1;
	unsigned int :19;
	unsigned int one :1;
	unsigned int minute :8;
	unsigned int hour :7;
	unsigned int date :23;
	unsigned int oneOpt :1;
}__attribute__((packed)) DCFCheck;

static int parity(int n) {
	n ^= n >> 1;
	n ^= n >> 2;
	n ^= n >> 4;
	n ^= n >> 8;
	n ^= n >> 16;

	return n & 1;
}

void dcf_addBit(unsigned short pulse, unsigned short delta) {
	static union {
		unsigned long long bits;
		DCFCheck check;
		DCF dcf;
	} buffer = { 0 };

	static signed char offset = -1;

	const int pulseOk = (50 <= pulse && pulse <= 250);
	const int deltaOk1 = (950 <= delta && delta <= 1050);
	const int deltaOk2 = (1950 <= delta && delta <= 2050);
	const int deltaOk = deltaOk1 | deltaOk2;
	const int isLast = deltaOk2;

	if (pulseOk && deltaOk) {
		if (0 <= offset && offset <= 60) {
			if (pulse > 150) {
				buffer.bits |= 1ull << offset;
			}
			++offset;
		} else {
			offset = -1;
		}

		if (isLast) {
			if (offset == 59 || offset == 60) {
				if (!buffer.check.zero && buffer.check.one
						&& !parity(buffer.check.minute)
						&& !parity(buffer.check.hour)
						&& !parity(buffer.check.date)) {

					dcf_handleTelegram(&buffer.dcf);
				}
			}

			offset = 0;
			buffer.bits = 0;
		}
	} else {
		offset = -1;
	}
}
