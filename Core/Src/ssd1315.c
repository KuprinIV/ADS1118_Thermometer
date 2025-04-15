/*
 * ssd1315.c
 *
 *  Created on: 16 февр. 2025 г.
 *      Author: Ilya
 */

#include "ssd1315.h"
#include <string.h>

// low-level display functions
static void SSD1315_SetColumnStartAddressInPAM(uint8_t start_addr);
static void SSD1315_SetPageStartAddressInPAM(uint8_t start_addr);
static void SSD1315_SetMemoryAddressingMode(uint8_t mode);
static void SSD1315_SetColumnAddressRange(uint8_t start_addr, uint8_t end_addr);
static void SSD1315_SetPageAddressRange(uint8_t start_addr, uint8_t end_addr);
static void SSD1315_SetDisplayStartLine(uint8_t start_line);
static void SSD1315_SetDisplayContrast(uint8_t contrast);
static void SSD1315_SetDisplaySegmentsRemap(uint8_t is_remap);
static void SSD1315_SetEntireDisplayOn(void);
static void SSD1315_SetDisplayMode(uint8_t mode);
static void SSD1315_SetMultiplexRatio(uint8_t mux_ratio);
static void SSD1315_SetIref(uint8_t is_external, uint8_t int_iref);
static void SSD1315_SetDisplayOnOff(uint8_t state);
static void SSD1315_SetComOutsScanDirection(uint8_t is_remap);
static void SSD1315_SetDisplayOffset(uint8_t offset);
static void SSD1315_SetDisplayClock(uint8_t clk_div, uint8_t osc_freq);
static void SSD1315_SetPrechargePeriod(uint8_t phase1_per, uint8_t phase2_per);
static void SSD1315_SetComOutsHwConfig(uint8_t is_alter_cfg, uint8_t is_remap);
static void SSD1315_SetVComHLevel(uint8_t level);
static void SSD1315_NOP(void);
static void SSD1315_SetChargePump(uint8_t is_enabled, uint8_t level);
static void SSD1315_HorizontalScrollSetup(uint8_t start_col_addr, uint8_t end_col_addr, uint8_t start_page_addr, uint8_t end_page_addr,
		uint8_t scroll_interval, uint8_t dir);
static void SSD1315_VerticalAndHorizontalScrollSetup(uint8_t start_col_addr, uint8_t end_col_addr, uint8_t start_page_addr, uint8_t end_page_addr,
		uint8_t scroll_interval, uint8_t dir, uint8_t is_hor_scroll_enabled, uint8_t vertical_scroll_offset);
static void SSD1315_ScrollEnableCtrl(uint8_t is_enabled);
static void SSD1315_SetVerticalScrollArea(uint8_t num_of_fixed_rows_at_top, uint8_t num_of_rows_in_area);
static void SSD1315_SetScrollBySingleCol(uint8_t start_col_addr, uint8_t end_col_addr, uint8_t start_page_addr, uint8_t end_page_addr, uint8_t dir);
static void SSD1315_SetFadeOutAndBlinking(uint8_t is_enabled, uint8_t action, uint8_t step_interval);
static void SSD1315_ZoomInModeCtrl(uint8_t is_enabled);

// I2C write display functions
static void SSD1315_write_command(uint8_t cmd, uint8_t* cmd_data, uint8_t cmd_data_length);
static void SSD1315_write_data(uint8_t* data, uint16_t data_length);

extern I2C_HandleTypeDef hi2c1;

/**
 * @brief Init SSD1315 OLED controller
 * @param None
 * @retval None
 */
void SSD1315_Init(void)
{
	HAL_Delay(50); // wait at least 20 ms
	// check is I2C device on bus
	if(HAL_I2C_IsDeviceReady(&hi2c1, DEVICE_ADDR, 3, 100) == HAL_OK)
	{
		// set horizontal memory addressing mode
		SSD1315_SetMemoryAddressingMode(0);
		// set column start and end address: from 0 to 127
		SSD1315_SetColumnAddressRange(0, 127);
		// set page start and end address: from 0 to 7
		SSD1315_SetPageAddressRange(0, 7);
		// set charge pump output to 7,5 V
		SSD1315_SetChargePump(1, 0);
		// remap columns
		SSD1315_SetDisplaySegmentsRemap(1);

		// enable display
		SSD1315_SetDisplayOnOff(1);
	}
}

/**
 * @brief Load framebuffer to SSD1315 OLED controller
 * @param None
 * @retval None
 */
void SSD1315_UpdateFramebuffer(uint8_t* lcd_framebuffer, uint16_t length)
{
	SSD1315_write_data(lcd_framebuffer, length);
}

