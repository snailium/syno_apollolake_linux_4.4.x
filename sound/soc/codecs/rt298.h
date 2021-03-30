/*
 * rt298.h  --  RT298 ALSA SoC audio driver
 *
 * Copyright 2011 Realtek Microelectronics
 * Author: Johnny Hsu <johnnyhsu@realtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RT298_H__
#define __RT298_H__

#define VERB_CMD(V, N, D) ((N << 20) | (V << 8) | D)

#define RT298_AUDIO_FUNCTION_GROUP			0x01
#define RT298_DAC_OUT1					0x02
#define RT298_DAC_OUT2					0x03
#define RT298_DIG_CVT					0x06
#define RT298_ADC_IN1					0x09
#define RT298_ADC_IN2					0x08
#define RT298_MIXER_IN					0x0b
#define RT298_MIXER_OUT1				0x0c
#define RT298_MIXER_OUT2				0x0d
#define RT298_DMIC1					0x12
#define RT298_DMIC2					0x13
#define RT298_SPK_OUT					0x14
#define RT298_MIC1					0x18
#define RT298_LINE1					0x1a
#define RT298_BEEP					0x1d
#define RT298_SPDIF					0x1e
#define RT298_VENDOR_REGISTERS				0x20
#define RT298_HP_OUT					0x21
#define RT298_MIXER_IN1					0x22
#define RT298_MIXER_IN2					0x23

#define RT298_SET_PIN_SFT				6
#define RT298_SET_PIN_ENABLE				0x40
#define RT298_SET_PIN_DISABLE				0
#define RT298_SET_EAPD_HIGH				0x2
#define RT298_SET_EAPD_LOW				0

#define RT298_MUTE_SFT					7

/* Verb commands */
#define RT298_GET_PARAM(NID, PARAM) VERB_CMD(AC_VERB_PARAMETERS, NID, PARAM)
#define RT298_SET_POWER(NID) VERB_CMD(AC_VERB_SET_POWER_STATE, NID, 0)
#define RT298_SET_AUDIO_POWER RT298_SET_POWER(RT298_AUDIO_FUNCTION_GROUP)
#define RT298_SET_HPO_POWER RT298_SET_POWER(RT298_HP_OUT)
#define RT298_SET_SPK_POWER RT298_SET_POWER(RT298_SPK_OUT)
#define RT298_SET_DMIC1_POWER RT298_SET_POWER(RT298_DMIC1)
#define RT298_SPK_MUX\
	VERB_CMD(AC_VERB_SET_CONNECT_SEL, RT298_SPK_OUT, 0)
#define RT298_HPO_MUX\
	VERB_CMD(AC_VERB_SET_CONNECT_SEL, RT298_HP_OUT, 0)
#define RT298_ADC0_MUX\
	VERB_CMD(AC_VERB_SET_CONNECT_SEL, RT298_MIXER_IN1, 0)
#define RT298_ADC1_MUX\
	VERB_CMD(AC_VERB_SET_CONNECT_SEL, RT298_MIXER_IN2, 0)
#define RT298_SET_MIC1\
	VERB_CMD(AC_VERB_SET_PIN_WIDGET_CONTROL, RT298_MIC1, 0)
#define RT298_SET_PIN_HPO\
	VERB_CMD(AC_VERB_SET_PIN_WIDGET_CONTROL, RT298_HP_OUT, 0)
#define RT298_SET_PIN_SPK\
	VERB_CMD(AC_VERB_SET_PIN_WIDGET_CONTROL, RT298_SPK_OUT, 0)
#define RT298_SET_PIN_DMIC1\
	VERB_CMD(AC_VERB_SET_PIN_WIDGET_CONTROL, RT298_DMIC1, 0)
#define RT298_SET_PIN_SPDIF\
	VERB_CMD(AC_VERB_SET_PIN_WIDGET_CONTROL, RT298_SPDIF, 0)
#define RT298_SET_PIN_DIG_CVT\
	VERB_CMD(AC_VERB_SET_DIGI_CONVERT_1, RT298_DIG_CVT, 0)
#define RT298_SPK_EAPD\
	VERB_CMD(AC_VERB_SET_EAPD_BTLENABLE, RT298_SPK_OUT, 0)
#define RT298_SET_AMP_GAIN_HPO\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_HP_OUT, 0)
#define RT298_SET_AMP_GAIN_ADC_IN1\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_ADC_IN1, 0)
#define RT298_SET_AMP_GAIN_ADC_IN2\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_ADC_IN2, 0)
#define RT298_GET_HP_SENSE\
	VERB_CMD(AC_VERB_GET_PIN_SENSE, RT298_HP_OUT, 0)
#define RT298_GET_MIC1_SENSE\
	VERB_CMD(AC_VERB_GET_PIN_SENSE, RT298_MIC1, 0)
#define RT298_SET_DMIC2_DEFAULT\
	VERB_CMD(AC_VERB_SET_CONFIG_DEFAULT_BYTES_3, RT298_DMIC2, 0)
#define RT298_SET_SPDIF_DEFAULT\
	VERB_CMD(AC_VERB_SET_CONFIG_DEFAULT_BYTES_3, RT298_SPDIF, 0)
