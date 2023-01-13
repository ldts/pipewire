/* Simple Plugin API
 *
 * Copyright © 2018 Wim Taymans
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef SPA_AUDIO_RAW_TYPES_H
#define SPA_AUDIO_RAW_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup spa_param
 * \{
 */

#include <spa/param/audio/raw.h>

#define SPA_TYPE_INFO_AudioFormat		SPA_TYPE_INFO_ENUM_BASE "AudioFormat"
#define SPA_TYPE_INFO_AUDIO_FORMAT_BASE		SPA_TYPE_INFO_AudioFormat ":"

static const struct spa_type_info spa_type_audio_format[] = {
	{ SPA_AUDIO_FORMAT_UNKNOWN, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "UNKNOWN", NULL },
	{ SPA_AUDIO_FORMAT_ENCODED, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "ENCODED", NULL },
	{ SPA_AUDIO_FORMAT_S8, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S8", NULL },
	{ SPA_AUDIO_FORMAT_U8, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U8", NULL },
	{ SPA_AUDIO_FORMAT_S16_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S16LE", NULL },
	{ SPA_AUDIO_FORMAT_S16_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S16BE", NULL },
	{ SPA_AUDIO_FORMAT_U16_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U16LE", NULL },
	{ SPA_AUDIO_FORMAT_U16_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U16BE", NULL },
	{ SPA_AUDIO_FORMAT_S24_32_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24_32LE", NULL },
	{ SPA_AUDIO_FORMAT_S24_32_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24_32BE", NULL },
	{ SPA_AUDIO_FORMAT_U24_32_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U24_32LE", NULL },
	{ SPA_AUDIO_FORMAT_U24_32_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U24_32BE", NULL },
	{ SPA_AUDIO_FORMAT_S32_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S32LE", NULL },
	{ SPA_AUDIO_FORMAT_S32_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S32BE", NULL },
	{ SPA_AUDIO_FORMAT_U32_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U32LE", NULL },
	{ SPA_AUDIO_FORMAT_U32_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U32BE", NULL },
	{ SPA_AUDIO_FORMAT_S24_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24LE", NULL },
	{ SPA_AUDIO_FORMAT_S24_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24BE", NULL },
	{ SPA_AUDIO_FORMAT_U24_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U24LE", NULL },
	{ SPA_AUDIO_FORMAT_U24_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U24BE", NULL },
	{ SPA_AUDIO_FORMAT_S20_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S20LE", NULL },
	{ SPA_AUDIO_FORMAT_S20_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S20BE", NULL },
	{ SPA_AUDIO_FORMAT_U20_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U20LE", NULL },
	{ SPA_AUDIO_FORMAT_U20_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U20BE", NULL },
	{ SPA_AUDIO_FORMAT_S18_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S18LE", NULL },
	{ SPA_AUDIO_FORMAT_S18_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S18BE", NULL },
	{ SPA_AUDIO_FORMAT_U18_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U18LE", NULL },
	{ SPA_AUDIO_FORMAT_U18_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U18BE", NULL },
	{ SPA_AUDIO_FORMAT_F32_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F32LE", NULL },
	{ SPA_AUDIO_FORMAT_F32_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F32BE", NULL },
	{ SPA_AUDIO_FORMAT_F64_LE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F64LE", NULL },
	{ SPA_AUDIO_FORMAT_F64_BE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F64BE", NULL },

	{ SPA_AUDIO_FORMAT_ULAW, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "ULAW", NULL },
	{ SPA_AUDIO_FORMAT_ALAW, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "ALAW", NULL },

	{ SPA_AUDIO_FORMAT_U8P, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U8P", NULL },
	{ SPA_AUDIO_FORMAT_S16P, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S16P", NULL },
	{ SPA_AUDIO_FORMAT_S24_32P, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24_32P", NULL },
	{ SPA_AUDIO_FORMAT_S32P, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S32P", NULL },
	{ SPA_AUDIO_FORMAT_S24P, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24P", NULL },
	{ SPA_AUDIO_FORMAT_F32P, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F32P", NULL },
	{ SPA_AUDIO_FORMAT_F64P, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F64P", NULL },
	{ SPA_AUDIO_FORMAT_S8P, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S8P", NULL },

#if __BYTE_ORDER == __BIG_ENDIAN
	{ SPA_AUDIO_FORMAT_S16_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S16OE", NULL },
	{ SPA_AUDIO_FORMAT_S16, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S16", NULL },
	{ SPA_AUDIO_FORMAT_U16_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U16OE", NULL },
	{ SPA_AUDIO_FORMAT_U16, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U16", NULL },
	{ SPA_AUDIO_FORMAT_S24_32_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24_32OE", NULL },
	{ SPA_AUDIO_FORMAT_S24_32, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24_32", NULL },
	{ SPA_AUDIO_FORMAT_U24_32_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U24_32OE", NULL },
	{ SPA_AUDIO_FORMAT_U24_32, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U24_32", NULL },
	{ SPA_AUDIO_FORMAT_S32_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S32OE", NULL },
	{ SPA_AUDIO_FORMAT_S32, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S32", NULL },
	{ SPA_AUDIO_FORMAT_U32_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U32OE", NULL },
	{ SPA_AUDIO_FORMAT_U32, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U32", NULL },
	{ SPA_AUDIO_FORMAT_S24_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24OE", NULL },
	{ SPA_AUDIO_FORMAT_S24, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24", NULL },
	{ SPA_AUDIO_FORMAT_U24_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U24OE", NULL },
	{ SPA_AUDIO_FORMAT_U24, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U24", NULL },
	{ SPA_AUDIO_FORMAT_S20_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S20OE", NULL },
	{ SPA_AUDIO_FORMAT_S20, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S20", NULL },
	{ SPA_AUDIO_FORMAT_U20_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U20OE", NULL },
	{ SPA_AUDIO_FORMAT_U20, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U20", NULL },
	{ SPA_AUDIO_FORMAT_S18_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S18OE", NULL },
	{ SPA_AUDIO_FORMAT_S18, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S18", NULL },
	{ SPA_AUDIO_FORMAT_U18_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U18OE", NULL },
	{ SPA_AUDIO_FORMAT_U18, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U18", NULL },
	{ SPA_AUDIO_FORMAT_F32_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F32OE", NULL },
	{ SPA_AUDIO_FORMAT_F32, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F32", NULL },
	{ SPA_AUDIO_FORMAT_F64_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F64OE", NULL },
	{ SPA_AUDIO_FORMAT_F64, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F64", NULL },
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	{ SPA_AUDIO_FORMAT_S16, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S16", NULL },
	{ SPA_AUDIO_FORMAT_S16_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S16OE", NULL },
	{ SPA_AUDIO_FORMAT_U16, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U16", NULL },
	{ SPA_AUDIO_FORMAT_U16_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U16OE", NULL },
	{ SPA_AUDIO_FORMAT_S24_32, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24_32", NULL },
	{ SPA_AUDIO_FORMAT_S24_32_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24_32OE", NULL },
	{ SPA_AUDIO_FORMAT_U24_32, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U24_32", NULL },
	{ SPA_AUDIO_FORMAT_U24_32_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U24_32OE", NULL },
	{ SPA_AUDIO_FORMAT_S32, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S32", NULL },
	{ SPA_AUDIO_FORMAT_S32_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S32OE", NULL },
	{ SPA_AUDIO_FORMAT_U32, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U32", NULL },
	{ SPA_AUDIO_FORMAT_U32_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U32OE", NULL },
	{ SPA_AUDIO_FORMAT_S24, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24", NULL },
	{ SPA_AUDIO_FORMAT_S24_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S24OE", NULL },
	{ SPA_AUDIO_FORMAT_U24, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U24", NULL },
	{ SPA_AUDIO_FORMAT_U24_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U24OE", NULL },
	{ SPA_AUDIO_FORMAT_S20, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S20", NULL },
	{ SPA_AUDIO_FORMAT_S20_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S20OE", NULL },
	{ SPA_AUDIO_FORMAT_U20, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U20", NULL },
	{ SPA_AUDIO_FORMAT_U20_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U20OE", NULL },
	{ SPA_AUDIO_FORMAT_S18, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S18", NULL },
	{ SPA_AUDIO_FORMAT_S18_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "S18OE", NULL },
	{ SPA_AUDIO_FORMAT_U18, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U18", NULL },
	{ SPA_AUDIO_FORMAT_U18_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "U18OE", NULL },
	{ SPA_AUDIO_FORMAT_F32, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F32", NULL },
	{ SPA_AUDIO_FORMAT_F32_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F32OE", NULL },
	{ SPA_AUDIO_FORMAT_F64, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F64", NULL },
	{ SPA_AUDIO_FORMAT_F64_OE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FORMAT_BASE "F64OE", NULL },
#endif
	{ 0, 0, NULL, NULL },
};

#define SPA_TYPE_INFO_AudioFlags	SPA_TYPE_INFO_FLAGS_BASE "AudioFlags"
#define SPA_TYPE_INFO_AUDIO_FLAGS_BASE	SPA_TYPE_INFO_AudioFlags ":"

static const struct spa_type_info spa_type_audio_flags[] = {
	{ SPA_AUDIO_FLAG_NONE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FLAGS_BASE "none", NULL },
	{ SPA_AUDIO_FLAG_UNPOSITIONED, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_FLAGS_BASE "unpositioned", NULL },
	{ 0, 0, NULL, NULL },
};

#define SPA_TYPE_INFO_AudioChannel		SPA_TYPE_INFO_ENUM_BASE "AudioChannel"
#define SPA_TYPE_INFO_AUDIO_CHANNEL_BASE	SPA_TYPE_INFO_AudioChannel ":"

static const struct spa_type_info spa_type_audio_channel[] = {
	{ SPA_AUDIO_CHANNEL_UNKNOWN, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "UNK", NULL },
	{ SPA_AUDIO_CHANNEL_NA,	SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "NA", NULL },
	{ SPA_AUDIO_CHANNEL_MONO, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "MONO", NULL },
	{ SPA_AUDIO_CHANNEL_FL, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "FL", NULL },
	{ SPA_AUDIO_CHANNEL_FR, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "FR", NULL },
	{ SPA_AUDIO_CHANNEL_FC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "FC", NULL },
	{ SPA_AUDIO_CHANNEL_LFE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "LFE", NULL },
	{ SPA_AUDIO_CHANNEL_SL, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "SL", NULL },
	{ SPA_AUDIO_CHANNEL_SR, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "SR", NULL },
	{ SPA_AUDIO_CHANNEL_FLC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "FLC", NULL },
	{ SPA_AUDIO_CHANNEL_FRC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "FRC", NULL },
	{ SPA_AUDIO_CHANNEL_RC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "RC", NULL },
	{ SPA_AUDIO_CHANNEL_RL, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "RL", NULL },
	{ SPA_AUDIO_CHANNEL_RR, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "RR", NULL },
	{ SPA_AUDIO_CHANNEL_TC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "TC", NULL },
	{ SPA_AUDIO_CHANNEL_TFL, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "TFL", NULL },
	{ SPA_AUDIO_CHANNEL_TFC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "TFC", NULL },
	{ SPA_AUDIO_CHANNEL_TFR, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "TFR", NULL },
	{ SPA_AUDIO_CHANNEL_TRL, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "TRL", NULL },
	{ SPA_AUDIO_CHANNEL_TRC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "TRC", NULL },
	{ SPA_AUDIO_CHANNEL_TRR, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "TRR", NULL },
	{ SPA_AUDIO_CHANNEL_RLC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "RLC", NULL },
	{ SPA_AUDIO_CHANNEL_RRC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "RRC", NULL },
	{ SPA_AUDIO_CHANNEL_FLW, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "FLW", NULL },
	{ SPA_AUDIO_CHANNEL_FRW, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "FRW", NULL },
	{ SPA_AUDIO_CHANNEL_LFE2, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "LFE2", NULL },
	{ SPA_AUDIO_CHANNEL_FLH, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "FLH", NULL },
	{ SPA_AUDIO_CHANNEL_FCH, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "FCH", NULL },
	{ SPA_AUDIO_CHANNEL_FRH, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "FRH", NULL },
	{ SPA_AUDIO_CHANNEL_TFLC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "TFLC", NULL },
	{ SPA_AUDIO_CHANNEL_TFRC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "TFRC", NULL },
	{ SPA_AUDIO_CHANNEL_TSL, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "TSL", NULL },
	{ SPA_AUDIO_CHANNEL_TSR, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "TSR", NULL },
	{ SPA_AUDIO_CHANNEL_LLFE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "LLFR", NULL },
	{ SPA_AUDIO_CHANNEL_RLFE, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "RLFE", NULL },
	{ SPA_AUDIO_CHANNEL_BC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "BC", NULL },
	{ SPA_AUDIO_CHANNEL_BLC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "BLC", NULL },
	{ SPA_AUDIO_CHANNEL_BRC, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "BRC", NULL },

	{ SPA_AUDIO_CHANNEL_AUX0, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX0", NULL },
	{ SPA_AUDIO_CHANNEL_AUX1, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX1", NULL },
	{ SPA_AUDIO_CHANNEL_AUX2, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX2", NULL },
	{ SPA_AUDIO_CHANNEL_AUX3, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX3", NULL },
	{ SPA_AUDIO_CHANNEL_AUX4, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX4", NULL },
	{ SPA_AUDIO_CHANNEL_AUX5, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX5", NULL },
	{ SPA_AUDIO_CHANNEL_AUX6, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX6", NULL },
	{ SPA_AUDIO_CHANNEL_AUX7, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX7", NULL },
	{ SPA_AUDIO_CHANNEL_AUX8, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX8", NULL },
	{ SPA_AUDIO_CHANNEL_AUX9, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX9", NULL },
	{ SPA_AUDIO_CHANNEL_AUX10, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX10", NULL },
	{ SPA_AUDIO_CHANNEL_AUX11, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX11", NULL },
	{ SPA_AUDIO_CHANNEL_AUX12, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX12", NULL },
	{ SPA_AUDIO_CHANNEL_AUX13, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX13", NULL },
	{ SPA_AUDIO_CHANNEL_AUX14, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX14", NULL },
	{ SPA_AUDIO_CHANNEL_AUX15, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX15", NULL },
	{ SPA_AUDIO_CHANNEL_AUX16, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX16", NULL },
	{ SPA_AUDIO_CHANNEL_AUX17, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX17", NULL },
	{ SPA_AUDIO_CHANNEL_AUX18, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX18", NULL },
	{ SPA_AUDIO_CHANNEL_AUX19, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX19", NULL },
	{ SPA_AUDIO_CHANNEL_AUX20, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX20", NULL },
	{ SPA_AUDIO_CHANNEL_AUX21, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX21", NULL },
	{ SPA_AUDIO_CHANNEL_AUX22, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX22", NULL },
	{ SPA_AUDIO_CHANNEL_AUX23, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX23", NULL },
	{ SPA_AUDIO_CHANNEL_AUX24, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX24", NULL },
	{ SPA_AUDIO_CHANNEL_AUX25, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX25", NULL },
	{ SPA_AUDIO_CHANNEL_AUX26, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX26", NULL },
	{ SPA_AUDIO_CHANNEL_AUX27, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX27", NULL },
	{ SPA_AUDIO_CHANNEL_AUX28, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX28", NULL },
	{ SPA_AUDIO_CHANNEL_AUX29, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX29", NULL },
	{ SPA_AUDIO_CHANNEL_AUX30, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX30", NULL },
	{ SPA_AUDIO_CHANNEL_AUX31, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX31", NULL },
	{ SPA_AUDIO_CHANNEL_AUX32, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX32", NULL },
	{ SPA_AUDIO_CHANNEL_AUX33, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX33", NULL },
	{ SPA_AUDIO_CHANNEL_AUX34, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX34", NULL },
	{ SPA_AUDIO_CHANNEL_AUX35, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX35", NULL },
	{ SPA_AUDIO_CHANNEL_AUX36, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX36", NULL },
	{ SPA_AUDIO_CHANNEL_AUX37, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX37", NULL },
	{ SPA_AUDIO_CHANNEL_AUX38, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX38", NULL },
	{ SPA_AUDIO_CHANNEL_AUX39, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX39", NULL },
	{ SPA_AUDIO_CHANNEL_AUX40, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX40", NULL },
	{ SPA_AUDIO_CHANNEL_AUX41, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX41", NULL },
	{ SPA_AUDIO_CHANNEL_AUX42, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX42", NULL },
	{ SPA_AUDIO_CHANNEL_AUX43, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX43", NULL },
	{ SPA_AUDIO_CHANNEL_AUX44, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX44", NULL },
	{ SPA_AUDIO_CHANNEL_AUX45, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX45", NULL },
	{ SPA_AUDIO_CHANNEL_AUX46, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX46", NULL },
	{ SPA_AUDIO_CHANNEL_AUX47, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX47", NULL },
	{ SPA_AUDIO_CHANNEL_AUX48, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX48", NULL },
	{ SPA_AUDIO_CHANNEL_AUX49, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX49", NULL },
	{ SPA_AUDIO_CHANNEL_AUX50, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX50", NULL },
	{ SPA_AUDIO_CHANNEL_AUX51, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX51", NULL },
	{ SPA_AUDIO_CHANNEL_AUX52, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX52", NULL },
	{ SPA_AUDIO_CHANNEL_AUX53, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX53", NULL },
	{ SPA_AUDIO_CHANNEL_AUX54, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX54", NULL },
	{ SPA_AUDIO_CHANNEL_AUX55, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX55", NULL },
	{ SPA_AUDIO_CHANNEL_AUX56, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX56", NULL },
	{ SPA_AUDIO_CHANNEL_AUX57, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX57", NULL },
	{ SPA_AUDIO_CHANNEL_AUX58, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX58", NULL },
	{ SPA_AUDIO_CHANNEL_AUX59, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX59", NULL },
	{ SPA_AUDIO_CHANNEL_AUX60, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX60", NULL },
	{ SPA_AUDIO_CHANNEL_AUX61, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX61", NULL },
	{ SPA_AUDIO_CHANNEL_AUX62, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX62", NULL },
	{ SPA_AUDIO_CHANNEL_AUX63, SPA_TYPE_Int, SPA_TYPE_INFO_AUDIO_CHANNEL_BASE "AUX63", NULL },
	{ 0, 0, NULL, NULL },
};

/**
 * \}
 */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SPA_AUDIO_RAW_RAW_TYPES_H */
