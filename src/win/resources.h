/*
 * Copyright (C) 2020 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#define IDR_MENU1              101
#define IDD_TEXTINPUT          103
#define IDC_TEXTLABEL          104
#define IDC_TEXTEDIT           105

#define IDD_NUMINPUT           106
#define IDC_NUMLABEL           107
#define IDC_NUMEDIT            108
#define IDC_NUMSPIN            109

#define IDS_CHOOSE_GAME_FOLDER 1001

#define ID_SCREENSHOT             40001
#define ID_RESTART                40002
#define ID_EXIT                   40003
#define ID_SCREEN_WINDOW          40011
#define ID_SCREEN_FULL            40012
#define ID_SCREEN_INTEGER_SCALING 40013
#define ID_OPTION_MOUSE_MOVE      40021
#define ID_OPTION_AUTO_COPY       40022
#define ID_MSGSKIP                40030

struct SDL_RWops;
struct SDL_RWops *open_resource(const char* name, const char* type);

#endif /* __RESOURCES_H__ */
