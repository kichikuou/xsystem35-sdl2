/*
 * menu_callback.h  menu callback function
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
/* $Id: menu_callback.h,v 1.4 2001/04/04 21:55:39 chikama Exp $ */

#ifndef __MENU_CALLBACK__
#define __MENU_CALLBACK__

#include <gtk/gtk.h>

extern boolean menu_ok_exit;
extern boolean menu_ok_input;

extern GtkWidget *menu_item_pcm;
extern GtkWidget *menu_item_cdrom;
extern GtkWidget *menu_item_midi;
extern GtkWidget *menu_item_msgskip_on;
extern GtkWidget *menu_item_msgskip_off;

extern GtkWidget *menu_label_inputstring_title;
extern GtkWidget *menu_label_inputstring_maxchar;
extern GtkWidget *menu_label_inputnum_title;
extern GtkWidget *menu_label_inputnum_max;
extern GtkWidget *menu_label_inputnum_min;
extern GtkWidget *menu_label_inputnum_def;
extern GtkWidget *menu_label_msgbox;
extern GtkWidget *menu_spinbutton;
extern GtkWidget *menu_textentry;
extern GtkWidget *menu_textentry2;
extern gchar     *menu_textentry_string;

extern GtkWidget *menu_window_popup; /* popup menu */
extern GtkWidget *menu_window_exit;  /* exit window */
extern GtkWidget *menu_window_is;    /* input string window */
extern GtkWidget *menu_window_is2;   /* input string window no 2*/
extern GtkWidget *menu_window_in;    /* input number window */
extern GtkWidget *menu_window_about; /* about xsystem35 window */
extern GtkWidget *menu_window_msgbox;/* message box window */


void
on_window3_destroy                     (GtkObject       *object,
                                        gpointer         user_data);

void
on_item2_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item3_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item4_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item5_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item6_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item7_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item8_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item9_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item10_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item11_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item12_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item14_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item15_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item16_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item17_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_button1_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_button2_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_button4_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_button3_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_button5_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_button6_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_button11_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_menu1_deactivate                    (GtkMenuShell    *menushell,
                                        gpointer         user_data);

void
on_entry1_activate                     (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinbutton1_activate                (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_entry2_activate                     (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_button16_clicked                    (GtkButton       *button,
                                        gpointer         user_data);
#endif /* !__MENU_CALLBACK__ */