/**
 * @brief Enable control of SSD1315 OLED controller
 * @param is_on: 0 - disabled, 1 - enabled
 * @retval None
 */
void SSD1315_DisplayOnOff(uint8_t is_on)
{
	// enable display
	SSD1315_SetDisplayOnOff(is_on);
}

/****************** Low-level display functions *********************/

/**
 * @brief Set column start address in page address memory mode
 * @param start_addr - column start address
 * @retval None
 */
static void SSD1315_SetColumnStartAddressInPAM(uint8_t start_addr)
{
	// set lower nibble of address
	SSD1315_write_command((start_addr & 0x0F), NULL, 0);
	// set higher nibble of address
	SSD1315_write_command(((start_addr >> 4) & 0x07) | 0x10, NULL, 0);
}

/**
 * @brief Set page start address in page address memory mode
 * @param start_addr - page start address
 * @retval None
 */
static void SSD1315_SetPageStartAddressInPAM(uint8_t start_addr)
{
	SSD1315_write_command(((start_addr & 0x07) + 0xB0), NULL, 0);
}

/**
 * @brief Set memory addressing mode
 * @param mode: 0 - horizontal mode, 1 - vertical mode, 2 - page mode (default), 3 - not used
 * @retval None
 */
static void SSD1315_SetMemoryAddressingMode(uint8_t mode)
{
	uint8_t command_param = 0;
	// set memory addressing mode
	command_param = (mode & 0x03);
	SSD1315_write_command(CMD_SET_MEM_ADDRESSING_MODE, &command_param, 1);
}

/**
 * @brief Set column addresses range (used only for horizontal or vertical addressing memory addressing mode)
 * @param start_addr - column start address (minimum value 0, default 0)
 * @param end_addr - column end address (max value 127, default 127)
 * @retval None
 */
static void SSD1315_SetColumnAddressRange(uint8_t start_addr, uint8_t end_addr)
{
	uint8_t command_params[2] = {0};
	// check values
	if(start_addr >= end_addr) return;
	if(start_addr > 127) start_addr = 0;
	if(end_addr > 127) end_addr = 127;
	// set column start and end address
	command_params[0] = start_addr;
	command_params[1] = end_addr;
	SSD1315_write_command(CMD_SET_COL_ADDRESS, command_params, 2);
}

/**
 * @brief Set page addresses range (used only for horizontal or vertical addressing memory addressing mode)
 * @param start_addr - page start address (minimum value 0, default 0)
 * @param end_addr - page end address (max value 7, default 7)
 * @retval None
 */
static void SSD1315_SetPageAddressRange(uint8_t start_addr, uint8_t end_addr)
{
	uint8_t command_params[2] = {0};
	// check values
	if(start_addr >= end_addr) return;
	if(start_addr > 127) start_addr = 0;
	if(end_addr > 7) end_addr = 7;
	// set page start and end address
	command_params[0] = start_addr;
	command_params[1] = end_addr;
	SSD1315_write_command(CMD_SET_PAGE_ADDRESS, command_params, 2);
}

/**
 * @brief Set display start line in RAM
 * @param start_line - value from 0 to 63
 * @retval None
 */
static void SSD1315_SetDisplayStartLine(uint8_t start_line)
{
	// set start line
	SSD1315_write_command(((start_line & 0x3F) + 0x40), NULL, 0);
}

/**
 * @brief Set display contrast value
 * @param contrast - value from 1 to 255 (default 127)
 * @retval None
 */
static void SSD1315_SetDisplayContrast(uint8_t contrast)
{
	uint8_t command_param = 0;
	// check value
	if(contrast < 1) contrast = 1;
	// set contrast value
	command_param = contrast;
	SSD1315_write_command(CMD_SET_CONTRAST_CTRL, &command_param, 1);
}

/**
 * @brief Set display segments remapping
 * @param is_remap: 0 - not remapped (column address 0 is mapped to SEG0), 1 - is remapped (column address 127 is mapped to SEG0)
 * @retval None
 */
static void SSD1315_SetDisplaySegmentsRemap(uint8_t is_remap)
{
	SSD1315_write_command(CMD_SET_SEG_REMAP + (is_remap & 0x01), NULL, 0);
}

/**
 * @brief SSD1315 enable entire display on
 * @param None
 * @retval None
 */
static void SSD1315_SetEntireDisplayOn(void)
{
	SSD1315_write_command(CMD_ENTIRE_DISPLAY_ON, NULL, 0);
}

/**
 * @brief Set display mode
 * @param mode: 0 - normal mode, 1 - inverted
 * @retval None
 */
