#ifndef __CODEC_ROUTE_NAME_H__
#define __CODEC_ROUTE_NAME_H__

//#define USE_4780_INTERNEL_CODEC 1

#ifdef USE_4780_INTERNEL_CODEC

#define CODEC_MODE_OFF                 (0x1U << 0)

#define CODEC_MODE_AIPN1_2_ADC         (0x1U << 1)//adc
#define CODEC_MODE_AIP2_2_ADC          (0x1U << 2)
#define CODEC_MODE_AIP3_2_ADC          (0x1U << 3)
#define CODEC_MODE_AIP2_AIP3_2_ADC     (0x1U << 4)

#define CODEC_MODE_DACL_DACR_2_HP      (0x1U << 6)//hp
#define CODEC_MODE_DACL_2_HP           (0x1U << 7)
#define CODEC_MODE_AIPN1_2_HP          (0x1U << 8) //bypass
#define CODEC_MODE_AIP2_2_HP           (0x1U << 9)
#define CODEC_MODE_AIP3_2_HP           (0x1U << 10)
#define CODEC_MODE_AIP2_AIP3_2_HP      (0x1U << 11)

#define CODEC_MODE_DACL_2_LO            (0x1U << 12)//line out
#define CODEC_MODE_DACR_2_LO            (0x1U << 13)
#define CODEC_MODE_DACL_DACR_2_LO       (0x1U << 14)
#define CODEC_MODE_AIPN1_2_LO           (0x1U << 15)//bypass
#define CODEC_MODE_AIP2_2_LO            (0x1U << 16)
#define CODEC_MODE_AIP3_2_LO            (0x1U << 17)
#define CODEC_MODE_AIP2_AIP3_2_LO       (0x1U << 18)

#else  //4775 and 4770

//mode
#define CODEC_MODE_OFF         (0x1U << 0)
#define CODEC_MODE_MIC1_2_ADC         (0x1U << 1)//adc
#define CODEC_MODE_MIC2_2_ADC         (0x1U << 2)
#define CODEC_MODE_MIC1_MIC2_2_ADC    (0x1U << 3)
#define CODEC_MODE_MIC2_MIC1_2_ADC    (0x1U << 4)
#define CODEC_MODE_LINE_IN_2_ADC      (0x1U << 5)
#define CODEC_MODE_MIC1_2_HP       (0x1U << 6)//hp
#define CODEC_MODE_MIC2_2_HP       (0x1U << 7)
#define CODEC_MODE_MIC1_MIC2_2_HP  (0x1U << 8)
#define CODEC_MODE_MIC2_MIC1_2_HP  (0x1U << 9)
#define CODEC_MODE_LINE_IN_2_HP    (0x1U << 10)
#define CODEC_MODE_DAC_2_HP        (0x1U << 11)
#define CODEC_MODE_MIC1_2_LO            (0x1U << 12)//line out
#define CODEC_MODE_MIC2_2_LO            (0x1U << 13)
#define CODEC_MODE_MIC1_MIC2_2_LO       (0x1U << 14)
#define CODEC_MODE_LINE_IN_2_LO         (0x1U << 15)
#define CODEC_MODE_DAC_2_LO             (0x1U << 16)
#define CODEC_MODE_MIC1_2_JD       (0x1U << 17)//jack detect
#define CODEC_MODE_MIC2_2_JD       (0x1U << 18)
#define CODEC_MODE_MIC1_MIC2_2_JD  (0x1U << 19)
#define CODEC_MODE_MIC2_MIC1_2_JD  (0x1U << 20)
#define CODEC_MODE_LINE_IN_2_JD    (0x1U << 21)
#define CODEC_MODE_DAC_2_JD        (0x1U << 22)

#endif

#define CODEC_SET_MODE          SOUND_MIXER_PRIVATE1

#define CODEC_SET_GCR           SOUND_MIXER_PRIVATE2

#endif /* __CODEC_ROUTE_NAME_H__ */
