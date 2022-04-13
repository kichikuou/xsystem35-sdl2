/*
 * menu.c  popup menu
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
/* $Id: menu.c,v 1.14 2002/09/18 13:16:22 chikama Exp $ */

#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>

#include "portab.h"
#include "menu.h"
#include "menu_gui.h"
#include "menu_callback.h"
#include "nact.h"
#include "input.h"
#include "msgskip.h"

static boolean menu_initilized = FALSE;

static int menu_callback() {
	sys_getInputInfo();
	return TRUE;
}


void menu_open(void) {
	if (!menu_initilized) return;
	
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menu_item_msgskip_on), msgskip_isActivated());
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menu_item_msgskip_off), !msgskip_isActivated());
	// gtk_menu_popup_at_pointer() cannot be used because there's no GdkEvent here.
	gtk_menu_popup(GTK_MENU(menu_window_popup), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	gtk_widget_show(menu_window_popup);
	nact->popupmenu_opened = TRUE;
}

void menu_quitmenu_open(void) {
	if (!menu_initilized) return;
	
	gtk_widget_show(menu_window_exit);
	nact->popupmenu_opened = TRUE;
}

boolean menu_inputstring(INPUTSTRING_PARAM *p) {
	char s[256];

	if (!menu_initilized) return FALSE;
	
	menu_ok_input = FALSE;
	gtk_label_set_text(GTK_LABEL(menu_label_inputstring_title), p->title);
	
	sprintf(s, _("MAX %d charater"), p->max);
	gtk_label_set_text(GTK_LABEL(menu_label_inputstring_maxchar), strdup(s));
	
	gtk_entry_set_max_length(GTK_ENTRY(menu_textentry), p->max *2);
	gtk_entry_set_text(GTK_ENTRY(menu_textentry), p->oldstring);
	
	gtk_widget_show(menu_window_is);
	
	g_idle_add(menu_callback, menu_window_is);
	gtk_main();
	g_idle_remove_by_data(menu_window_is);
	
	if (menu_ok_input) {
		p->newstring = menu_textentry_string;
	} else {
		p->newstring = p->oldstring;
	}
	return TRUE;
}

boolean menu_inputstring2(INPUTSTRING_PARAM *p) {
	if (!menu_initilized) return FALSE;
	
	menu_ok_input = FALSE;
	gtk_entry_set_max_length(GTK_ENTRY(menu_textentry2), p->max *2);
	gtk_entry_set_text(GTK_ENTRY(menu_textentry2), p->oldstring);
	gtk_widget_set_size_request(menu_textentry2, p->h * p->max + 8, p->h + 4);
	
	gtk_widget_show(menu_window_is2);
	
	g_idle_add(menu_callback, menu_window_is2);
	gtk_main();
	g_idle_remove_by_data(menu_window_is2);
	
	if (menu_ok_input) {
		p->newstring = menu_textentry_string;
	} else {
		p->newstring = p->oldstring;
	}
	return TRUE;
}

boolean menu_inputnumber(INPUTNUM_PARAM *p) {
	GtkAdjustment *adj;
	char s[256];
	
	if (!menu_initilized) return FALSE;
	
	menu_ok_input = FALSE;
	
	gtk_window_set_title (GTK_WINDOW(menu_window_in), p->title);
	sprintf(s, "%d ", p->max);
	gtk_label_set_text(GTK_LABEL(menu_label_inputnum_max), s);
	sprintf(s, "%d ", p->min);
	gtk_label_set_text(GTK_LABEL(menu_label_inputnum_min), s);
	sprintf(s, "%d ", p->def);
	gtk_label_set_text(GTK_LABEL(menu_label_inputnum_def), s);
	
	adj = gtk_adjustment_new (p->def, p->min, p->max, 1, 1, 1);
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(menu_spinbutton), GTK_ADJUSTMENT(adj));
	/* こうしないと前の value が表示される */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(menu_spinbutton), p->def);
	
	gtk_widget_show(menu_window_in);
	
	g_idle_add(menu_callback, menu_window_in);
	gtk_main();
	g_idle_remove_by_data(menu_window_in);
	
	if (menu_ok_input) {
		p->value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(menu_spinbutton));
	} else {
		p->value = -1;
	}
	return TRUE;
}

void menu_msgbox_open(char *msg) {
	if (!menu_initilized) return;
	
	gtk_label_set_text(GTK_LABEL(menu_label_msgbox), msg);
	gtk_widget_show(menu_window_msgbox);
	g_idle_add((GSourceFunc)sys_getInputInfo, menu_window_msgbox);
	gtk_main();
	g_idle_remove_by_data(menu_window_msgbox);
}

void menu_init() {
	menu_initilized = TRUE;
	menu_window_popup = create_menu1();
	menu_window_in    = create_window1();
	menu_window_is    = create_window2();
	menu_window_exit  = create_window3();
	menu_window_about = create_window4();
	menu_window_is2   = create_window5();
	menu_window_msgbox= create_window6();
}

void menu_gtkmainiteration() {
	if (!menu_initilized) return;
	
	while (gtk_events_pending()) {
		gtk_main_iteration();
		menu_callback();
	}
}

void menu_setSkipState(boolean enabled, boolean activated) {
}