static void SSD1315_SetDisplayMode(uint8_t mode)
{
	SSD1315_write_command(CMD_NORMAL_DISPLAY + (mode & 0x01), NULL, 0);
}

/**
 * @brief Set multiplex ratio
 * @param mux_ratio - value from 15 to 63
 * @retval None
 */
static void SSD1315_SetMultiplexRatio(uint8_t mux_ratio)
{
	uint8_t command_param = 0;
	// check value
	if(mux_ratio < 15) mux_ratio = 15;
	// set MUX ratio value
	command_param = (mux_ratio & 0x3F);
	SSD1315_write_command(CMD_MUX_RATIO, &command_param, 1);
}

/**
 * @brief Set Iref segment current
 * @param is_external: 0 - external Iref setting, 1 - internal Iref setting
 * @param int_iref: 0 - Iref = 19 uA (152 uA per segment), 1 - Iref = 30 uA (240 uA per segment)
 * @retval None
 */
static void SSD1315_SetIref(uint8_t is_external, uint8_t int_iref)
{
	uint8_t command_param = 0;
	// set values
	command_param = ((is_external & 0x01)<<4) | ((int_iref & 0x01)<<5);
	SSD1315_write_command(CMD_SET_IREF, &command_param, 1);
}

/**
 * @brief Set display on/off
 * @param state: 0 - display off, 1 - display on
 * @retval None
 */
static void SSD1315_SetDisplayOnOff(uint8_t state)
{
	SSD1315_write_command(CMD_SET_DISPLAY_OFF + (state & 0x01), NULL, 0);
}

/**
 * @brief Set COM outputs scan direction
 * @param is_remap: 0 - normal mode (scan from COM0 to COM(N-1)), 1 - 0 - remapped mode (scan from COM(N-1) to COM0)
 * @retval None
 */
static void SSD1315_SetComOutsScanDirection(uint8_t is_remap)
{
	uint8_t cmd = 0xC0;
	if(is_remap) cmd |= 0x08;
	SSD1315_write_command(cmd, NULL, 0);
}

/**
 * @brief Set display offset
 * @param offset - vertical shift by COM from 0 to 63
 * @retval None
 */
static void SSD1315_SetDisplayOffset(uint8_t offset)
{
	uint8_t command_param = 0;
	// set offset value
	command_param = (offset & 0x3F);
	SSD1315_write_command(CMD_SET_DISPLAY_OFFSET, &command_param, 1);
}

/**
 * @brief Set display clock parameters
 * @param clk_div: divide clock ratio = clk_div+1
 * @param osc_freq - set oscillator frequency. Default is 0x08
 * @retval None
 */
static void SSD1315_SetDisplayClock(uint8_t clk_div, uint8_t osc_freq)
{
	uint8_t command_param = 0;
	// set clock divide value
	command_param = ((clk_div & 0x0F) | ((osc_freq & 0x0F)<<4));
	SSD1315_write_command(CMD_SET_DISPLAY_CLKDIV, &command_param, 1);
}

/**
 * @brief Set pre-charge period
 * @param phase1_per: phase 1 period up to 30 DCLK (2, 4, 6...). Default is 2
 * @param phase2_per: phase 2 period up to 30 DCLK (2, 4, 6...). Default is 2
 * @retval None
 */
static void SSD1315_SetPrechargePeriod(uint8_t phase1_per, uint8_t phase2_per)
{
	uint8_t command_param = 0;
	// check values
	if(phase1_per == 0 || phase2_per == 0) return;
	// set clock divide value
	command_param = ((phase1_per & 0x0F) | ((phase2_per & 0x0F)<<4));
	SSD1315_write_command(CMD_SET_PRECHG_PERIOD, &command_param, 1);
}

/**
 * @brief Set COM pins hardware configuration
 * @param is_alter_cfg: 0 - sequential pins configuration, 1 - alternative COM pins configuration (default)
 * @param is_remap: 0 - disable COM left/right remap (default), 1 - enable COM left/right remap
 * @retval None
 */
static void SSD1315_SetComOutsHwConfig(uint8_t is_alter_cfg, uint8_t is_remap)
{
	uint8_t command_param = 0;
	// set configuration
	command_param = (((is_alter_cfg & 0x01)<<4) | ((is_remap & 0x01)<<5));
	SSD1315_write_command(CMD_SET_COM_PINS_HWCFG, &command_param, 1);
}

/**
 * @brief Set COM pins hign level voltage
 * @param level: 0 - 0,65*Vcc, 1 - 0,71*Vcc, 2 - 0,77*Vcc (default), 3 - 0,83*Vcc
 * @retval None
 */
