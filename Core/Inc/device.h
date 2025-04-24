/*
 * device.h
 *
 *  Created on: Apr 14, 2025
 *      Author: KUPRIN_IV
 */

#ifndef INC_DEVICE_H_
#define INC_DEVICE_H_

#include "main.h"

#define LONG_PRESS_TICKS		(1000/SCAN_PERIOD_MS)
#define VBAT_LOW_MV				3450
#define VBAT_FULL_MV			4200
//#define IS_SHOW_ROOM_TEMP		1

typedef enum {NotPressed = 0, Pressed = 1, LongPressed = 2} ButtonState;
typedef enum {NotCharge = 0, Charging = 1} ChargeState;

typedef struct
{
	int16_t thermocouple_temp;
	int16_t room_temp;
	uint32_t vbat_mv;
	uint8_t is_thermocouple_connected;
} Data, *pData;

void DEV_PowerCtrl(uint8_t is_on);
uint8_t DEV_ScanBtn(ButtonState* bs);
ChargeState DEV_GetChargeState(void);
uint8_t DEV_GetBatteryCharge(uint32_t bat_mv);


#endif /* INC_DEVICE_H_ */
