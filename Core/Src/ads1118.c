/*
 * ads1118.c
 *
 *  Created on: Apr 14, 2025
 *      Author: KUPRIN_IV
 */

#include "ads1118.h"
#include <string.h>

static uint8_t ADS1118_TransmitReceiveData(uint16_t conf_reg, int16_t* data);
static int16_t ADS1118_ConvertTSensorData(int16_t raw_data);
static int16_t ADS1118_ConvertThermocoupleData(int16_t raw_data);

extern Data dev_state;
static union ADS1118_ConfigReg ads1118_conf;
static uint8_t adc_mode = 0; // 0 - ADC mode, 1 - temperature sensor mode
//static int16_t full_scale_pga_mv[8] = {6144, 4096, 2048, 1024, 512, 256, 256, 256};

/**
  * @brief  Initialize ADS1118 ADC and start conversion
  * @param  none
  * @retval none
  */
void ADS1118_Init(void)
{
	int16_t temp_data = 0;
	// set ADS1118 configuration
	ads1118_conf.config.mux = MUX_AINP_AIN0_AINN_AIN1;
	ads1118_conf.config.pga = PGA_FS_0_256V;
	ads1118_conf.config.mode = 1; // power-down single-shot mode
	ads1118_conf.config.data_rate = DR_128SPS;
	ads1118_conf.config.ts_mode = adc_mode;
	ads1118_conf.config.pull_up_en = 1; // enable DOUT pin pull-up resistor
	ads1118_conf.config.nop = NOP_UPD_CONF_REG;
	ads1118_conf.config.op_status = 1; // begin a single conversion

	// start first conversion
	ADS1118_TransmitReceiveData(ads1118_conf.reg_value, &temp_data); // temp_data value is ignored
}

/**
  * @brief  Write configuration register data and read previous conversion data from ADS1118 by SPI
  * @param  conf_reg - configuration register value
  * @param  data - read data value pointer (upper 16 bit are conversion result, lower 16 bit - configuration register value)
  * @retval 0 - data isn't ready, 1 - data is ready
  */
static uint8_t ADS1118_TransmitReceiveData(uint16_t conf_reg, int16_t* data)
{
	uint8_t reg_data_write[4] = {0};
	uint8_t reg_data_read[4] = {0};
	union ADS1118_ConfigReg ads_conf_reg;
	uint8_t is_data_ready = 0;

	// fill transmit data
	reg_data_write[0] = ((conf_reg>>8)  & 0xFF);
	reg_data_write[1] = (conf_reg & 0xFF);
	reg_data_write[2] = 0xFF;
	reg_data_write[3] = 0xFF;

	// SPI data transfer
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4);
	for(uint8_t i = 0; i < 4; i++)
	{
		while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {}
		LL_SPI_TransmitData8(SPI1, reg_data_write[i]);

		while(LL_SPI_IsActiveFlag_BSY(SPI1)) {}

		while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}
		reg_data_read[i] = LL_SPI_ReceiveData8(SPI1);
	}
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_4);

	// check is configuration register is written correctly
	ads_conf_reg.reg_value = (uint16_t)((reg_data_read[2]<<8)|reg_data_read[3]);

	if(ads_conf_reg.config.nop == NOP_UPD_CONF_REG /*&& ads_conf_reg.config.cnv_rdy_flag == 0*/) // data is valid and ready
	{
		*data = (int16_t)((reg_data_read[0]<<8)|reg_data_read[1]);
		is_data_ready = 1;
	}

	return is_data_ready;
}

/**
  * @brief  Convert temperature sensor data of ADS1118 to °C
  * @param  raw_data - conversion result from ADS1118
  * @retval temperature value in 0,125 °C steps
  */
static int16_t ADS1118_ConvertTSensorData(int16_t raw_data)
{
	int32_t temp = (int32_t)(raw_data>>2)*ROOM_TEMP_A_COEFF + ROOM_TEMP_B_COEFF;
	temp = (temp>>13) + ROOM_TEMP_OFFSET_COEFF;
	return (int16_t)temp;
//	return raw_data;
}

/**
  * @brief  Convert thermocouple voltage data of ADS1118 to °C
  * @param  raw_data - conversion result from ADS1118
  * @retval temperature value in 0,125 °C steps relative to ambient
  */
static int16_t ADS1118_ConvertThermocoupleData(int16_t raw_data)
{
	int32_t temp = (int32_t)raw_data*THERMOCOUPLE_COEFF;
	temp >>= 13;
	return (int16_t)temp;
//	return raw_data;
}

/**
  * @brief  Read data from ADS1118
  * @param  data - data structure pointer with device parameters
  * @retval None
  */
void ADS1118_ReadData(pData data)
{
	int16_t data_val = 0;
	uint8_t is_data_ready = 0;

	adc_mode ^= 0x01; // toggle ADC mode
	ads1118_conf.config.ts_mode = adc_mode; // update TS mode in config register

	// read data of previous conversion from ADS1118 and write config register
	is_data_ready = ADS1118_TransmitReceiveData(ads1118_conf.reg_value, &data_val);

	if(is_data_ready)
	{
		// set data type
		if(adc_mode) // we've read ADC thermocouple voltage
		{
			// check is thermocouple connected
			if(data_val > 10000)
			{
				dev_state.is_thermocouple_connected = 0;
			}
			else
			{
				dev_state.is_thermocouple_connected = 1;
			}
			dev_state.thermocouple_temp = ADS1118_ConvertThermocoupleData(data_val);
		}
		else // we've read temperature sensor data
		{
			dev_state.room_temp = ADS1118_ConvertTSensorData(data_val);
		}
	}
}
