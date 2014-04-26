/*
 * menu_callback.c  menu callback function
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
/* $Id: menu_callback.c,v 1.11 2003/04/25 17:23:55 chikama Exp $ */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "portab.h"
#include "menu.h"
#include "menu_callback.h"
#include "menu_gui.h"
#include "nact.h"
#include "imput.h"
#include "music.h"
#include "s39init.h"

boolean menu_ok_exit;   /* exit window で OK が押されたかどうか */
boolean menu_ok_input;  /* 文字列/数値入力 window で OK が... */

GtkWidget *menu_item_pcm;
GtkWidget *menu_item_cdrom;
GtkWidget *menu_item_midi;
GtkWidget *menu_item_msgskip_on;
GtkWidget *menu_item_msgskip_off;

GtkWidget *menu_label_inputstring_title;
GtkWidget *menu_label_inputstring_maxchar;
GtkWidget *menu_label_inputnum_max;
GtkWidget *menu_label_inputnum_min;
GtkWidget *menu_label_inputnum_def;
GtkWidget *menu_label_msgbox;
GtkWidget *menu_spinbutton;
GtkWidget *menu_textentry;
GtkWidget *menu_textentry2;
gchar     *menu_textentry_string;

GtkWidget *menu_window_popup; /* popup menu */
GtkWidget *menu_window_exit;  /* exit window */
GtkWidget *menu_window_is;    /* input string window */
GtkWidget *menu_window_is2;   /* input string window no 2*/
GtkWidget *menu_window_in;    /* input number window */
GtkWidget *menu_window_about; /* about xsystem35 window */
GtkWidget *menu_window_msgbox;/* message box window */

void
on_window3_destroy                     (GtkObject       *object,
                                        gpointer         user_data)
{

}

/* message skip on */
void
on_item2_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	set_skipMode(TRUE);
	gtk_menu_popdown(GTK_MENU(menu_window_popup));
}

/* message skip off */
void
on_item3_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	set_skipMode(FALSE);
	gtk_menu_popdown(GTK_MENU(menu_window_popup));
}

/* mouse auto move on */
void
on_item5_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	nact->sys_mouse_movesw = 2;
	gtk_menu_popdown(GTK_MENU(menu_window_popup));
}


/* mouse auto move off */
void
on_item6_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	nact->sys_mouse_movesw = 0;
	gtk_menu_popdown(GTK_MENU(menu_window_popup));
}


void
on_item7_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_menu_popdown(GTK_MENU(menu_window_popup));
	s39ini_winopen();
}

/* PCM off */
void
on_item9_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	// mus_pcm_set_state(FALSE);
	gtk_menu_popdown(GTK_MENU(menu_window_popup));
}


/* CD on */
void
on_item11_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	// mus_cdrom_set_state(TRUE);
	gtk_menu_popdown(GTK_MENU(menu_window_popup));
}


/* CD off */
void
on_item12_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	// mus_cdrom_set_state(FALSE);
	gtk_menu_popdown(GTK_MENU(menu_window_popup));
}


/* MIDI on */
void
on_item14_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	// mus_midi_set_state(TRUE);
	gtk_menu_popdown(GTK_MENU(menu_window_popup));
}

/* MIDI off */
void
on_item15_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	// mus_midi_set_state(FALSE);
	gtk_menu_popdown(GTK_MENU(menu_window_popup));
}

/* PopupMenu about */
void
on_item16_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	nact->popupmenu_opened = TRUE;
	gtk_widget_show(menu_window_about);
}

/* PopupMenu exit */
void
on_item17_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	nact->popupmenu_opened = TRUE;
	gtk_widget_show(menu_window_exit);
}

/* input number window OK button */
void
on_button1_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	menu_ok_input = TRUE;
	gtk_widget_hide(menu_window_in);
	gtk_main_quit();
}


/* input number window cancel button */
void
on_button2_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	menu_ok_input = FALSE;
	gtk_widget_hide(menu_window_in);
	gtk_main_quit();
}


/* input string window cancel button */
void
on_button4_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	menu_ok_input = FALSE;
	gtk_widget_hide(menu_window_is);
	gtk_main_quit();
}


/* input string window OK button */
void
on_button3_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	menu_textentry_string = gtk_entry_get_text(GTK_ENTRY(menu_textentry));
	menu_ok_input = TRUE;
	gtk_widget_hide(menu_window_is);
	gtk_main_quit();
}

/* Exit Yes */
void
on_button5_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	menu_ok_exit = TRUE;
	nact->is_quit = TRUE;
	nact->popupmenu_opened = FALSE;
}


/* Exit No */
void
on_button6_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	nact->popupmenu_opened = FALSE;
	menu_ok_exit = FALSE;
	gtk_widget_hide(menu_window_exit);
}

/* About OK */
void
on_button11_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide(menu_window_about);
	nact->popupmenu_opened = FALSE;
}

/* menu canceld */
void
on_menu1_deactivate                    (GtkMenuShell    *menushell,
                                        gpointer         user_data)
{
	gtk_menu_popdown(GTK_MENU(menu_window_popup));
	nact->popupmenu_opened = FALSE;
}

/* get enter in text entry */
void
on_entry1_activate                     (GtkEditable     *editable,
                                        gpointer         user_data)
{
	menu_textentry_string = gtk_entry_get_text(GTK_ENTRY(editable));
	menu_ok_input = TRUE;
	gtk_widget_hide(menu_window_is);
	gtk_main_quit();
}

/* get enter in spin button */
void
on_spinbutton1_activate                (GtkEditable     *editable,
                                        gpointer         user_data)
{
	GtkAdjustment *adj;
	adj = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(menu_spinbutton));
	if (adj->value >= adj->lower && adj->value <= adj->upper) {
		menu_ok_input = TRUE;
		gtk_widget_hide(menu_window_in);
		gtk_main_quit();
	}
}

/* get enter in text entry */
void
on_entry2_activate                     (GtkEditable     *editable,
                                        gpointer         user_data)
{
	menu_textentry_string = gtk_entry_get_text(GTK_ENTRY(editable));
	menu_ok_input = TRUE;
	gtk_widget_hide(menu_window_is2);
	gtk_main_quit();
}

void
on_button16_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide(menu_window_msgbox);
	gtk_main_quit();
}
