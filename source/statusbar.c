#include <stdio.h>
#include <string.h>

#include "statusbar.h"
#include "gfx.h"

#include "wifi_full_bin.h"
#include "wifi_none_bin.h"
#include "battery_full_bin.h"
#include "battery_mid_high_bin.h"
#include "battery_mid_low_bin.h"
#include "battery_low_bin.h"
#include "battery_lowest_bin.h"
#include "battery_charging_bin.h"

u8* batteryLevels[] = {
	(u8*)battery_lowest_bin,
	(u8*)battery_lowest_bin,
	(u8*)battery_low_bin,
	(u8*)battery_mid_low_bin,
	(u8*)battery_mid_high_bin,
	(u8*)battery_full_bin,
};

#define SECONDS_IN_DAY 86400
#define SECONDS_IN_HOUR 3600
#define SECOND_IN_MINUTE 60

void drawStatusBar(bool wifiStatus, bool charging, int batteryLevel)
{
	u64 timeInSeconds = osGetTime() / 1000;
	u64 dayTime = timeInSeconds % SECONDS_IN_DAY;
	u16 hour = dayTime / SECONDS_IN_HOUR;
	u16 min = (dayTime % SECONDS_IN_HOUR) / SECOND_IN_MINUTE;

	char timeString[256];
	sprintf(timeString, "%d:%d", hour, min);
	gfxDrawText(GFX_TOP, GFX_LEFT, NULL, timeString, 240 - 18, 400 / 2 - 12);

	if(wifiStatus)
	{
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)wifi_full_bin, 18, 20, 240 - 18, 0);
	}
	else
	{
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)wifi_none_bin, 18, 20, 240 - 18, 0);
	}

	if(charging)
	{
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)battery_charging_bin, 18, 27, 240 - 18, 400 - 27);
	}
	else
	{
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, batteryLevels[batteryLevel], 18, 27, 240 - 18, 400 - 27);
	}
}

