/**
	
	Settings page module

	made by johnnycz, Jan 2007
	last edit:
		$Id: settings_page.c,v 1.22 2007-02-17 23:24:54 johnnycz Exp $

*/

#include "quakedef.h"
#include "settings.h"
#include "Ctrl_EditBox.h"

CEditBox editbox;

extern cvar_t menu_advanced;

cvar_t  skin_browser_showsize		= {"skin_browser_showsize",		"0"};
cvar_t  skin_browser_showdate		= {"skin_browser_showdate",		"1"};
cvar_t  skin_browser_showtime		= {"skin_browser_showtime",		"0"};
cvar_t  skin_browser_sortmode		= {"skin_browser_sortmode",		"1"};
cvar_t  skin_browser_showstatus		= {"skin_browser_showstatus",	"1"};
cvar_t  skin_browser_stripnames		= {"skin_browser_stripnames",	"1"};
cvar_t  skin_browser_interline		= {"skin_browser_interline",	"0"};
cvar_t  skin_browser_scrollnames	= {"skin_browser_scrollnames",	"1"};
cvar_t	skin_browser_democolor		= {"skin_browser_democolor",	"255 255 255 255"};	// White.
cvar_t	skin_browser_selectedcolor	= {"skin_browser_selectedcolor","0 150 235 255"};	// Light blue.
cvar_t	skin_browser_dircolor		= {"skin_browser_dircolor",		"170 80 0 255"};	// Redish.
#ifdef WITH_ZIP
cvar_t	skin_browser_zipcolor		= {"skin_browser_dircolor",		"255 170 0 255"};	// Orange.
#endif

filelist_t skins_filelist;

#define LETW 8
#define LINEHEIGHT 8
#define PADDING 1
#define EDITBOXWIDTH 16
#define EDITBOXMAXLENGTH 64
#define HELPLINES 5
#define SKINPREVIEWHEIGHT 120

static float SliderPos(float min, float max, float val) { return (val-min)/(max-min); }

static const char* colors[14] = { "White", "Brown", "Lavender", "Khaki", "Red", "Lt Brown", "Peach", "Lt Peach", "Purple", "Dk Purple", "Tan", "Green", "Yellow", "Blue" };
#define COLORNAME(x) colors[bound(0, ((int) x), sizeof(colors) / sizeof(char*) - 1)]

static int STHeight(setting* s) {
	if (s->advanced && !menu_advanced.value) return 0;
	switch (s->type) {
	case stt_separator: return LINEHEIGHT*3;
	case stt_advmark: case stt_basemark: return 0;
	default: return LINEHEIGHT+PADDING;
	}
}

static int Setting_PrintLabel(int x, int y, int w, const char *l, qbool active)
{
	int startpos = x + w/2 - min(strlen(l), w/2)*LETW;
	UI_Print(startpos, y, l, (int) active);
	x = w/2 + x;
	// if (active) UI_DrawCharacter(x, y, FLASHINGARROW());
	return x + LETW*2;
}

static void Setting_DrawNum(int x, int y, int w, setting* setting, qbool active)
{
	char buf[16];
	x = Setting_PrintLabel(x,y,w, setting->label, active);
	x = UI_DrawSlider (x, y, SliderPos(setting->min, setting->max, setting->cvar->value));
	if (setting->step > 0.99)
		snprintf(buf, sizeof(buf), "%3d", (int) setting->cvar->value);
	else
		snprintf(buf, sizeof(buf), "%3.1f", setting->cvar->value);

	UI_Print(x + LETW, y, buf, active);
}

static void Setting_DrawBool(int x, int y, int w, setting* setting, qbool active)
{
	x = Setting_PrintLabel(x,y,w, setting->label, active);
	UI_Print(x, y, setting->cvar->value ? "on" : "off", active);
}

static void Setting_DrawBoolAdv(int x, int y, int w, setting* setting, qbool active)
{
	const char* val;
	x = Setting_PrintLabel(x,y,w, setting->label, active);
	val = setting->readfnc ? setting->readfnc() : "off";
	UI_Print(x, y, val, active);
}

static void Setting_DrawSeparator(int x, int y, int w, setting* set)
{
	char buf[32];	
	snprintf(buf, sizeof(buf), "\x1d %s \x1f", set->label);
	UI_Print_Center(x, y+LINEHEIGHT+LINEHEIGHT/2, w, buf, true);
}

