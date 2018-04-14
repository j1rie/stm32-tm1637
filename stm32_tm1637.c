#include "stm32f4xx_hal.h"
#include "stm32_tm1637.h"

// Internal Functions
void _tm1637Start(void);
void _tm1637Stop(void);
void _tm1637ReadResult(void);
void _tm1637WriteByte(unsigned char b);
uint32_t tm1637_TM_DELAY_Init(void);
const char segmentMap[] = { (SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F), //0
		(SEG_B + SEG_C), //1
		(SEG_A + SEG_B + SEG_D + SEG_E + SEG_G), //2
		(SEG_A + SEG_B + SEG_C + SEG_D + SEG_G), //3
		(SEG_B + SEG_C + SEG_F + SEG_G), //4
		(SEG_A + SEG_C + SEG_D + SEG_F + SEG_G), //5
		(SEG_A + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G), //6
		(SEG_A + SEG_B + SEG_C + SEG_F), //7
		(SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G), //8
		(SEG_A + SEG_B + SEG_C + SEG_D + SEG_F + SEG_G), //9
		(SEG_A + SEG_B + SEG_C + SEG_E + SEG_F + SEG_G), //A
		(SEG_C + SEG_D + SEG_E + SEG_F + SEG_G), //b
		(SEG_A + SEG_D + SEG_E + SEG_F), //C
		(SEG_B + SEG_C + SEG_D + SEG_E + SEG_G), //d
		(SEG_A + SEG_D + SEG_E + SEG_F + SEG_G), //E
		(SEG_A + SEG_E + SEG_F + SEG_G), //F
		0x00 }; // None
uint8_t _dig1 = None;

void tm1637Init(void) {
	tm1637_TM_DELAY_Init();
	CLK_PORT_CLK_ENABLE();
	DIO_PORT_CLK_ENABLE();
	GPIO_InitTypeDef g;
	g.Pull = GPIO_PULLUP;
	g.Mode = GPIO_MODE_OUTPUT_OD; // OD = open drain
	g.Speed = GPIO_SPEED_FREQ_LOW;
	g.Pin = CLK_PIN;
	HAL_GPIO_Init(CLK_PORT, &g);
	g.Pin = DIO_PIN;
	HAL_GPIO_Init(DIO_PORT, &g);

	tm1637DisplayDigit(0, 0);
	tm1637DisplayDigit(1, 1);
	tm1637DisplayDigit(2, 2);
	tm1637DisplayDigit(3, 3);
	tm1637SetBrightness(8);
}

void tm1637DisplayChar(int ch, uint8_t column) {
	//coppy char to memory
	if (column == 1)
		_dig1 = ch;

	_tm1637Start();
	_tm1637WriteByte(WriteDataToDispReg);
	_tm1637ReadResult();
	_tm1637Stop();

	_tm1637Start();
	_tm1637WriteByte(AdressCmd + column);
	_tm1637ReadResult();

	_tm1637WriteByte(ch);
	_tm1637ReadResult();

	_tm1637Stop();

}

void tm1637DisplayDigit(int ch, uint8_t column) {

	tm1637DisplayChar(segmentMap[ch], column);
}
void tm1637ShowColon(uint8_t show) {

	_tm1637Start();
	_tm1637WriteByte(WriteDataToDispReg);
	_tm1637ReadResult();
	_tm1637Stop();

	_tm1637Start();
	_tm1637WriteByte(AdressCmd + 1);
	_tm1637ReadResult();

	if (show) //enable colon +128 to value
		_dig1 += 128;
	_tm1637WriteByte(_dig1);

	_tm1637ReadResult();
	_tm1637Stop();
}

void tm1637DisplayDecimal(int v, int displaySeparator) {
	unsigned char digitArr[4];
	for (int i = 0; i < 4; ++i) {
		digitArr[i] = segmentMap[v % 10];
		if (i == 2 && displaySeparator) {
			digitArr[i] |= 1 << 7;
		}
		v /= 10;
	}

	_tm1637Start();
	_tm1637WriteByte(WriteDataToDispReg);
	_tm1637ReadResult();
	_tm1637Stop();

	_tm1637Start();
	_tm1637WriteByte(AdressCmd);
	_tm1637ReadResult();

	for (int i = 0; i < 4; ++i) {
		_tm1637WriteByte(digitArr[3 - i]);
		_tm1637ReadResult();
	}

	_tm1637Stop();
}

// Valid brightness values: 0 - 8.
// 0 = display off.
void tm1637SetBrightness(char brightness) {
	// Brightness command:
	// 1000 0XXX = display off
	// 1000 1BBB = display on, brightness 0-7
	// X = don't care
	// B = brightness
	_tm1637Start();
	_tm1637WriteByte(DisplCntrl + brightness);
	_tm1637ReadResult();
	_tm1637Stop();
}

void _tm1637Start(void) {
	_tm1637ClkHigh
	_tm1637DioHigh
	Delay(5);
	_tm1637DioLow
}

void _tm1637Stop(void) {
	_tm1637ClkLow
	Delay(10);
	_tm1637DioLow
	Delay(10);
	_tm1637ClkHigh
	Delay(10);
	_tm1637DioHigh
}

void _tm1637ReadResult(void) {
	_tm1637ClkLow
	Delay(10);
	// while (dio); // We're cheating here and not actually reading back the response.
	_tm1637ClkHigh
	Delay(10);
	_tm1637ClkLow
}

void _tm1637WriteByte(unsigned char b) {
	for (int i = 0; i < 8; ++i) {
		_tm1637ClkLow
		if (b & 0x01) {
			_tm1637DioHigh
		} else {
			_tm1637DioLow
		}
		Delay(10);
		b >>= 1;
		_tm1637ClkHigh
		Delay(10);
	}
}

uint32_t tm1637_TM_DELAY_Init(void) {
#if !defined(STM32F0xx)
	uint32_t c;

	/* Enable TRC */
	CoreDebug->DEMCR &= ~0x01000000;
	CoreDebug->DEMCR |= 0x01000000;

	/* Enable counter */
	DWT->CTRL &= ~0x00000001;
	DWT->CTRL |= 0x00000001;

	/* Reset counter */
	DWT->CYCCNT = 0;

	/* Check if DWT has started */
	c = DWT->CYCCNT;

	/* 2 dummys */
	__ASM volatile ("NOP");
	__ASM volatile ("NOP");

	/* Return difference, if result is zero, DWT has not started */
	return (DWT->CYCCNT - c);
#else
	/* Return OK */
	return 1;
#endif
}
