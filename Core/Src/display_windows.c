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

extern Data dev_state;

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
	int temp_val = (int)DEV_RoundTemperatureValue(data->thermocouple_temp + data->room_temp); // add room temperature value, because thermocouple shows relative heat

	// check is thermocouple connected
	if(data->is_thermocouple_connected)
	{
		printInteger(wnd->strings[0].Text, "", &temp_val, delim, 1); // show temperature value
	}
	else
	{
		printString(wnd->strings[0].Text, "", "---");
	}

    wnd->strings[0].x_pos = 0;
    wnd->strings[0].y_pos = 20;
    wnd->strings[0].align = AlignCenter;
    wnd->strings[0].font = MSSanSerif_20;
    wnd->strings[0].inverted = NotInverted;

#ifdef IS_SHOW_ROOM_TEMP
    int room_temp_val = (int)data->room_temp;
	printInteger(wnd->strings[1].Text, "", &room_temp_val, delim, 1);

	wnd->strings[0].y_pos = 10; // override string position

    wnd->strings[1].x_pos = 0;
    wnd->strings[1].y_pos = 37;
    wnd->strings[1].align = AlignCenter;
    wnd->strings[1].font = MSSanSerif_20;
    wnd->strings[1].inverted = NotInverted;

    wnd->StringsQuantity = 2;
#else
    wnd->StringsQuantity = 1;
#endif

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
		bat_charge = DEV_GetBatteryCharge(data->vbat_mv);
	}

	DP_PaintBatteryIndicator(bat_charge);
	return 0;
}
