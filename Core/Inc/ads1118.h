/*
 * ads1118.h
 *
 *  Created on: Apr 14, 2025
 *      Author: KUPRIN_IV
 */

#ifndef INC_ADS1118_H_
#define INC_ADS1118_H_

#include "main.h"
#include "device.h"

typedef struct
{
	unsigned cnv_rdy_flag : 1;
	unsigned nop : 2;
	unsigned pull_up_en : 1;
	unsigned ts_mode : 1;
	unsigned data_rate : 3;
	unsigned mode : 1;
	unsigned pga : 3;
	unsigned mux : 3;
	unsigned op_status : 1;
}ADS1118_Config;

union ADS1118_ConfigReg
{
	uint16_t reg_value;
	ADS1118_Config config;
};

#define	MUX_AINP_AIN0_AINN_AIN1		0x00
#define	MUX_AINP_AIN0_AINN_AIN3		0x01
#define	MUX_AINP_AIN1_AINN_AIN3		0x02
#define	MUX_AINP_AIN2_AINN_AIN3		0x03
#define	MUX_AINP_AIN0_AINN_GND		0x04
#define	MUX_AINP_AIN1_AINN_GND		0x05
#define	MUX_AINP_AIN2_AINN_GND		0x06
#define	MUX_AINP_AIN3_AINN_GND		0x07

#define PGA_FS_6_144V				0x00
#define PGA_FS_4_096V				0x01
#define PGA_FS_2_048V				0x02
#define PGA_FS_1_024V				0x03
#define PGA_FS_0_512V				0x04
#define PGA_FS_0_256V				0x07

#define CONT_CONV_MODE				0x00
#define PD_SINGLE_SHOT_MODE			0x01

#define DR_8SPS						0x00
#define DR_16SPS					0x01
#define DR_32SPS					0x02
#define DR_64SPS					0x03
#define DR_128SPS					0x04
#define DR_250SPS					0x05
#define DR_475SPS					0x06
#define DR_860SPS					0x07

#define ADC_MODE					0x00
#define TS_MODE						0x01

#define NOP_UPD_CONF_REG			0x01

#define FULL_SCALE_ADC				32768

#define ROOM_TEMP_A_COEFF			2006
#define ROOM_TEMP_B_COEFF			(-24615)
#define ROOM_TEMP_OFFSET_COEFF		(-5)
#define THERMOCOUPLE_COEFF			12600

void ADS1118_Init(void);
void ADS1118_ReadData(pData data);

#endif /* INC_ADS1118_H_ */
