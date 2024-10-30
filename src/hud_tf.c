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

static void SCR_DrawTfBigClock(int x, int y, int style, int blink, float scale, const char* t)
{
	extern  mpic_t* sb_nums[2][11];
	extern  mpic_t* sb_colon/*, *sb_slash*/;
	qbool lblink = blink && ((int)(curtime * 10)) % 10 < 5;

	style = bound(0, style, 1);

	while (*t) {
		if (*t >= '0' && *t <= '9') {
			Draw_STransPic(x, y, sb_nums[style][*t - '0'], scale);
			x += 24 * scale;
		}
		else if (*t == ':') {
			if (lblink || !blink) {
				Draw_STransPic(x, y, sb_colon, scale);
			}

			x += 16 * scale;
		}
		else {
			Draw_SCharacter(x, y, *t + (style ? 128 : 0), 3 * scale);
			x += 24 * scale;
		}
		t++;
	}
}

static void SCR_DrawTfSmallClock(int x, int y, int style, int blink, float scale, const char* t, qbool proportional)
{
	qbool lblink = blink && ((int)(curtime * 10)) % 10 < 5;
	int c;

	style = bound(0, style, 3);

	while (*t) {
		c = (int)*t;
		if (c >= '0' && c <= '9') {
			if (style == 1) {
				c += 128;
			}
			else if (style == 2 || style == 3) {
				c -= 30;
			}
		}
		else if (c == ':') {
			if (style == 1 || style == 3) {
				c += 128;
			}
			if (lblink || !blink) {
				;
			}
			else {
				c = ' ';
			}
		}
		x += Draw_SCharacterP(x, y, c, scale, proportional);
		t++;
	}
}

static void SCR_HUD_DrawTfClock(hud_t* hud)
{
	int width, height;
	int x, y;
	int tens_minutes, minutes, tens_seconds, seconds;
	char t[80] = {0};

	static cvar_t
		* hud_tfclock_big = NULL,
		* hud_tfclock_style,
		* hud_tfclock_blink,
		* hud_tfclock_scale,
		* hud_tfclock_proportional;

	if (hud_tfclock_big == NULL)    // first time
	{
		hud_tfclock_big = HUD_FindVar(hud, "big");
		hud_tfclock_style = HUD_FindVar(hud, "style");
		hud_tfclock_scale = HUD_FindVar(hud, "scale");
		hud_tfclock_blink = HUD_FindVar(hud, "blink");
		hud_tfclock_proportional = HUD_FindVar(hud, "proportional");
	}

	tens_minutes = fmod(cl.tftime / 600, 6);
	minutes = fmod(cl.tftime / 60, 10);
	tens_seconds = fmod(cl.tftime / 10, 6);
	seconds = fmod(cl.tftime, 10);
	snprintf(t, sizeof(t), "%i%i:%i%i", tens_minutes, minutes, tens_seconds, seconds);
	width = SCR_GetClockStringWidth(t, hud_tfclock_big->integer, hud_tfclock_scale->value, hud_tfclock_proportional->integer);
	height = SCR_GetClockStringHeight(hud_tfclock_big->integer, hud_tfclock_scale->value);

	if (HUD_PrepareDraw(hud, width, height, &x, &y)) {
		if (hud_tfclock_big->value) {
			SCR_DrawTfBigClock(x, y, hud_tfclock_style->value, hud_tfclock_blink->value, hud_tfclock_scale->value, t);
		}
		else {
			SCR_DrawTfSmallClock(x, y, hud_tfclock_style->value, hud_tfclock_blink->value, hud_tfclock_scale->value, t, hud_tfclock_proportional->integer);
		}
	}
}

