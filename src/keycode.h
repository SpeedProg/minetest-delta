/*
Minetest-c55
Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef KEYCODE_HEADER
#define KEYCODE_HEADER

#include "common_irrlicht.h"
#include <string>
#include "gettext.h"

static const char *KeyNames[] =
	{ "-", N_("Left Button"), N_("Right Button"), N_("Cancel"), N_("Middle Button"),
			N_("X Button 1"), N_("X Button 2"), "-", N_("Back"), N_("Tab"), "-",
			"-", N_("Clear"), N_("Return"), "-", "-", N_("Shift"), N_("Control"),
			N_("Menu"), N_("Pause"), N_("Capital"), N_("Kana"), "-", N_("Junja"),
			N_("Final"), N_("Kanji"), "-", N_("Escape"), N_("Convert"),
			N_("Nonconvert"), N_("Accept"), N_("Mode Change"), N_("Space"),
			N_("Priot"), N_("Next"), N_("End"), N_("Home"), N_("Left"), N_("Up"),
			N_("Right"), N_("Down"), N_("Select"), N_("Print"), N_("Execute"),
			N_("Snapshot"), N_("Insert"), N_("Delete"), N_("Help"), "0", "1",
			"2", "3", "4", "5",	"6", "7", "8", "9", "-", "-", "-", "-", "-",
			"-", "-", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K",
			"L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
			"Y", "Z", N_("Left Windows"), N_("Right Windows"), N_("Apps"), "-",
			N_("Sleep"), N_("Numpad 0"), N_("Numpad 1"), N_("Numpad 2"),
			N_("Numpad 3"), N_("Numpad 4"), N_("Numpad 5"), N_("Numpad 6"),
			N_("Numpad 7"), N_("Numpad 8"), N_("Numpad 9"), N_("Numpad *"),
			N_("Numpad +"), N_("Numpad /"), N_("Numpad -"),	N_("Numpad ."),
			N_("Numpad /"), "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
			"F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17", "F18",
			"F19", "F20", "F21", "F22", "F23", "F24", "-", "-", "-", "-", "-",
			"-", "-", "-", N_("Num Lock"), N_("Scroll Lock"), "-", "-", "-",
			"-", "-", "-", "-",	"-", "-", "-", "-", "-", "-", "-",
			N_("Left Shift"), N_("Right Shight"), N_("Left Control"),
			N_("Right Control"), N_("Left Menu"), N_("Right Menu"), "-", "-",
			"-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
			"-", "-", "-", "-", "-", "-", N_("Plus"), N_("Comma"), N_("Minus"),
			N_("Period"), "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
			"-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
			"-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
			"-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
			"-", "-", "-", "-", "-", N_("Attn"), N_("CrSel"), N_("ExSel"),
			N_("Erase OEF"), N_("Play"), N_("Zoom"), N_("PA1"),
			N_("OEM Clear"), "-" };


irr::EKEY_CODE keyname_to_keycode(const char *name);
std::string keycode_to_keyname(s32 keycode);

// Key configuration getter
irr::EKEY_CODE getKeySetting(const char *settingname);

// Clear fast lookup cache
void clearKeyCache();

#endif

