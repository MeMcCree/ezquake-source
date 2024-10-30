/*
Copyright (C) 2011 azazello and ezQuake team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "quakedef.h"
#include "common_draw.h"
#include "hud.h"
#include "hud_common.h"
#include "vx_stuff.h"

static void SCR_HUD_DrawClip(hud_t* hud)
{
	static cvar_t* scale = NULL, * style, * digits, * align, * proportional;
	static int value;
	if (scale == NULL) {
		// first time called
		scale = HUD_FindVar(hud, "scale");
		style = HUD_FindVar(hud, "style");
		digits = HUD_FindVar(hud, "digits");
		align = HUD_FindVar(hud, "align");
		proportional = HUD_FindVar(hud, "proportional");
	}

	value = HUD_Stats(STAT_CLIP);
	if (cl.spectator == cl.autocam && value != -1) {
		SCR_HUD_DrawNum(hud, value, 0, scale->value, style->value, digits->value, align->string, proportional->integer);
	}
}

static void SCR_HUD_DrawNumGren1(hud_t* hud)
{
	static cvar_t* scale = NULL, * style, * digits, * align, * proportional;
	static int value;
	if (scale == NULL) {
		// first time called
		scale = HUD_FindVar(hud, "scale");
		style = HUD_FindVar(hud, "style");
		digits = HUD_FindVar(hud, "digits");
		align = HUD_FindVar(hud, "align");
		proportional = HUD_FindVar(hud, "proportional");
	}

	value = HUD_Stats(STAT_NUMGREN1);
	if (cl.spectator == cl.autocam) {
		SCR_HUD_DrawNum(hud, value, 0, scale->value, style->value, digits->value, align->string, proportional->integer);
	}
}

static void SCR_HUD_DrawNumGren2(hud_t* hud)
{
	static cvar_t* scale = NULL, * style, * digits, * align, * proportional;
	static int value;
	if (scale == NULL) {
		// first time called
		scale = HUD_FindVar(hud, "scale");
		style = HUD_FindVar(hud, "style");
		digits = HUD_FindVar(hud, "digits");
		align = HUD_FindVar(hud, "align");
		proportional = HUD_FindVar(hud, "proportional");
	}

	value = HUD_Stats(STAT_NUMGREN2);
	if (cl.spectator == cl.autocam) {
		SCR_HUD_DrawNum(hud, value, 0, scale->value, style->value, digits->value, align->string, proportional->integer);
	}
}

void SCR_HUD_DrawGrenIcon(hud_t* hud, int num, float scale)
{
	extern mpic_t* sb_grens[11];
	int   x, y, width, height;

	scale = max(scale, 0.01);

	width = height = 8 * scale;

	if (cl.spectator == cl.autocam && num != 0) {
		if (!HUD_PrepareDraw(hud, width, height, &x, &y) || num == 0)
			return;
		
		if (sb_grens[num] == NULL) return;
		Draw_SPic(x, y, sb_grens[num], scale);
	}
}

void SCR_HUD_DrawGrenIcon1(hud_t* hud)
{
	static cvar_t* scale = NULL;

	if (scale == NULL)  // first time called
	{
		scale = HUD_FindVar(hud, "scale");
	}
	
	SCR_HUD_DrawGrenIcon(hud, cl.stats[STAT_TPGREN1], scale->value);
}

void SCR_HUD_DrawGrenIcon2(hud_t* hud)
{
	static cvar_t* scale = NULL;

	if (scale == NULL)  // first time called
	{
		scale = HUD_FindVar(hud, "scale");
	}

	SCR_HUD_DrawGrenIcon(hud, cl.stats[STAT_TPGREN2], scale->value);
}

void TF_HudInit(void)
{
	// clip
	HUD_Register(
		"clip", NULL, "TF current clip",
		HUD_INVENTORY, ca_active, 0, SCR_HUD_DrawClip,
		"1", "face", "after", "center", "0", "0", "0", "0 0 0", NULL,
		"style", "0",
		"scale", "1",
		"align", "right",
		"digits", "3",
		"proportional", "0",
		NULL
	);

	// numgren1
	HUD_Register(
		"numgren1", NULL, "Part of your status - number of grenades of first type.",
		HUD_INVENTORY, ca_active, 0, SCR_HUD_DrawNumGren1,
		"1", "face", "after", "center", "0", "0", "0", "0 0 0", NULL,
		"style", "0",
		"scale", "1",
		"align", "right",
		"digits", "3",
		"proportional", "0",
		NULL
	);

	// numgren2
	HUD_Register(
		"numgren2", NULL, "Part of your status - number of grenades of second type.",
		HUD_INVENTORY, ca_active, 0, SCR_HUD_DrawNumGren2,
		"1", "face", "after", "center", "0", "0", "0", "0 0 0", NULL,
		"style", "0",
		"scale", "1",
		"align", "right",
		"digits", "3",
		"proportional", "0",
		NULL
	);

	HUD_Register("tpgren1", NULL, "Grenade type 1 icon",
		HUD_INVENTORY, ca_active, 0, SCR_HUD_DrawGrenIcon1,
		"0", "ibar", "left", "top", "0", "0", "0", "0 0 0", NULL,
		"style", "0",
		"scale", "0.25",
		"align", "right",
		"digits", "3",
		"proportional", "0",
		NULL);

	HUD_Register("tpgren2", NULL, "Grenade type 2 icon",
		HUD_INVENTORY, ca_active, 0, SCR_HUD_DrawGrenIcon2,
		"0", "ibar", "left", "top", "0", "0", "0", "0 0 0", NULL,
		"style", "0",
		"scale", "0.25",
		"align", "right",
		"digits", "3",
		"proportional", "0",
		NULL);
}
