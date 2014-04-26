/*
 * menu_gui.h menu gui
 *
 * Copyright (C) 2000- Masaki Chikama (Wren) <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: menu_gui.h,v 1.4 2001/04/04 21:55:40 chikama Exp $ */

#ifndef __MENU_GUI__
#define __MENU_GUI__

#include <gtk/gtk.h>

GtkWidget* create_window1 (void);
GtkWidget* create_window2 (void);
GtkWidget* create_window3 (void);
GtkWidget* create_window4 (void);
GtkWidget* create_window5 (void);
GtkWidget* create_window6 (void);
GtkWidget* create_menu1 (void);

#endif /* !__MENU_GUI__ */