static void SSD1315_SetVComHLevel(uint8_t level)
{
	uint8_t command_param = 0;
	// set level
	command_param = ((level & 0x03)<<4);
	SSD1315_write_command(CMD_SET_VCOMH_LVL, &command_param, 1);
}

/**
 * @brief NOP command
 * @param None
 * @retval None
 */
static void SSD1315_NOP(void)
{
	SSD1315_write_command(CMD_NOP, NULL, 0);
}

/**
 * @brief Charge pump control
 * @param is_enabled: 0 - disable (default), 1 - enable
 * @param level: 0 - 7,5V (default), 2 - 8,5V, 3 - 9V
 * @retval None
 */
static void SSD1315_SetChargePump(uint8_t is_enabled, uint8_t level)
{
	uint8_t command_param = 0;
	uint8_t level_cfgs[4] = {0x14, 0x00, 0x94, 0x95};
	// set configuration
	if(is_enabled)
	{
		command_param = level_cfgs[level & 0x03];
		SSD1315_write_command(CMD_CHG_PUMP_SETTING, &command_param, 1);
	}
}

/**
 * @brief Horizontal scroll setup
 * @param start_col_addr - start column address
 * @param end_col_addr - end column address
 * @param start_page_addr - start page address
 * @param end_page_addr - end page address
 * @param scroll_interval - scroll step in display clock frames: 0 - 6 frames, 1 - 32 frames, 2 - 64 frames, 3 - 128 frames,
 * 4 - 3 frames, 5 - 4 frames, 6 - 5 frames, 7 - 2 frames
 * @param dir: 0 - right scroll, 1 - left scroll
 * @retval None
 */
static void SSD1315_HorizontalScrollSetup(uint8_t start_col_addr, uint8_t end_col_addr, uint8_t start_page_addr, uint8_t end_page_addr, uint8_t scroll_interval, uint8_t dir)
{
	uint8_t command_params[6] = {0};

	// check values
	if(start_col_addr > end_col_addr) start_col_addr = end_col_addr;
	if(start_page_addr > end_page_addr) start_page_addr = end_page_addr;

	// set parameters
	command_params[0] = 0; // dummy byte
	command_params[1] = (start_page_addr & 0x07);
	command_params[2] = (scroll_interval & 0x07);
	command_params[3] = (end_page_addr & 0x07);
	command_params[4] = (start_col_addr & 0x7F);
	command_params[5] = (end_col_addr & 0x7F);

	SSD1315_write_command(CMD_HOR_SCROLL_SETUP + (dir & 0x01), command_params, 6);
}

/**
 * @brief Vertical scroll setup
 * @param start_col_addr - start column address
 * @param end_col_addr - end column address
 * @param start_page_addr - start page address
 * @param end_page_addr - end page address
 * @param scroll_interval - scroll step in display clock frames: 0 - 6 frames, 1 - 32 frames, 2 - 64 frames, 3 - 128 frames,
 * 4 - 3 frames, 5 - 4 frames, 6 - 5 frames, 7 - 2 frames
 * @param dir: 0 - vertical and right scroll, 1 - vertical and left scroll
 * @retval None
 */
static void SSD1315_VerticalAndHorizontalScrollSetup(uint8_t start_col_addr, uint8_t end_col_addr, uint8_t start_page_addr, uint8_t end_page_addr,
		uint8_t scroll_interval, uint8_t dir, uint8_t is_hor_scroll_enabled, uint8_t vertical_scroll_offset)
{
	uint8_t command_params[7] = {0};

	// check values
	if(start_col_addr > end_col_addr) start_col_addr = end_col_addr;
	if(start_page_addr > end_page_addr) start_page_addr = end_page_addr;

	// set parameters
	command_params[0] = (is_hor_scroll_enabled & 0x01);
	command_params[1] = (start_page_addr & 0x07);
	command_params[2] = (scroll_interval & 0x07);
	command_params[3] = (end_page_addr & 0x07);
	command_params[4] = (vertical_scroll_offset & 0x3F);
	command_params[5] = (start_col_addr & 0x7F);
	command_params[6] = (end_col_addr & 0x7F);

	SSD1315_write_command(CMD_VER_SCROLL_SETUP + (dir & 0x01), command_params, 7);
}

/**
 * @brief Scroll enable control
 * @param is_enabled: 0 - scroll disabled, 1 - scroll enabled
 * @retval None
 */
static void SSD1315_ScrollEnableCtrl(uint8_t is_enabled)
{
	SSD1315_write_command(CMD_DEACTIVATE_SCROLL + (is_enabled & 0x01), NULL, 0);
}