static void SCR_HUD_DrawScoreBlue(hud_t* hud)
{
	int width, height;
	int x, y;
	int blue, red;
	char t[80] = { 0 };
	char* score = Info_ValueForKey(cl.serverinfo, "score");
	if (!score[0]) return;

	static cvar_t
		* hud_score_style = NULL,
		* hud_score_align,
		* hud_score_scale,
		* hud_score_flags,
		* hud_score_digits,
		* hud_score_proportional;

	if (hud_score_style == NULL)    // first time
	{
		hud_score_style = HUD_FindVar(hud, "style");
		hud_score_scale = HUD_FindVar(hud, "scale");
		hud_score_flags = HUD_FindVar(hud, "flags");
		hud_score_digits = HUD_FindVar(hud, "digits");
		hud_score_align = HUD_FindVar(hud, "align");
		hud_score_proportional = HUD_FindVar(hud, "proportional");
	}

	blue = red = 0;
	while (*score != ':') {
		blue *= 10;
		blue += (*score) - '0';
		score++;
		if (!(*score)) return;
	}
	score++;
	while (*score) {
		red *= 10;
		red += (*score) - '0';
		score++;
	}
	
	if (hud_score_flags->value) {
		blue /= 10;
		red /= 10;
	}
	
	if (cl.spectator == cl.autocam) {
		SCR_HUD_DrawNum(hud, blue, 0, hud_score_scale->value, hud_score_style->value, hud_score_digits->value, hud_score_align->string, hud_score_proportional->integer);
	}
}

static void SCR_HUD_DrawScoreRed(hud_t* hud)
{
	int width, height;
	int x, y;
	int blue, red;
	char t[80] = { 0 };
	char* score = Info_ValueForKey(cl.serverinfo, "score");
	if (!score[0]) return;

	static cvar_t
		* hud_score_style = NULL,
		* hud_score_align,
		* hud_score_scale,
		* hud_score_flags,
		* hud_score_digits,
		* hud_score_proportional;

	if (hud_score_style == NULL)    // first time
	{
		hud_score_style = HUD_FindVar(hud, "style");
		hud_score_scale = HUD_FindVar(hud, "scale");
		hud_score_flags = HUD_FindVar(hud, "flags");
		hud_score_digits = HUD_FindVar(hud, "digits");
		hud_score_align = HUD_FindVar(hud, "align");
		hud_score_proportional = HUD_FindVar(hud, "proportional");
	}

	blue = red = 0;
	while (*score != ':') {
		blue *= 10;
		blue += (*score) - '0';
		score++;
		if (!(*score)) return;
	}
	score++;
	while (*score) {
		red *= 10;
		red += (*score) - '0';
		score++;
	}

	if (hud_score_flags->value) {
		blue /= 10;
		red /= 10;
	}

	if (cl.spectator == cl.autocam) {
		SCR_HUD_DrawNum(hud, red, 0, hud_score_scale->value, hud_score_style->value, hud_score_digits->value, hud_score_align->string, hud_score_proportional->integer);
	}
}

void TF_HudInit(void)
{
	HUD_Register(
		"scoreblue", NULL, "Shows current game time (mm:ss).",
		HUD_PLUSMINUS, ca_active, 8, SCR_HUD_DrawScoreBlue,
		"1", "top", "right", "console", "0", "0", "0", "0 0 0", NULL,
		"style", "0",
		"scale", "1",
		"align", "right",
		"digits", "3",
		"proportional", "0",
		"flags", "0",
		NULL
	);

	HUD_Register(
		"scorered", NULL, "Shows current game time (mm:ss).",
		HUD_PLUSMINUS, ca_active, 8, SCR_HUD_DrawScoreRed,
		"1", "top", "right", "console", "0", "0", "0", "0 0 0", NULL,
		"style", "0",
		"scale", "1",
		"align", "right",
		"digits", "3",
		"proportional", "0",
		"flags", "0",
		NULL
	);

	HUD_Register(
		"tfclock", NULL, "Shows current game time (mm:ss).",
		HUD_PLUSMINUS, ca_disconnected, 8, SCR_HUD_DrawTfClock,
		"1", "top", "right", "console", "0", "0", "0", "0 0 0", NULL,
		"big", "1",
		"style", "0",
		"scale", "1",
		"blink", "1",
		"proportional", "0",
		NULL
	);

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
