/**
 ******************************************************************************
 * File Name          : dcf.h
 * Description        : This file provides code for the DCF decoder.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 Marvin Sielenkemper.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

#ifndef __DCF_H
#define __DCF_H

#ifdef __cplusplus
extern "C" {
#endif

void dcf_addBit(unsigned short pulse, unsigned short delta);

typedef struct {
	unsigned int :16;
	unsigned int changeMezMesz :1;
	unsigned int isMesz :1;
	unsigned int isMez :1;
	unsigned int changeSec :1;
	unsigned int :1;
	unsigned int minute01 :4;
	unsigned int minute10 :3;
	unsigned int :1;
	unsigned int hour01 :4;
	unsigned int hour10 :2;
	unsigned int :1;
	unsigned int day01 :4;
	unsigned int day10 :2;
	unsigned int weekday :3;
	unsigned int month01 :4;
	unsigned int month10 :1;
	unsigned int year01 :4;
	unsigned int year10 :4;
}__attribute__((packed)) DCF;

void dcf_handleTelegram(DCF* dcf);

#ifdef __cplusplus
}
#endif

#endif
