#ifndef __PCM1770_H__
#define __PCM1770_H__

#define REGISTER01 	0x01
#define MUTR		0x80
#define MUTL		0x40
#define ATL(level)	(level&0x3F)

#define REGISTER02 	0x02
#define ATR(level)	(level&0x3F)

#define REGISTER03 	0x03
#define OVER		0x80 // 0 128fs  1 192/256/384fs
#define RINV		0x20 
#define AMIX		0x10
#define DEM			0x80 // 44.1kHz De-Emphasis Control

#define FMT(fmt)	(fmt&0x07)
#define FMT_LJF_16_24	0
#define FMT_IIS_16_24	1
#define FMT_RJ_24		2
#define FMT_RJ_20		3
#define FMT_RJ_16		4
#define FMT_LJF_MM		2

#define REGISTER04 	0x04
#define ZCAT		0x10
#define PWRD		0x01

#endif
