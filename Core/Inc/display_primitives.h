/*
 * display_primitives.h
 *
 *  Created on: 18 февр. 2025 г.
 *      Author: Ilya
 */

#ifndef INC_DISPLAY_PRIMITIVES_H_
#define INC_DISPLAY_PRIMITIVES_H_

#include "main.h"
#include "fonts.h"
#include "device.h"

#define LCDWIDTH 					128
#define LCDHEIGHT 					64


typedef enum {AlignLeft, AlignCenter, AlignRight} Align;
typedef enum {Inverted, NotInverted} IsInverted;
typedef enum {Prev, Next, NoAction} Action;

typedef struct
{
	uint8_t x_pos;
    uint8_t y_pos;
    Align align;
    FontInfo font;
    char Text[15];
    IsInverted inverted;
}String,*pString;

typedef struct Window
{
  String strings[3];
  uint8_t StringsQuantity;
  int (*callback)(struct Window* ,pData , Action ,Action );
  struct Window* next;
  struct Window* prev;
  struct Window* top;
  struct Window* bottom;
}Window,*pWindow;


void DP_ClearBuffer(void);
void DP_UpdateBuffer(void);
void DP_SetStringInBuffer(String* string);
void DP_SetWindow(pWindow wnd);
void DP_InvertRegion(uint8_t xn, uint8_t yn, uint8_t xk, uint8_t yk);
void DP_DrawLine(int8_t xn, int8_t yn, int8_t xk, int8_t yk);
void DP_DrawCircle(uint8_t x, uint8_t y, uint8_t R);
void DP_DrawEllipse(uint8_t x_pos, uint8_t y_pos, uint8_t rad_x, uint8_t rad_y);
void DP_DrawRect(uint8_t x_pos, uint8_t y_pos,uint8_t width, uint8_t height);
void DP_FillRect(uint8_t x_pos, uint8_t y_pos,uint8_t width, uint8_t height);
void DP_FillCircle(uint8_t Xpos, uint8_t Ypos, uint8_t Radius);
void DP_FillEllipse(uint8_t Xpos, uint8_t Ypos, uint8_t XRadius, uint8_t YRadius);
void DP_PaintBatteryIndicator(uint8_t percentage);

#endif /* INC_DISPLAY_PRIMITIVES_H_ */
