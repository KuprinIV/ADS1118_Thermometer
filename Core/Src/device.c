/*
 * device.c
 *
 *  Created on: Apr 14, 2025
 *      Author: KUPRIN_IV
 */
#include "device.h"

Data dev_state = {0, 0, 0};

/**
 * @brief Device power control
 * @param is_on: 0 - power off, 1 - power on
 * @retval None
 */
void DEV_PowerCtrl(uint8_t is_on)
{
	if(is_on)
	{
		PWRON_GPIO_Port->BSRR = PWRON_Pin;
	}
	else
	{
		PWRON_GPIO_Port->BRR = PWRON_Pin;
	}
}

/**
 * @brief Scan device button
 * @param bs - button state pointer
 * @retval 0 - button state isn't updated, 1 - button state is updated
 */
uint8_t DEV_ScanBtn(ButtonState* bs)
{
	static uint8_t btn_state_prev;
	static uint16_t long_btn_press_cntr;
	static uint8_t is_long_btn_press_detected;
	uint8_t is_state_updated = 0;

	if(!btn_state_prev && !(BTN_GPIO_Port->IDR & BTN_Pin)) // button is pressed
	{
		btn_state_prev = 1;
		*bs = Pressed;
		is_state_updated = 1;
	}
	else if(btn_state_prev && !(BTN_GPIO_Port->IDR & BTN_Pin)) // button is holding on
	{
		if(long_btn_press_cntr < LONG_PRESS_TICKS)
		{
			long_btn_press_cntr++;
		}
		else if(!is_long_btn_press_detected)
		{
			is_long_btn_press_detected = 1;
			*bs = LongPressed;
			is_state_updated = 1;
		}
	}
	else if(btn_state_prev && (BTN_GPIO_Port->IDR & BTN_Pin)) // button is released
	{
		btn_state_prev = 0;
		long_btn_press_cntr = 0; // reset long button press counter
		is_long_btn_press_detected = 0; // reset long button press detection flag
		*bs = NotPressed;
		is_state_updated = 1;
	}

	return is_state_updated;
}

/**
 * @brief Get battery charging state
 * @param None
 * @retval NotCharge - battery isn't charging, Charging - battery is charging
 */
ChargeState DEV_GetChargeState(void)
{
	ChargeState res = NotCharge;
	// TP0456 IC CHG pin is active low (OD)
	if(!(CHG_RDY_GPIO_Port->IDR & CHG_RDY_Pin))
	{
		res = Charging;
	}
	return res;
}
