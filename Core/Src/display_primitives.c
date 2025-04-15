/*
 * display_primitives.c
 *
 *  Created on: 18 февр. 2025 г.
 *      Author: Ilya
 */

#include "display_primitives.h"
#include "ssd1315.h"
#include <string.h>

#define ABS(x) (x) >= 0 ? (x):(-(x))

static void DP_DrawPixel(uint8_t x, uint8_t y);
static void DP_DrawBitmap(uint8_t* bmp, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

uint8_t lcd_framebuffer[LCDWIDTH*LCDHEIGHT/8] = {0};

/**
  * @brief  Clear framebuffer data
  * @param  none
  * @retval none
  */
void DP_ClearBuffer(void)
{
	memset(lcd_framebuffer, 0, sizeof(lcd_framebuffer));
}

/**
  * @brief  Write framebuffer data to display memory
  * @param  none
  * @retval none
  */
void DP_UpdateBuffer(void)
{
	// write data
	SSD1315_UpdateFramebuffer(lcd_framebuffer, sizeof(lcd_framebuffer));
}

/**
  * @brief  Write string data to framebuffer
  * @param  string - data structure with string parameters
  * @retval none
  */
void DP_SetStringInBuffer(String* string)
{
	uint8_t x = string->x_pos, y = string->y_pos, x_inv = 0;
	const unsigned char* pStr = (const unsigned char*)string->Text;
	uint8_t currentCharWidth = 0;
	int Length = strlen((char*)string->Text), LengthInv = Length, widthInPixels = 0;
	//calculate width of text in pixels
	const unsigned char* lStr = (const unsigned char*)string->Text;
	while(LengthInv-- > 0)
	{
	   widthInPixels += string->font.descriptor[*(lStr)-' '].width + 1;
	   lStr++;
	}

	if(string->align == AlignCenter) x = (LCDWIDTH - widthInPixels + x)/2;
	if(string->align == AlignRight) x = LCDWIDTH - 1 - widthInPixels - x;
	x_inv = x;

	while(Length-- > 0)
	{
		currentCharWidth = string->font.descriptor[*(pStr)-' '].width;
		DP_DrawBitmap((uint8_t*)(string->font.pFont+string->font.descriptor[*(pStr)-' '].offset), x, y, currentCharWidth, string->font.Height);
		x += currentCharWidth+1;
		pStr++;
	}

	if(string->inverted == Inverted)
	{
		DP_InvertRegion(x_inv-1, y-1, x_inv+widthInPixels + 1, y+string->font.Height);
	}
}

/**
  * @brief  Write window data to framebuffer
  * @param  wnd - data structure with window parameters
  * @retval none
  */
void DP_SetWindow(pWindow wnd)
{
	int i;

    for(i = 0; i < wnd->StringsQuantity; i++)
    {
    	DP_SetStringInBuffer(&wnd->strings[i]);
    }
}

/**
  * @brief  Invert display region data in framebuffer
  * @param  xn - x-coordinate of top-left corner
  * @param  yn - y-coordinate of top-left corner
  * @param  xk - x-coordinate of bottom-right corner
  * @param  yk - y-coordinate of bottom-right corner
  * @retval none
  */
void DP_InvertRegion(uint8_t xn, uint8_t yn, uint8_t xk, uint8_t yk)
{
	uint8_t mask = 0;
	uint8_t i, j;

	// transform y-coordinates
	yn = LCDHEIGHT - 1 - yn;
	yk = LCDHEIGHT - 1 - yk;

	for(j = yk/8; j <= yn/8; j++)
	{
		for(i = xn; i < xk;i++)
		{
			if(j == yn/8)
			{
			  mask = 0xFF>>(7-(yn%8));
			}
			else
			{
			  if(j == yk/8)
			  {
				  mask = 0xFF<<(yk%8);
			  }
			  else
			  {
			  	mask = 0xFF;
			  }
			}
			lcd_framebuffer[LCDWIDTH*j + i] ^= mask;
		}
	}
}

/**
  * @brief  Write line data in framebuffer
  * @param  xn - x-coordinate of line start
  * @param  yn - y-coordinate of line start
  * @param  xk - x-coordinate of line end
  * @param  yk - y-coordinate of line end
  * @retval none
  */
void DP_DrawLine(int8_t xn, int8_t yn, int8_t xk, int8_t yk)
{
	int8_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
	curpixel = 0;

	deltax = ABS(yk-yn);        /* The difference between the x's */
	deltay = ABS(xk-xn);        /* The difference between the y's */
	x = xn;                       /* Start x off at the first pixel */
	y = yn;                       /* Start y off at the first pixel */

	if (xk >= xn)                 /* The x-values are increasing */
	{
		xinc1 = 1;
		xinc2 = 1;
	}
	else                          /* The x-values are decreasing */
	{
		xinc1 = -1;
		xinc2 = -1;
	}

	if (yk >= yn)                 /* The y-values are increasing */
	{
		yinc1 = 1;
		yinc2 = 1;
	}
	else                          /* The y-values are decreasing */
	{
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay)         /* There is at least one x-value for every y-value */
	{
		xinc2 = 0;                  /* Don't change the x when numerator >= denominator */
		yinc1 = 0;                  /* Don't change the y for every iteration */
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;         /* There are more x-values than y-values */
	}
	else                          /* There is at least one y-value for every x-value */
	{
		xinc1 = 0;                  /* Don't change the x for every iteration */
		yinc2 = 0;                  /* Don't change the y when numerator >= denominator */
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;         /* There are more y-values than x-values */
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
		DP_DrawPixel(x,y); /* Draw the current pixel */
		num += numadd;                            /* Increase the numerator by the top of the fraction */
		if (num >= den)                           /* Check if numerator >= denominator */
		{
			num -= den;                             /* Calculate the new numerator value */
			x += xinc1;                             /* Change the x as appropriate */
			y += yinc1;                             /* Change the y as appropriate */
		}
		x += xinc2;                               /* Change the x as appropriate */
		y += yinc2;                               /* Change the y as appropriate */
	}
}

/**
  * @brief  Write circle data in framebuffer
  * @param  x - x-coordinate of circle center
  * @param  y - y-coordinate of circle center
  * @param  R - circle radius in pixels
  * @retval none
  */
void DP_DrawCircle(uint8_t x, uint8_t y, uint8_t R)
{
	int8_t  decision;       /* Decision Variable */
	uint8_t  curx;   /* Current X Value */
	uint8_t  cury;   /* Current Y Value */

	decision = 3 - (R << 1);
	curx = 0;
	cury = R;

	while (curx <= cury)
	{
		DP_DrawPixel((x - cury),(y + curx));
		DP_DrawPixel((x - cury),(y - curx));
		DP_DrawPixel((x - curx),(y + cury));
		DP_DrawPixel((x - curx),(y - cury));
		DP_DrawPixel((x + cury),(y + curx));
		DP_DrawPixel((x + cury),(y - curx));
		DP_DrawPixel((x + curx),(y + cury));
		DP_DrawPixel((x + curx),(y - cury));

		if (decision < 0)
		{
			decision += (curx << 2) + 6;
		}
		else
		{
			decision += ((curx - cury) << 2) + 10;
			cury--;
		}
		curx++;
	}
}

/**
  * @brief  Write ellipse data in framebuffer
  * @param  x_pos - x-coordinate of ellipse center
  * @param  y_pos - y-coordinate of ellipse center
  * @param  rad_x - ellipse radius by x-coordinate in pixels
  * @param  rad_y - ellipse radius by y-coordinate in pixels
  * @retval none
  */
void DP_DrawEllipse(uint8_t x_pos, uint8_t y_pos, uint8_t rad_x, uint8_t rad_y)
{
    char x = 0, y = -rad_x, err = 2-2*rad_y, e2;
    float k = 0, rad1 = 0, rad2 = 0;

    rad1 = rad_y;
    rad2 = rad_x;

    k = (float)(rad2/rad1);

    do
    {
    	DP_DrawPixel((x_pos+y),(y_pos -(uint8_t)(x/k)));
    	DP_DrawPixel((x_pos+y),(y_pos +(uint8_t)(x/k)));
    	DP_DrawPixel((x_pos-y), (y_pos +(uint8_t)(x/k)));
    	DP_DrawPixel((x_pos-y), (y_pos -(uint8_t)(x/k)));

        e2 = err;
        if (e2 <= x) {
            err += ++x*2+1;
            if (-y == x && e2 <= y) e2 = 0;
        }
        if (e2 > y) err += ++y*2+1;
    }
    while (y <= 0);
}

/**
  * @brief  Write rectangular data in framebuffer
  * @param  x_pos - x-coordinate of top-left corner
  * @param  y_pos - y-coordinate of top-left corner
  * @param  width - rectangular width in pixels
  * @param  height - rectangular height in pixels
  * @retval none
  */
void DP_DrawRect(uint8_t x_pos, uint8_t y_pos,uint8_t width, uint8_t height)
{
	DP_DrawLine(x_pos,y_pos,x_pos+width,y_pos);
	DP_DrawLine(x_pos+width,y_pos,x_pos+width,y_pos+height);
	DP_DrawLine(x_pos,y_pos+height,x_pos+width,y_pos+height);
	DP_DrawLine(x_pos,y_pos,x_pos,y_pos+height);
}

/**
  * @brief  Write filled rectangular data in framebuffer
  * @param  x_pos - x-coordinate of top-left corner
  * @param  y_pos - y-coordinate of top-left corner
  * @param  width - rectangular width in pixels
  * @param  height - rectangular height in pixels
  * @retval none
  */
void DP_FillRect(uint8_t x_pos, uint8_t y_pos,uint8_t width, uint8_t height)
{
    for(;height>0;height--)
    {
    	DP_DrawLine(x_pos,y_pos,x_pos+width-1,y_pos);
    	y_pos++;
    }
}

/**
  * @brief  Write filled circle data in framebuffer
  * @param  Xpos - x-coordinate of circle center
  * @param  Ypos - y-coordinate of circle center
  * @param  Radius - circle radius in pixels
  * @retval none
  */
void DP_FillCircle(uint8_t Xpos, uint8_t Ypos, uint8_t Radius)
{
	int8_t  decision;        /* Decision Variable */
	uint8_t  curx;    /* Current X Value */
	uint8_t  cury;    /* Current Y Value */

	decision = 3 - (Radius << 1);

	curx = 0;
	cury = Radius;

	while (curx <= cury)
	{
		if(cury > 0)
		{
			DP_DrawLine(Xpos + curx, Ypos - cury,Xpos + curx, Ypos + cury);
			DP_DrawLine(Xpos - curx, Ypos - cury,Xpos - curx, Ypos + cury);
		}

		if(curx > 0)
		{
			DP_DrawLine(Xpos - cury, Ypos - curx,Xpos - cury, Ypos + curx);
			DP_DrawLine(Xpos + cury, Ypos - curx,Xpos + cury, Ypos + curx);
		}
		if (decision < 0)
		{
			decision += (curx << 2) + 6;
		}
		else
		{
			decision += ((curx - cury) << 2) + 10;
			cury--;
		}
		curx++;
	}
	DP_DrawCircle(Xpos, Ypos, Radius);
}

/**
  * @brief  Write filled ellipse data in framebuffer
  * @param  Xpos - x-coordinate of ellipse center
  * @param  Ypos - y-coordinate of ellipse center
  * @param  XRadius - ellipse radius by x-coordinate in pixels
  * @param  YRadius - ellipse radius by y-coordinate in pixels
  * @retval none
  */
void DP_FillEllipse(uint8_t Xpos, uint8_t Ypos, uint8_t XRadius, uint8_t YRadius)
{
	char x = 0, y = -XRadius, err = 2-2*YRadius, e2;
	float k = 0, rad1 = 0, rad2 = 0;

	rad1 = YRadius;
	rad2 = XRadius;

	k = (float)(rad2/rad1);

	do
	{
		DP_DrawLine((Xpos+y), (Ypos-(uint8_t)(x/k)),(Xpos+y), (Ypos + (uint8_t)(x/k) + 1));
		DP_DrawLine((Xpos-y), (Ypos-(uint8_t)(x/k)),(Xpos-y), (Ypos + (uint8_t)(x/k) + 1));

		e2 = err;
		if (e2 <= x)
		{
			err += ++x*2+1;
			if (-y == x && e2 <= y) e2 = 0;
		}
		if (e2 > y) err += ++y*2+1;
	}
	while (y <= 0);
}

/**
  * @brief  Put battery indicator image into framebuffer
  * @param  percentage - battery charge state from 0 (empty) to 10 (full)
  * @retval None
  */
void PaintBatteryIndicator(uint8_t percentage)
{
	uint8_t BatteryBorder[12] = {0x3C,0x66,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x7E};
	// fill indicator by percentage
	for(uint8_t i = 0; i < percentage; i++)
	{
		if(i < 9)
		{
			BatteryBorder[10-i] |= 0x3C;
		}
		else
		{
			BatteryBorder[10-i] |= 0x18;
		}
	}
	DP_DrawBitmap(BatteryBorder, LCDWIDTH-20, 0, 12, 6);
}

/**
  * @brief  Write pixel in framebuffer
  * @param  x - x-coordinate of pixel
  * @param  y - y-coordinate of pixel
  * @retval none
  */
static void DP_DrawPixel(uint8_t x, uint8_t y)
{
	// transform y-coordinate
	y = LCDHEIGHT - 1 - y;

	lcd_framebuffer[LCDWIDTH*(y>>3)+x] |= 1<<(y%8);
}

/**
  * @brief  Write bitmap data to framebuffer
  * @param  bmp - bitmap data array
  * @param  x - x-coordinate of top-left corner
  * @param  y - y-coordinate of top-left corner
  * @param  width - bitmap width in pixels
  * @param  height - bitmap height in pixels
  * @retval none
  */
static void DP_DrawBitmap(uint8_t* bmp, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    uint8_t bytes_in_col = ((height%8) == 0)?(height>>3):((height>>3) + 1);

    if(x + width > LCDWIDTH - 1) x = LCDWIDTH - 1 - width;
    if(y + height > LCDHEIGHT - 1) y = LCDHEIGHT - 1 - height;

    for(uint8_t i = 0; i < width; i++)
    {
    	for(uint8_t j = 0; j < bytes_in_col; j++)
    	{
    		for(uint8_t k = 0; k < 8; k++)
    		{
				if((bmp[j*width+i]>>k) & 0x01)
					DP_DrawPixel(x+i, y+j*8+k);
    		}
    	}
    }
}