static void Setting_DrawAction(int x, int y, int w, setting* set, qbool active)
{
	Setting_PrintLabel(x, y, w, set->label, active);
}

static void Setting_DrawNamed(int x, int y, int w, setting* set, qbool active)
{
	x = Setting_PrintLabel(x, y, w, set->label, active);
	UI_Print(x, y, set->named_ints[(int) bound(set->min, set->cvar->value, set->max)], active);
}

static void Setting_DrawString(int x, int y, int w, setting* setting, qbool active)
{
	x = Setting_PrintLabel(x,y,w, setting->label, active);
	if (active) {
		CEditBox_Draw(&editbox, x, y, true);
	} else {
		UI_Print(x, y, setting->cvar->string, false);
	}
}

static void Setting_DrawColor(int x, int y, int w, setting* set, qbool active)
{
	x = Setting_PrintLabel(x, y, w, set->label, active);
	if (set->cvar->value >= 0) {
	 	Draw_Fill(x, y, LETW*3, LINEHEIGHT, Sbar_ColorForMap(set->cvar->value));
		UI_Print(x + 4*LETW, y, va("%i (%s)", (int) set->cvar->value, COLORNAME(set->cvar->value)), active);
	} else
		UI_Print(x, y, "off", active);
}

static void Setting_DrawSkin(int x, int y, int w, setting* set, qbool active)
{
	x = Setting_PrintLabel(x, y, w, set->label, active);
	UI_Print(x, y, set->cvar->string, active);
}

static void Setting_DrawBind(int x, int y, int w, setting* set, qbool active, qbool bindmode)
{
	int keys[2];
	char *name;
	char c[2];
	
	c[0] = FLASHINGARROW();
	c[1] = 0;

	x = Setting_PrintLabel(x, y, w, set->label, active);

	if (bindmode && active) {
		UI_Print(x, y, c, active);
	}

	x += LETW*2;
	
	M_FindKeysForCommand (set->varname, keys);

	if (keys[0] == -1) {
		UI_Print (x, y, "???", active);
	} else {
#ifdef WITH_KEYMAP
		char    str[256];
		name = Key_KeynumToString (keys[0], str);
#else // WITH_KEYMAP
		name = Key_KeynumToString (keys[0]);
#endif // WITH_KEYMAP else
		UI_Print (x, y, name, active);
		x += strlen(name)*8;
		if (keys[1] != -1) {
			UI_Print (x + 8, y, "or", active);
#ifdef WITH_KEYMAP
			UI_Print (x + 4*8, y, Key_KeynumToString (keys[1], str), active);
#else // WITH_KEYMAP
			UI_Print (x + 4*8, y, Key_KeynumToString (keys[1]), active);
#endif // WITH_KEYMAP else
		}
	}

}

static void Setting_Increase(setting* set) {
	float newval;

	switch (set->type) {
		case stt_bool: Cvar_Set (set->cvar, set->cvar->value ? "0" : "1"); break;
		case stt_custom: if (set->togglefnc) set->togglefnc(false); break;
		case stt_num:
		case stt_named:
		case stt_playercolor:
			newval = set->cvar->value + set->step;
			if (set->max >= newval)
				Cvar_SetValue(set->cvar, newval);
			else if (set->type == stt_named)
				Cvar_SetValue(set->cvar, set->min);
			break;
		case stt_action: if (set->actionfnc) set->actionfnc(); break;

		// unhandled
		case stt_separator:
		case stt_string:
		case stt_bind:
			break;
	}
}

static void Setting_Decrease(setting* set) {
	float newval;

	switch (set->type) {
		case stt_bool: Cvar_Set (set->cvar, set->cvar->value ? "0" : "1"); break;
		case stt_custom: if (set->togglefnc) set->togglefnc(true); break;
		case stt_num:
		case stt_named:
		case stt_playercolor:
			newval = set->cvar->value - set->step;
			if (set->min <= newval)
				Cvar_SetValue(set->cvar, newval);
			else if (set->type == stt_named)
				Cvar_SetValue(set->cvar, set->max);
			break;

		//unhandled
		case stt_separator:
		case stt_action:
		case stt_string:
		case stt_bind:
			break;
	}
}

