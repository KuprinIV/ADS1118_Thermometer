/*
 * ads1118.c
 *
 *  Created on: Apr 14, 2025
 *      Author: KUPRIN_IV
 */

#include "ads1118.h"
#include <string.h>

static void ADS1118_TransmitData(uint16_t conf_reg);
static void ADS1118_TransmitReceiveData(uint16_t conf_reg, uint32_t* data);
static int16_t ADS1118_ConvertTSensorData(int16_t raw_data);
static int16_t ADS1118_ConvertThermocoupleData(int16_t raw_data);

extern SPI_HandleTypeDef hspi1;
extern Data dev_state;
static union ADS1118_ConfigReg ads1118_conf;
static uint8_t adc_mode = 0; // 0 - ADC mode, 1 - temperature sensor mode
static uint32_t ads1118_read_value = 0;

/**
  * @brief  Initialize ADS1118 ADC and start conversion
  * @param  none
  * @retval none
  */
void ADS1118_Init(void)
{
	// set ADS1118 configuration
	ads1118_conf.config.mux = MUX_AINP_AIN0_AINN_AIN1;
	ads1118_conf.config.pga = PGA_FS_6_144V;
	ads1118_conf.config.mode = 0; // power-down single-shot mode
	ads1118_conf.config.data_rate = DR_128SPS;
	ads1118_conf.config.ts_mode = adc_mode;
	ads1118_conf.config.pull_up_en = 1; // enable DOUT pin pull-up resistor
	ads1118_conf.config.nop = NOP_UPD_CONF_REG;
	ads1118_conf.config.op_status = 1; // begin a single conversion

	ADS1118_TransmitData(ads1118_conf.reg_value);
}

/**
  * @brief  Write configuration register data to ADS1118 by SPI
  * @param  conf_reg - configuration register value
  * @retval none
  */
static void ADS1118_TransmitData(uint16_t conf_reg)
{
	uint8_t reg_data_write[4] = {0};

	// fill transmit data
	reg_data_write[0] = ((conf_reg>>8)  & 0xFF);
	reg_data_write[1] = (conf_reg & 0xFF);
	reg_data_write[2] = reg_data_write[0];
	reg_data_write[3] = reg_data_write[1];

	// send data to IC by SPI
	HAL_SPI_Transmit_IT(&hspi1, reg_data_write, 4);
}

/**
  * @brief  Write configuration register data and read previous conversion data from ADS1118 by SPI
  * @param  conf_reg - configuration register value
  * @param  data - read data value pointer (upper 16 bit are conversion result, lower 16 bit - configuration register value)
  * @retval none
  */
static void ADS1118_TransmitReceiveData(uint16_t conf_reg, uint32_t* data)
{
	uint8_t reg_data_write[4] = {0};

	// fill transmit data
	reg_data_write[0] = ((conf_reg>>8)  & 0xFF);
	reg_data_write[1] = (conf_reg & 0xFF);
	reg_data_write[2] = reg_data_write[0];
	reg_data_write[3] = reg_data_write[1];

	HAL_SPI_TransmitReceive_IT(&hspi1, reg_data_write, (uint8_t*)&ads1118_read_value, 4);
}

/**
  * @brief  Convert temperature sensor data of ADS1118 to °C
  * @param  raw_data - conversion result from ADS1118
  * @retval temperature value in °C
  */
static int16_t ADS1118_ConvertTSensorData(int16_t raw_data)
{
	int16_t temp_sensor = (raw_data>>7); // 14-bit value with 1/32 °C step
	return temp_sensor;
}

/**
  * @brief  Convert thermocouple voltage data of ADS1118 to °C
  * @param  raw_data - conversion result from ADS1118
  * @retval temperature value in °C
  */
static int16_t ADS1118_ConvertThermocoupleData(int16_t raw_data)
{
	return raw_data; // TODO: need to convert to °C
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	uint16_t data_val = 0;
	union ADS1118_ConfigReg conf_reg;

	if(hspi->Instance == SPI1)
	{
		// assume that ADS1118 read value is updated
		data_val = (uint16_t)((ads1118_read_value>>16) & 0xFFFF);
		conf_reg.reg_value = (uint16_t)(ads1118_read_value & 0xFFFF);

		// check NOP bits
		if(conf_reg.config.nop == NOP_UPD_CONF_REG) // data is valid
		{
			if(adc_mode == 0) // we've read ADC thermocouple voltage
			{
				dev_state.thermocouple_temp = ADS1118_ConvertThermocoupleData(data_val);
			}
			else // we've read temperature sensor data
			{
				dev_state.room_temp = ADS1118_ConvertTSensorData((int16_t)data_val);
			}
		}

		// start new conversion
		adc_mode ^= 0x01; // toggle ADC mode
		ads1118_conf.config.ts_mode = adc_mode; // update TS mode in config register
		ADS1118_TransmitReceiveData(ads1118_conf.reg_value, &ads1118_read_value);
	}
}