#define RT298_DACL_GAIN\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_DAC_OUT1, 0xa000)
#define RT298_DACR_GAIN\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_DAC_OUT1, 0x9000)
#define RT298_ADCL_GAIN\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_ADC_IN1, 0x6000)
#define RT298_ADCR_GAIN\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_ADC_IN1, 0x5000)
#define RT298_MIC_GAIN\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_MIC1, 0x7000)
#define RT298_SPOL_GAIN\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_SPK_OUT, 0xa000)
#define RT298_SPOR_GAIN\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_SPK_OUT, 0x9000)
#define RT298_HPOL_GAIN\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_HP_OUT, 0xa000)
#define RT298_HPOR_GAIN\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_HP_OUT, 0x9000)
#define RT298_F_DAC_SWITCH\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_MIXER_OUT1, 0x7000)
#define RT298_F_RECMIX_SWITCH\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_MIXER_OUT1, 0x7100)
#define RT298_REC_MIC_SWITCH\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_MIXER_IN, 0x7000)
#define RT298_REC_I2S_SWITCH\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_MIXER_IN, 0x7100)
#define RT298_REC_LINE_SWITCH\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_MIXER_IN, 0x7200)
#define RT298_REC_BEEP_SWITCH\
	VERB_CMD(AC_VERB_SET_AMP_GAIN_MUTE, RT298_MIXER_IN, 0x7300)
#define RT298_DAC_FORMAT\
	VERB_CMD(AC_VERB_SET_STREAM_FORMAT, RT298_DAC_OUT1, 0)
#define RT298_ADC_FORMAT\
	VERB_CMD(AC_VERB_SET_STREAM_FORMAT, RT298_ADC_IN1, 0)
#define RT298_COEF_INDEX\
	VERB_CMD(AC_VERB_SET_COEF_INDEX, RT298_VENDOR_REGISTERS, 0)
#define RT298_PROC_COEF\
	VERB_CMD(AC_VERB_SET_PROC_COEF, RT298_VENDOR_REGISTERS, 0)

/* Index registers */
#define RT298_A_BIAS_CTRL1	0x01
#define RT298_A_BIAS_CTRL2	0x02
#define RT298_POWER_CTRL1	0x03
#define RT298_A_BIAS_CTRL3	0x04
#define RT298_POWER_CTRL2	0x08
#define RT298_I2S_CTRL1		0x09
#define RT298_I2S_CTRL2		0x0a
#define RT298_CLK_DIV		0x0b
#define RT298_DC_GAIN		0x0d
#define RT298_POWER_CTRL3	0x0f
#define RT298_MIC1_DET_CTRL	0x19
#define RT298_MISC_CTRL1	0x20
#define RT298_IRQ_CTRL		0x33
#define RT298_WIND_FILTER_CTRL	0x46
#define RT298_PLL_CTRL1		0x49
#define RT298_CBJ_CTRL1		0x4f
#define RT298_CBJ_CTRL2		0x50
#define RT298_PLL_CTRL		0x63
#define RT298_DEPOP_CTRL1	0x66
#define RT298_DEPOP_CTRL2	0x67
#define RT298_DEPOP_CTRL3	0x68
#define RT298_DEPOP_CTRL4	0x69

/* SPDIF (0x06) */
#define RT298_SPDIF_SEL_SFT	0
#define RT298_SPDIF_SEL_PCM0	0
#define RT298_SPDIF_SEL_PCM1	1
#define RT298_SPDIF_SEL_SPOUT	2
#define RT298_SPDIF_SEL_PP	3

/* RECMIX (0x0b) */
#define RT298_M_REC_BEEP_SFT	0
#define RT298_M_REC_LINE1_SFT	1
#define RT298_M_REC_MIC1_SFT	2
#define RT298_M_REC_I2S_SFT	3

/* Front (0x0c) */
#define RT298_M_FRONT_DAC_SFT	0
#define RT298_M_FRONT_REC_SFT	1

/* SPK-OUT (0x14) */
#define RT298_M_SPK_MUX_SFT	14
#define RT298_SPK_SEL_MASK	0x1
#define RT298_SPK_SEL_SFT	0
#define RT298_SPK_SEL_F		0
#define RT298_SPK_SEL_S		1

/* HP-OUT (0x21) */
#define RT298_M_HP_MUX_SFT	14
#define RT298_HP_SEL_MASK	0x1
#define RT298_HP_SEL_SFT	0
#define RT298_HP_SEL_F		0
#define RT298_HP_SEL_S		1

/* ADC (0x22) (0x23) */
#define RT298_ADC_SEL_MASK	0x7
#define RT298_ADC_SEL_SFT	0
#define RT298_ADC_SEL_SURR	0
#define RT298_ADC_SEL_FRONT	1
#define RT298_ADC_SEL_DMIC	2
#define RT298_ADC_SEL_BEEP	4
#define RT298_ADC_SEL_LINE1	5
#define RT298_ADC_SEL_I2S	6
#define RT298_ADC_SEL_MIC1	7

#define RT298_SCLK_S_MCLK	0
#define RT298_SCLK_S_PLL	1

enum {
	RT298_AIF1,
	RT298_AIF2,
	RT298_AIFS,
};

int rt298_mic_detect(struct snd_soc_codec *codec, struct snd_soc_jack *jack);

#endif /* __RT298_H__ */