static void Setting_Reset(setting* set)
{
	switch (set->type) {
		case stt_num:
		case stt_string:
		case stt_named:
		case stt_bool:
			Cvar_ResetVar(set->cvar);
			break;

		case stt_bind:
			break;
	}
}

static void Setting_BindKey(setting* set, int key)
{
	Key_SetBinding(key, set->varname);
}

static void M_UnbindCommand (const char *command) {
	int j, l;
	char *b;

	l = strlen(command);

	for (j = 0; j < (sizeof(keybindings) / sizeof(*keybindings)); j++) {
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
			Key_Unbind (j);
	}
}

static void Setting_UnbindKey(setting* set)
{
	M_UnbindCommand(set->varname);
}

// adjusts current viewed area of the settings page
static void CheckViewpoint(settings_page *tab, int h)
{
	if (tab->marked == 1 && tab->settings[0].type == stt_separator) tab->viewpoint = 0;

	if (tab->viewpoint > tab->marked) { 
		// marked entry is above us
		tab->viewpoint = tab->marked;
	} else while(STHeight(tab->settings + tab->marked) + tab->settings[tab->marked].top > tab->settings[tab->viewpoint].top + h) {
		// marked entry is below
		tab->viewpoint++;
	}
}

static void CheckCursor(settings_page *tab, qbool up)
{	// this makes sure that cursor doesn't point at some meta-entry, section heading or hidden setting
	setting *s;
	while (tab->marked < 0) { 
		tab->marked = 0;
		up = false;
	}
	if (tab->marked >= tab->count) {
		tab->marked = tab->count - 1;
		up = true;
	}
	s = tab->settings + tab->marked;
	if (s->type == stt_separator || (s->advanced && !menu_advanced.value) || s->type == stt_advmark || s->type == stt_basemark) {
		tab->marked += up ? -1 : +1;
		CheckCursor(tab, up);
	}
}

static void StringEntryLeave(setting* set) {
	Cvar_Set(set->cvar, editbox.text);
}

static void StringEntryEnter(setting* set) {
	CEditBox_Init(&editbox, EDITBOXWIDTH, EDITBOXMAXLENGTH);
	strncpy(editbox.text, set->cvar->string, EDITBOXMAXLENGTH);
}

static void EditBoxCheck(settings_page* tab, int oldm, int newm)
{
	if (tab->settings[oldm].type == stt_string && oldm != newm)
		StringEntryLeave(tab->settings + oldm);
	if (tab->settings[newm].type == stt_string && oldm != newm)
		StringEntryEnter(tab->settings + newm);
}

static void RecalcPositions(settings_page* page)
{	// especially when "show advanced options" is being changed
	// we need to recalculate where each option is
	int i;
	int curtop = 0;
	setting *s;

	for (i = 0; i < page->count; i++)
	{
		s = page->settings + i;
		s->top = curtop;
		
		curtop += STHeight(s);
	}
}

static void Setting_DrawHelpBox(int x, int y, int w, int h, settings_page* page)
{
	setting* s;
	char buf[2048];
	const char *helptext = "";

	UI_DrawBox(x, y, w, h);
	x += LETW; h -= LETW*2; w -= LETW*2; y += LETW;
	
	s = page->settings + page->marked;

	switch (s->type) {
	case stt_bool:
	case stt_named:
	case stt_num:
	case stt_string:
	case stt_playercolor:
		buf[0] = 0;
		helptext = "Further info not available...";
		Help_VarDescription(s->cvar->name, buf, sizeof(buf) - 1);
		helptext = buf;
		break;

	case stt_bind:
		if (page->mode == SPM_BINDING)
			helptext = "Press the key you want to assiciate with given action";
		else
			helptext = "Press Enter to change the associated key; Press Del to remove the binding";
		break;

	case stt_skin:
		helptext = "Press [Enter] to choose a skin image for this type of player";
		break;

	default:
		if (s->description)
			helptext = s->description;
		break;
	}

	if (!UI_PrintTextBlock(x, y, w, h, helptext, false))
		UI_Print(x, y + h, "Press [Space] to read more...", true);
}

static void Setting_DrawSkinPreview(int x, int y, int w, int h, char *skinfile)
{
	UI_DrawBox(x, y, bound(w, w, 320), h);

	// todo: finish the code, skinfile contains full system path to the .pcx file
	// we need to draw it into (x,y) location
}

