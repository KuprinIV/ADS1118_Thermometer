/*
 * device.c
 *
 *  Created on: Apr 14, 2025
 *      Author: KUPRIN_IV
 */
#include "device.h"

Data dev_state = {0, 0, 0, 0};
uint16_t bat_discharge_curve[2][11] = {{10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{4200, 4050, 4010, 3980, 3920, 3850, 3800, 3760, 3660, 3550, 3460}};

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

/**
 * @brief Get battery charge from 0 (empty) to 10 (full) range
 * @param bat_mv - battery voltage in mV
 * @retval battery charge from 0 (empty) to 10 (full) range
 */
uint8_t DEV_GetBatteryCharge(uint32_t bat_mv)
{
	uint8_t index_val = 0;
	// get nearest voltage value index from battery discharge curve
	for(uint8_t i = 1; i < 11; i++)
	{
		if(bat_mv >= bat_discharge_curve[1][i] && bat_mv < bat_discharge_curve[1][i-1])
		{
			index_val = i-1;
			break;
		}
		else if(bat_mv < bat_discharge_curve[1][10])
		{
			index_val = 10;
			break;
		}
		else if(bat_mv >= bat_discharge_curve[1][0])
		{
			index_val = 0;
			break;
		}
	}
	return (uint8_t)bat_discharge_curve[0][index_val];
}

/**
 * @brief Round temperature value from 0,125 °C steps to °C
 * @param temp_0t125 - temperature value in 0,125 °C steps
 * @retval rounded temperature value in °C
 */
int16_t DEV_RoundTemperatureValue(int16_t temp_0t125)
{
	int16_t res = 0;
	int16_t frac = 0;
	static int16_t samples_to_avg[NUM_AVGS];
	static uint8_t index;
	static uint8_t delay_cntr;

	samples_to_avg[index++] = temp_0t125;
	if(index == NUM_AVGS)
	{
		index = 0;
	}

	// calculate average value
	for(uint8_t i = 0; i < NUM_AVGS; i++)
	{
		res += samples_to_avg[i];
	}
	res /= NUM_AVGS;

	// add filter output delay to wait its stabilization
	if(delay_cntr < 10)
	{
		res = temp_0t125;
		delay_cntr++;
	}

	frac = (res & 0x07);
	res >>= 3;

	if(frac >= 0x04)
	{
		res++;
	}
	return res;
}
