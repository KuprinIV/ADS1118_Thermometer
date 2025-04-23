/*
 * display_windows.c
 *
 *  Created on: Apr 14, 2025
 *      Author: KUPRIN_IV
 */
#include "display_windows.h"
#include "device.h"
#include "print_to_string.h"
#include <stdio.h>

// callback
static int DisplayMainWindow(pWindow wnd, pData data, Action item_action, Action value_action);

// private variables
static Window MainWnd;
static pWindow CurrentWnd = &MainWnd;
//static pWindow temp_wnd;

extern Data dev_state;

//static uint8_t level = 0;

/**
  * @brief  Initialize interface windows
  * @param  none
  * @retval none
  */
void WindowsInit(void)
{
	// init windows
	MainWnd.next = NULL;
	MainWnd.prev = NULL;
	MainWnd.top = &MainWnd;
	MainWnd.bottom = NULL;
	MainWnd.callback = &DisplayMainWindow;
	CurrentWnd = &MainWnd;

	// draw main window
	RefreshWindow();
}

/**
  * @brief  Refresh window
  * @param  none
  * @retval none
  */
void RefreshWindow(void)
{
	CurrentWnd->callback(CurrentWnd, &dev_state, NoAction, NoAction);
	DP_UpdateBuffer();
	DP_ClearBuffer();
}

/**
  * @brief  Callback function for drawing main window
  * @param  wnd - data structure with window parameters
  * @param  data - data structure with device parameters
  * @param  item_action: NoAction - do nothing, Next - go to the next item, Prev - go to previous item
  * @param  value_action: NoAction - do nothing, Next - increase current item value, Prev - decrease current item value
  * @retval 0 - did some action in current window, 1 - after window drawing go to the top level window, 2 -  after window drawing go to the lower level window
  */
static int DisplayMainWindow(pWindow wnd, pData data, Action item_action, Action value_action)
{
	static uint8_t bat_charge;
	const char* delim[1] = {" ~C"}; // '~' is replaced by '°' symbol in font bitmaps array
	int temp_val = (int)data->thermocouple_temp;

	// show firmware version and release date
	printInteger(wnd->strings[0].Text, "", &temp_val, delim, 1);

    wnd->strings[0].x_pos = 1;
    wnd->strings[0].y_pos = 20;
    wnd->strings[0].align = AlignCenter;
    wnd->strings[0].font = MSSanSerif_20;
    wnd->strings[0].inverted = NotInverted;

	wnd->StringsQuantity = 1;
	DP_SetWindow(wnd);

	// draw battery charge
	if(DEV_GetChargeState() == Charging) // if battery is charging indicate it by ramp change of percent indicator
	{
		if(bat_charge < 10)
		{
			bat_charge++;
		}
		else
		{
			bat_charge = 0;
		}
	}
	else
	{
		bat_charge = getBatteryCharge(data->vbat_mv);
	}

	DP_PaintBatteryIndicator(bat_charge);
	return 0;
}