static void SSD1315_SetVerticalScrollArea(uint8_t num_of_fixed_rows_at_top, uint8_t num_of_rows_in_area)
{
	uint8_t command_params[2] = {0};

	// check parameters
	if(num_of_fixed_rows_at_top + num_of_rows_in_area > 63) num_of_fixed_rows_at_top = 63 - num_of_rows_in_area;

	// set parameters
	command_params[0] = (num_of_fixed_rows_at_top & 0x1F);
	command_params[1] = (num_of_rows_in_area & 0x3F);

	SSD1315_write_command(CMD_SET_VER_SCROLL_AREA, command_params, 2);
}

/**
 * @brief Content scroll by 1 column
 * @param start_col_addr - start column address
 * @param end_col_addr - end column address
 * @param start_page_addr - start page address
 * @param end_page_addr - end page address
 * @param dir: 0 - right scroll, 1 - left scroll
 * @retval None
 */
static void SSD1315_SetScrollBySingleCol(uint8_t start_col_addr, uint8_t end_col_addr, uint8_t start_page_addr, uint8_t end_page_addr, uint8_t dir)
{
	uint8_t command_params[6] = {0};

	// check values
	if(start_col_addr > end_col_addr) start_col_addr = end_col_addr;
	if(start_page_addr > end_page_addr) start_page_addr = end_page_addr;

	// set parameters
	command_params[0] = 0; // dummy byte
	command_params[1] = (start_page_addr & 0x07);
	command_params[2] = 1; // dummy byte
	command_params[3] = (end_page_addr & 0x07);
	command_params[4] = (start_col_addr & 0x7F);
	command_params[5] = (end_col_addr & 0x7F);

	SSD1315_write_command(CMD_CONTENT_SCROLL_SETUP + (dir & 0x01), command_params, 6);
}

/**
 * @brief Set fade out of blinking mode
 * @param is_enabled: 0 - modes disabled, 1 - mode enabled
 * @param action: 0 - fade out mode, 1 - blinking mode
 * @param step_interval - fade step interval in display clock frames. 0 - 8 frames, 1 - 16 frames, ..., 15 - 128 frames
 * @retval None
 */
static void SSD1315_SetFadeOutAndBlinking(uint8_t is_enabled, uint8_t action, uint8_t step_interval)
{
	uint8_t command_param = 0;

	command_param = (((is_enabled & 0x01) << 5) | ((action & 0x01) << 4) | (step_interval & 0x0F));
	SSD1315_write_command(CMD_SET_FADE_OUT_BLINK, &command_param, 1);
}

/**
 * @brief Zoom In mode control
 * @param is_enabled: 0 - mode disabled, 1 - mode enabled
 * @retval None
 */
static void SSD1315_ZoomInModeCtrl(uint8_t is_enabled)
{
	uint8_t command_param = (is_enabled & 0x01);
	SSD1315_write_command(CMD_SET_ZOOM_IN, &command_param, 1);
}

/****************** I2C write display functions *********************/

/**
 * @brief SSD1315 write command
 * @param cmd - command code
 * @param cmd_data - command data pointer
 * @param cmd_data_length - command data buffer length
 * @retval None
 */
static void SSD1315_write_command(uint8_t cmd, uint8_t* cmd_data, uint8_t cmd_data_length)
{
	uint8_t data[16] = {0};
	uint8_t control_byte = 0x80; // control byte: Co bit is set to "1", D/C# bit is set to "0"
	// store command code
	data[0] = control_byte;
	data[1] = cmd;
	// store command data bytes
	if(cmd_data_length > 7) cmd_data_length = 7; // limit command data buffer length
	if(cmd_data != NULL && cmd_data_length > 0)
	{
		for(uint8_t i = 0; i < cmd_data_length; i++)
		{
			data[2*i+2] = control_byte;
			data[2*i+3] = cmd_data[i];
		}
	}
	// send data to display
	HAL_I2C_Master_Transmit(&hi2c1, DEVICE_ADDR, data, 2*(cmd_data_length+1), 1000);
}

/**
 * @brief SSD1315 write data
 * @param data - display data pointer
 * @param data_length - display data buffer length
 * @retval None
 */
static void SSD1315_write_data(uint8_t* data, uint16_t data_length)
{
	// init control byte
	uint8_t control_byte = 0x40; // control byte: Co bit is set to "0", D/C# bit is set to "1 "
	// send data to display
	HAL_I2C_Mem_Write(&hi2c1, DEVICE_ADDR, control_byte, 1, data, data_length, 1000);
}
