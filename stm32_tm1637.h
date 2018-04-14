#ifndef STM32_TM1637_H_
#define STM32_TM1637_H_

#define SEG_None 16
#define SEG_A 1
#define SEG_B 2
#define SEG_C 4
#define SEG_D 8
#define SEG_E 16
#define SEG_F 32
#define SEG_G 64

///// DEFINE REGISTERS
#define WriteDataToDispReg 0x40
#define AdressCmd 0xc0
#define DisplCntrl 0x87

void tm1637Init(void);
void tm1637DisplayDecimal(int v, int displaySeparator);
void tm1637SetBrightness(char brightness);
void tm1637DisplayDigit (int ch, uint8_t column);
void tm1637ShowColon (uint8_t show);
void tm1637DisplayChar (int ch, uint8_t column);

extern const char segmentMap[];  //you can use segmentMap in main.c

__STATIC_INLINE void Delay(__IO uint32_t micros) {

#if !defined(STM32F0xx)
	uint32_t start = DWT->CYCCNT;

	/* Go to number of cycles for system */
	micros *= (HAL_RCC_GetHCLKFreq() / 1000000);

	/* Delay till end */
	while ((DWT->CYCCNT - start) < micros);
#else
	/* Go to clock cycles */
	micros *= (SystemCoreClock / 1000000) / 5;

	/* Wait till done */
	while (micros--);
#endif
}

#endif
