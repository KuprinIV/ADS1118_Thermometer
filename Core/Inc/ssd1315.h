/*
 * ssd1315.h
 *
 *  Created on: 16 февр. 2025 г.
 *      Author: Ilya
 */

#ifndef INC_SSD1315_H_
#define INC_SSD1315_H_

#include <stdint.h>

#define DEVICE_ADDR						0x78

// define commands list
#define CMD_SET_MEM_ADDRESSING_MODE		0x20
#define CMD_SET_COL_ADDRESS				0x21
#define CMD_SET_PAGE_ADDRESS			0x22
#define CMD_SET_CONTRAST_CTRL			0x81
#define CMD_SET_SEG_REMAP				0xA0
#define CMD_ENTIRE_DISPLAY_ON			0xA5
#define CMD_NORMAL_DISPLAY				0xA6
#define CMD_MUX_RATIO					0xA8
#define CMD_SET_IREF					0xAD
#define CMD_SET_DISPLAY_OFF				0xAE
#define CMD_SET_DISPLAY_ON				0xAF
#define CMD_SET_DISPLAY_OFFSET			0xD3
#define CMD_SET_DISPLAY_CLKDIV			0xD5
#define CMD_SET_PRECHG_PERIOD			0xD9
#define CMD_SET_COM_PINS_HWCFG			0xDA
#define CMD_SET_VCOMH_LVL				0xDB
#define CMD_NOP							0xE3
#define CMD_CHG_PUMP_SETTING			0x8D
#define CMD_HOR_SCROLL_SETUP			0x26
#define CMD_VER_SCROLL_SETUP			0x29
#define CMD_DEACTIVATE_SCROLL			0x2E
#define CMD_ACTIVATE_SCROLL				0x2F
#define CMD_SET_VER_SCROLL_AREA			0xA3
#define CMD_CONTENT_SCROLL_SETUP		0x2C
#define CMD_SET_FADE_OUT_BLINK			0x23
#define CMD_SET_ZOOM_IN					0xD6

void SSD1315_Init(void);
void SSD1315_UpdateFramebuffer(uint8_t* lcd_framebuffer, uint16_t length);
void SSD1315_DisplayOnOff(uint8_t is_on);


#endif /* INC_SSD1315_H_ */
