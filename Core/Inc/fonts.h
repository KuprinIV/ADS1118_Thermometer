/*
 * fonts.h
 *
 *  Created on: 5 сент. 2023 г.
 *      Author: Kuprin_IV
 */

#ifndef INC_FONTS_H_
#define INC_FONTS_H_

typedef struct
{
    unsigned char width;
    unsigned short offset;
}charDescriptor;

typedef struct
{
    const char* pFont;
    unsigned char Height;
    const charDescriptor* descriptor;
}FontInfo;

extern FontInfo font6x8;
extern FontInfo MSSanSerif_20;

#endif /* INC_FONTS_H_ */