qbool Settings_Key(settings_page* tab, int key)
{
	qbool up = false;
	setting_type type;
	int oldm = tab->marked;

	type = tab->settings[tab->marked].type;

	if (tab->mode == SPM_BINDING) {
		if (key != K_ESCAPE)
			Setting_BindKey(tab->settings + tab->marked, key);

		tab->mode = SPM_NORMAL;
		return true;
	}

	if (tab->mode == SPM_CHOOSESKIN) {
		if (key == K_ENTER) {
			char buf[MAX_PATH];
			COM_StripExtension(COM_SkipPath(FL_GetCurrentPath(&skins_filelist)), buf);
			Cvar_Set(tab->settings[tab->marked].cvar, buf);
			tab->mode = SPM_NORMAL;
			return true;
		}

		if (key == K_ESCAPE) {
			tab->mode = SPM_NORMAL;
			return true;
		}

		return FL_Key(&skins_filelist, key);
	}

	switch (key) { 
	case K_DOWNARROW:
	case K_MWHEELDOWN:
		tab->marked++; break;
	case K_UPARROW: 
	case K_MWHEELUP:
		tab->marked--; up = true; break;
	case K_PGDN: tab->marked += 5; break;
	case K_PGUP: tab->marked -= 5; up = true; break;
	case K_END: tab->marked = tab->count - 1; up = true; break;
	case K_HOME: tab->marked = 0; break;
	case K_RIGHTARROW:
		switch (type) {
		case stt_action: return false;
		case stt_string: CEditBox_Key(&editbox, key); return true;
		default: Setting_Increase(tab->settings + tab->marked);	return true;
		}

	case K_ENTER: case K_MOUSE1:
		switch (type) {
		case stt_string: StringEntryLeave(tab->settings + tab->marked); break;
		case stt_bind: tab->mode = SPM_BINDING; break;
		case stt_skin: tab->mode = SPM_CHOOSESKIN; break;
		default: Setting_Increase(tab->settings + tab->marked); break;
		}
		return true;

	case K_LEFTARROW: 
		switch (type) {
		case stt_action: return false;
		case stt_string: CEditBox_Key(&editbox, key); return true;
		default: Setting_Decrease(tab->settings + tab->marked);	return true;
		}

	case K_DEL:
		switch (type) {
		case stt_string: CEditBox_Key(&editbox, key); return true;
		case stt_bind: Setting_UnbindKey(tab->settings + tab->marked); return true;
		default: Setting_Reset(tab->settings + tab->marked); return true;
		}

	case K_SPACE:
		switch (tab->mode) {
		case SPM_NORMAL: tab->mode = SPM_VIEWHELP; return true;
		case SPM_VIEWHELP: tab->mode = SPM_NORMAL; return true;
		}
		break;

	default: 
		switch (type) {
		case stt_string:
			if (key != K_TAB && key != K_ESCAPE) {
				CEditBox_Key(&editbox, key);
				return true;
			}
			return false;

		default: return false;
		}
	}

	CheckCursor(tab, up);
	EditBoxCheck(tab, oldm, tab->marked);
	return true;
}

