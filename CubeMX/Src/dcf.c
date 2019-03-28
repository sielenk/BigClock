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

#define BUFFER_SIZE 65

void dcf_addBit(unsigned short pulse, unsigned short delta) {
	static unsigned short buffer[BUFFER_SIZE] = { 0 };
	static unsigned short* p = buffer;

	const int pulseOk = (50 <= pulse && pulse <= 250);
	const int deltaOk1 = (950 <= delta && delta <= 1050);
	const int deltaOk2 = (1950 <= delta && delta <= 2050);
	const int deltaOk = deltaOk1 | deltaOk2;
	const int isLast = deltaOk2;

	if (pulseOk && deltaOk) {
		*p++ = pulse;

		if (isLast) {
			const int count = p - buffer;
			p = buffer;

			if (count == 59) {
				union {
					unsigned long long buffer;
					DCF dcf;
					DCFCheck check;
				} u = { 0 };

				for (int i = 0; i < 59; ++i) {
					if (buffer[i] > 150) {
						u.buffer |= 1ull << i;
					}
				}

				if (!u.check.zero && u.check.one && !parity(u.check.minute)
						&& !parity(u.check.hour) && !parity(u.check.date)) {

					dcf_handleTelegram(&u.dcf);
				}
			}
		} else if (p > buffer + BUFFER_SIZE) {
			p = buffer;
		}
	} else {
		p = buffer;
	}
}