void Settings_Draw(int x, int y, int w, int h, settings_page* tab)
{
	int i;
	int ch;
	int nexttop;
	int hbh = 0;	// help box height
	setting *set;
	qbool active;
	static qbool prev_adv_state = false;

	if (!tab->count) return;

	if (prev_adv_state != (qbool) menu_advanced.value) {
		// someone toggled menu_advanced setting right in the currently viewed menu!
		RecalcPositions(tab);
		prev_adv_state = (qbool) menu_advanced.value;
	}

	nexttop = tab->settings[0].top;

	if (tab->mode == SPM_CHOOSESKIN) 
	{
		FL_Draw(&skins_filelist, x, y, w, h - SKINPREVIEWHEIGHT);
		Setting_DrawSkinPreview(x, y + h - SKINPREVIEWHEIGHT, w, SKINPREVIEWHEIGHT, FL_GetCurrentPath(&skins_filelist));
		return;
	}

	if (tab->mode != SPM_VIEWHELP) {
		hbh = HELPLINES * LINEHEIGHT;
	} else {
		hbh = h - LINEHEIGHT * 4;
	}

	Setting_DrawHelpBox(x, y + h - hbh, w, hbh, tab);
	h -= hbh;

	CheckViewpoint(tab, h);

	for (i = tab->viewpoint; i < tab->count && tab->settings[i].top + STHeight(tab->settings + i) <= h + tab->settings[tab->viewpoint].top; i++)
	{
		active = i == tab->marked;
		set = tab->settings + i;
		ch = STHeight(tab->settings + i);
		if ((set->advanced && !menu_advanced.value) || set->type == stt_advmark || set->type == stt_basemark) continue;

		if (active && set->type != stt_separator) {
			UI_DrawGrayBox(x, y, w, ch);
		}
		switch (set->type) {
			case stt_bool: if (set->cvar) Setting_DrawBool(x, y, w, set, active); break;
			case stt_custom: Setting_DrawBoolAdv(x, y, w, set, active); break;
			case stt_num: Setting_DrawNum(x, y, w, set, active); break;
			case stt_separator: Setting_DrawSeparator(x, y, w, set); break;
			case stt_action: Setting_DrawAction(x, y, w, set, active); break;
			case stt_named: Setting_DrawNamed(x, y, w, set, active); break;
			case stt_string: Setting_DrawString(x, y, w, set, active); break;
			case stt_playercolor: Setting_DrawColor(x, y, w, set, active); break;
			case stt_skin: Setting_DrawSkin(x, y, w, set, active); break;
			case stt_bind: Setting_DrawBind(x, y, w, set, active, tab->mode == SPM_BINDING); break;
		}
		y += ch;
		if (i < tab->count)
			nexttop = tab->settings[i+1].top;
	}
}

void Settings_OnShow(settings_page *page)
{
	int oldm = page->marked;

	RecalcPositions(page);

	CheckCursor(page, false);

	if (page->settings[page->marked].type == stt_string)
		StringEntryEnter(page->settings + page->marked);
}

void Settings_Init(settings_page *page, setting *arr, size_t size)
{
	int i;
	qbool onlyseparators = true;
	qbool advancedmode = false;

	page->count = size;
	page->marked = 0;
	page->settings = arr;
	page->viewpoint = 0;
	page->mode = SPM_NORMAL;

	for (i = 0; i < size; i++) {
		if (arr[i].type == stt_advmark) advancedmode = true;
		if (arr[i].type == stt_basemark) advancedmode = false;
		arr[i].advanced = advancedmode;

		if (onlyseparators && arr[i].type != stt_separator) {
			onlyseparators = false;
			page->marked = i;
		}
		if (arr[i].varname && !arr[i].cvar && arr[i].type == stt_bool) {
			arr[i].cvar = Cvar_FindVar(arr[i].varname);
			arr[i].varname = NULL;
			if (!arr[i].cvar)
				Cbuf_AddText(va("Warning: variable %s not found\n", arr[i].varname));
		}
	}

	RecalcPositions(page);

	if (onlyseparators) {
		Cbuf_AddText("Warning (Settings_Init): menu contained only separators\n");
		page->count = 0;
	}
}

void Settings_MainInit(void)
{
	Cvar_SetCurrentGroup(CVAR_GROUP_TEXTURES);
	Cvar_Register(&skin_browser_showsize);
    Cvar_Register(&skin_browser_showdate);
    Cvar_Register(&skin_browser_showtime);
    Cvar_Register(&skin_browser_sortmode);
    Cvar_Register(&skin_browser_showstatus);
    Cvar_Register(&skin_browser_stripnames);
    Cvar_Register(&skin_browser_interline);
	Cvar_Register(&skin_browser_scrollnames);
	Cvar_Register(&skin_browser_selectedcolor);
	Cvar_Register(&skin_browser_democolor);
	Cvar_Register(&skin_browser_dircolor);
#ifdef WITH_ZIP
	Cvar_Register(&skin_browser_zipcolor);
#endif
	Cvar_ResetCurrentGroup();

	FL_Init(&skins_filelist,
        &skin_browser_sortmode,
        &skin_browser_showsize,
        &skin_browser_showdate,
        &skin_browser_showtime,
        &skin_browser_stripnames,
        &skin_browser_interline,
        &skin_browser_showstatus,
		&skin_browser_scrollnames,
		&skin_browser_democolor,
		&skin_browser_selectedcolor,
		&skin_browser_dircolor,
#ifdef WITH_ZIP
		&skin_browser_zipcolor,
#endif
		"./qw/skins");

	FL_AddFileType(&skins_filelist, 0, ".pcx");
}
