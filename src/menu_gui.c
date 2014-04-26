/*
 * menu_gui.c  menu gui
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
/* $Id: menu_gui.c,v 1.12 2003/04/25 17:23:55 chikama Exp $ */

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "portab.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#ifndef GTK_CHECK_VERSION
#define GTK_CHECK_VERSION(major,minor,micro)    \
(GTK_MAJOR_VERSION > (major) || \
 (GTK_MAJOR_VERSION == (major) && GTK_MINOR_VERSION > (minor)) || \
 (GTK_MAJOR_VERSION == (major) && GTK_MINOR_VERSION == (minor) && \
  GTK_MICRO_VERSION >= (micro)))
#endif /* !GTK_CHECK_VERSION */

#if GTK_CHECK_VERSION (1,2,0)
#define GTKV12 
#if GTK_CHECK_VERSION(1,3,0)
#define GTKV13
#endif
#else
#define GTKV10
#endif

#ifdef GTKV13
#define GTK_WINDOW_DIALOG GTK_WINDOW_TOPLEVEL
#endif

#include "menu_callback.h"
#include "menu_gui.h"

GtkWidget*
create_window1 (void)
{
  GtkWidget *window1;
  GtkWidget *hbox1;
  GtkWidget *vbox1;
  GtkObject *spinbutton1_adj;
  GtkWidget *spinbutton1;
  GtkWidget *table1;
  GtkWidget *label3;
  GtkWidget *label9;
  GtkWidget *label10;
  GtkWidget *label11;
  GtkWidget *label12;
  GtkWidget *label13;
  GtkWidget *vbox2;
  GtkWidget *button1;
  GtkWidget *button2;
  GtkWidget *label4;

  window1 = gtk_window_new (GTK_WINDOW_DIALOG);
  gtk_object_set_data (GTK_OBJECT (window1), "window1", window1);
  gtk_window_set_title (GTK_WINDOW (window1), _("InputNumber"));
  gtk_window_set_policy (GTK_WINDOW (window1), FALSE, FALSE, FALSE);
  gtk_window_position (GTK_WINDOW (window1), GTK_WIN_POS_MOUSE);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hbox1", hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (window1), hbox1);

  vbox1 = gtk_vbox_new (FALSE, 10);
  gtk_widget_ref (vbox1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "vbox1", vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 0);
  gtk_widget_set_usize (vbox1, 128, -2);
#ifdef GTKV12
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 10);
#else
  gtk_container_border_width (GTK_CONTAINER (vbox1), 10);
#endif

  spinbutton1_adj = gtk_adjustment_new (100, 0, 100, 1, 1, 100);
  menu_spinbutton = spinbutton1 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton1_adj), 1, 0);
  gtk_widget_ref (spinbutton1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "spinbutton1", spinbutton1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (spinbutton1);
  gtk_box_pack_start (GTK_BOX (vbox1), spinbutton1, FALSE, FALSE, 0);

  table1 = gtk_table_new (3, 2, FALSE);
  gtk_widget_ref (table1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "table1", table1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox1), table1, FALSE, FALSE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 3);

  label3 = gtk_label_new (_("default"));
  gtk_widget_ref (label3);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label3", label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label3);
  gtk_table_attach (GTK_TABLE (table1), label3, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);

  label9 = gtk_label_new (_("min"));
  gtk_widget_ref (label9);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label9", label9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label9);
  gtk_table_attach (GTK_TABLE (table1), label9, 0, 1, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label9), GTK_JUSTIFY_LEFT);

  label10 = gtk_label_new (_("max"));
  gtk_widget_ref (label10);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label10", label10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label10);
  gtk_table_attach (GTK_TABLE (table1), label10, 0, 1, 2, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label10), GTK_JUSTIFY_LEFT);

  menu_label_inputnum_def = label11 = gtk_label_new (_("0"));
  gtk_widget_ref (label11);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label11", label11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label11);
  gtk_table_attach (GTK_TABLE (table1), label11, 1, 2, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label11), GTK_JUSTIFY_LEFT);

  menu_label_inputnum_min = label12 = gtk_label_new (_("0"));
  gtk_widget_ref (label12);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label12", label12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label12);
  gtk_table_attach (GTK_TABLE (table1), label12, 1, 2, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label12), GTK_JUSTIFY_LEFT);

  menu_label_inputnum_max = label13 = gtk_label_new (_("0"));
  gtk_widget_ref (label13);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label13", label13,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label13);
  gtk_table_attach (GTK_TABLE (table1), label13, 1, 2, 2, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label13), GTK_JUSTIFY_LEFT);

  vbox2 = gtk_vbox_new (TRUE, 0);
  gtk_widget_ref (vbox2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "vbox2", vbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox2, FALSE, TRUE, 0);
#ifdef GTKV12
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 10);
#else
  gtk_container_border_width (GTK_CONTAINER (vbox2), 10);
#endif
  button1 = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (button1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "button1", button1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button1);
  gtk_box_pack_start (GTK_BOX (vbox2), button1, FALSE, FALSE, 0);

  button2 = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (button2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "button2", button2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button2);
  gtk_box_pack_start (GTK_BOX (vbox2), button2, FALSE, FALSE, 0);

  label4 = gtk_label_new ("");
  gtk_widget_ref (label4);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label4", label4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (vbox2), label4, FALSE, FALSE, 0);

  gtk_signal_connect (GTK_OBJECT (spinbutton1), "activate",
                      GTK_SIGNAL_FUNC (on_spinbutton1_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (button1), "clicked",
                      GTK_SIGNAL_FUNC (on_button1_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (button2), "clicked",
                      GTK_SIGNAL_FUNC (on_button2_clicked),
                      NULL);

  return window1;
}

GtkWidget*
create_window2 (void)
{
  GtkWidget *window2;
  GtkWidget *vbox3;
  GtkWidget *label5;
  GtkWidget *entry1;
  GtkWidget *label6;
  GtkWidget *label7;
  GtkWidget *hbox2;
  GtkWidget *button4;
  GtkWidget *button3;

  window2 = gtk_window_new (GTK_WINDOW_DIALOG);
  gtk_object_set_data (GTK_OBJECT (window2), "window2", window2);
#ifdef GTKV12
  gtk_container_set_border_width (GTK_CONTAINER (window2), 10);
#else
  gtk_container_border_width (GTK_CONTAINER (window2), 10);
#endif
  gtk_window_set_title (GTK_WINDOW (window2), _("InputString"));
  gtk_window_set_policy (GTK_WINDOW (window2), FALSE, FALSE, FALSE);
  gtk_window_position (GTK_WINDOW (window2), GTK_WIN_POS_MOUSE);

  vbox3 = gtk_vbox_new (TRUE, 0);
  gtk_widget_ref (vbox3);
  gtk_object_set_data_full (GTK_OBJECT (window2), "vbox3", vbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox3);
  gtk_container_add (GTK_CONTAINER (window2), vbox3);

  menu_label_inputstring_title = label5 = gtk_label_new (_("title"));
  gtk_widget_ref (label5);
  gtk_object_set_data_full (GTK_OBJECT (window2), "label5", label5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label5);
  gtk_box_pack_start (GTK_BOX (vbox3), label5, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label5), 0.00, 0.5);

  menu_textentry = entry1 = gtk_entry_new ();
  gtk_widget_ref (entry1);
  gtk_object_set_data_full (GTK_OBJECT (window2), "entry1", entry1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry1);
  gtk_box_pack_start (GTK_BOX (vbox3), entry1, FALSE, FALSE, 0);

  menu_label_inputstring_maxchar = label6 = gtk_label_new (_("MAX charater"));
  gtk_widget_ref (label6);
  gtk_object_set_data_full (GTK_OBJECT (window2), "label6", label6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label6);
  gtk_box_pack_start (GTK_BOX (vbox3), label6, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label6), 0.00, 0.5);

  label7 = gtk_label_new (_("Notice) HANKAKU is not available"));
  gtk_widget_ref (label7);
  gtk_object_set_data_full (GTK_OBJECT (window2), "label7", label7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label7);
  gtk_box_pack_start (GTK_BOX (vbox3), label7, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label7), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label7), 0.00, 0.5);

  hbox2 = gtk_hbox_new (FALSE, 5);
  gtk_widget_ref (hbox2);
  gtk_object_set_data_full (GTK_OBJECT (window2), "hbox2", hbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox2, TRUE, TRUE, 0);

  button4 = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (button4);
  gtk_object_set_data_full (GTK_OBJECT (window2), "button4", button4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button4);
  gtk_box_pack_end (GTK_BOX (hbox2), button4, FALSE, FALSE, 0);

  button3 = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (button3);
  gtk_object_set_data_full (GTK_OBJECT (window2), "button3", button3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button3);
  gtk_box_pack_end (GTK_BOX (hbox2), button3, FALSE, FALSE, 0);

  gtk_signal_connect (GTK_OBJECT (button4), "clicked",
                      GTK_SIGNAL_FUNC (on_button4_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (button3), "clicked",
                      GTK_SIGNAL_FUNC (on_button3_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (entry1), "activate",
                      GTK_SIGNAL_FUNC (on_entry1_activate),
                      NULL);

  return window2;
}

GtkWidget*
create_window3 (void)
{
  GtkWidget *window3;
  GtkWidget *vbox4;
  GtkWidget *label8;
  GtkWidget *hbox3;
  GtkWidget *button5;
  GtkWidget *button6;

  window3 = gtk_window_new (GTK_WINDOW_DIALOG);
  gtk_object_set_data (GTK_OBJECT (window3), "window3", window3);
#ifdef GTKV12
  gtk_container_set_border_width (GTK_CONTAINER (window3), 10);
#else
  gtk_container_border_width (GTK_CONTAINER (window3), 10);
#endif
  gtk_window_set_title (GTK_WINDOW (window3), _("Exit"));
  gtk_window_set_policy (GTK_WINDOW (window3), FALSE, FALSE, FALSE);
  gtk_window_position (GTK_WINDOW (window3), GTK_WIN_POS_MOUSE);

  vbox4 = gtk_vbox_new (TRUE, 10);
  gtk_widget_ref (vbox4);
  gtk_object_set_data_full (GTK_OBJECT (window3), "vbox4", vbox4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox4);
  gtk_container_add (GTK_CONTAINER (window3), vbox4);
#ifdef GTKV12
  gtk_container_set_border_width (GTK_CONTAINER (vbox4), 5);
#else
  gtk_container_border_width (GTK_CONTAINER (vbox4), 5);
#endif
  label8 = gtk_label_new (_("Exit System35 ?"));
  gtk_widget_ref (label8);
  gtk_object_set_data_full (GTK_OBJECT (window3), "label8", label8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label8);
  gtk_box_pack_start (GTK_BOX (vbox4), label8, FALSE, FALSE, 0);

  hbox3 = gtk_hbox_new (TRUE, 10);
  gtk_widget_ref (hbox3);
  gtk_object_set_data_full (GTK_OBJECT (window3), "hbox3", hbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox4), hbox3, TRUE, TRUE, 0);

  button5 = gtk_button_new_with_label (_("Yes"));
  gtk_widget_ref (button5);
  gtk_object_set_data_full (GTK_OBJECT (window3), "button5", button5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button5);
  gtk_box_pack_start (GTK_BOX (hbox3), button5, FALSE, TRUE, 0);

  button6 = gtk_button_new_with_label (_("No"));
  gtk_widget_ref (button6);
  gtk_object_set_data_full (GTK_OBJECT (window3), "button6", button6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button6);
  gtk_box_pack_start (GTK_BOX (hbox3), button6, FALSE, TRUE, 0);

  gtk_signal_connect (GTK_OBJECT (window3), "destroy",
                      GTK_SIGNAL_FUNC (on_window3_destroy),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (button5), "clicked",
                      GTK_SIGNAL_FUNC (on_button5_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (button6), "clicked",
                      GTK_SIGNAL_FUNC (on_button6_clicked),
                      NULL);

  gtk_widget_grab_focus (button5);
  return window3;
}

GtkWidget*
create_window4 (void)
{
  GtkWidget *window4;
  GtkWidget *fixed1;
  GtkWidget *notebook1;
  GtkWidget *label26;
  GtkWidget *label23;
  GtkWidget *empty_notebook_page;
  GtkWidget *label24;
  GtkWidget *label25;
  GtkWidget *button11;

  window4 = gtk_window_new (GTK_WINDOW_DIALOG);
  gtk_object_set_data (GTK_OBJECT (window4), "window4", window4);
  gtk_window_set_title (GTK_WINDOW (window4), _("About"));
  gtk_window_position (GTK_WINDOW (window4), GTK_WIN_POS_MOUSE);

  fixed1 = gtk_fixed_new ();
  gtk_widget_ref (fixed1);
  gtk_object_set_data_full (GTK_OBJECT (window4), "fixed1", fixed1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fixed1);
  gtk_container_add (GTK_CONTAINER (window4), fixed1);

  notebook1 = gtk_notebook_new ();
  gtk_widget_ref (notebook1);
  gtk_object_set_data_full (GTK_OBJECT (window4), "notebook1", notebook1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (notebook1);
  gtk_fixed_put (GTK_FIXED (fixed1), notebook1, 0, 0);
  gtk_widget_set_uposition (notebook1, 0, 0);
  gtk_widget_set_usize (notebook1, 248, 104);

  label26 = gtk_label_new ("System 3.5 scenario decoder\nfor X\nVersion "VERSION" [proj. RainyMoon]");
  gtk_widget_ref (label26);
  gtk_object_set_data_full (GTK_OBJECT (window4), "label26", label26,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label26);
  gtk_container_add (GTK_CONTAINER (notebook1), label26);

  label23 = gtk_label_new (_("Information1"));
  gtk_widget_ref (label23);
  gtk_object_set_data_full (GTK_OBJECT (window4), "label23", label23,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label23);
#ifdef GTKV12
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label23);
#endif
  empty_notebook_page = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (empty_notebook_page);
  gtk_container_add (GTK_CONTAINER (notebook1), empty_notebook_page);

  label24 = gtk_label_new (_("Information2"));
  gtk_widget_ref (label24);
  gtk_object_set_data_full (GTK_OBJECT (window4), "label24", label24,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label24);
#ifdef GTKV12
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label24);
#endif
  empty_notebook_page = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (empty_notebook_page);
  gtk_container_add (GTK_CONTAINER (notebook1), empty_notebook_page);

  label25 = gtk_label_new (_("Information3"));
  gtk_widget_ref (label25);
  gtk_object_set_data_full (GTK_OBJECT (window4), "label25", label25,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label25);
#ifdef GTKV12
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), label25);
#endif
  button11 = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (button11);
  gtk_object_set_data_full (GTK_OBJECT (window4), "button11", button11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button11);
  gtk_fixed_put (GTK_FIXED (fixed1), button11, 184, 104);
  gtk_widget_set_uposition (button11, 184, 104);
  gtk_widget_set_usize (button11, 64, 24);

  gtk_signal_connect (GTK_OBJECT (button11), "clicked",
                      GTK_SIGNAL_FUNC (on_button11_clicked),
                      NULL);
  return window4;
}

GtkWidget*
create_menu1 (void)
{
  GtkWidget *menu1;
#ifdef GTKV12
  GtkAccelGroup *menu1_accels;
  GtkAccelGroup *item1_menu_accels;
  GtkAccelGroup *item4_menu_accels;
  GtkAccelGroup *item7_menu_accels;
  GtkAccelGroup *item10_menu_accels;
  GtkAccelGroup *item13_menu_accels;
#endif
  GtkWidget *item1;
  GtkWidget *item1_menu;
  GSList *msg_group = NULL;
  GtkWidget *item2;
  GtkWidget *item3;
  GtkWidget *__________1;
  GtkWidget *item4;
  GtkWidget *item4_menu;
  GSList *mouse_group = NULL;
  GtkWidget *item5;
  GtkWidget *item6;
  GtkWidget *__________2;
  GtkWidget *item7;
  GtkWidget *item7_menu;
  GSList *pcm_group = NULL;
  GtkWidget *item8;
  GtkWidget *item9;
  GtkWidget *item10;
  GtkWidget *item10_menu;
  GSList *cdrom_group = NULL;
  GtkWidget *item11;
  GtkWidget *item12;
  GtkWidget *item13;
  GtkWidget *item13_menu;
  GSList *midi_group = NULL;
  GtkWidget *item14;
  GtkWidget *item15;
  GtkWidget *__________3;
  GtkWidget *item16;
  GtkWidget *__________4;
  GtkWidget *item17;

  menu1 = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (menu1), "menu1", menu1);
#ifdef GTKV12
#ifdef GTKV13
#else
  menu1_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (menu1));
#endif
#endif
  item1 = gtk_menu_item_new_with_label (_("MessageSkip"));
  gtk_widget_ref (item1);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item1", item1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item1);
  gtk_container_add (GTK_CONTAINER (menu1), item1);

  item1_menu = gtk_menu_new ();
  gtk_widget_ref (item1_menu);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item1_menu", item1_menu,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item1), item1_menu);
#ifdef GTKV12
#ifdef GTKV13
#else
  item1_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (item1_menu));
#endif
#endif
  menu_item_msgskip_on = item2 = gtk_radio_menu_item_new_with_label (msg_group, _("ON"));
  msg_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item2));
  gtk_widget_ref (item2);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item2", item2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item2);
  gtk_container_add (GTK_CONTAINER (item1_menu), item2);

  menu_item_msgskip_off = item3 = gtk_radio_menu_item_new_with_label (msg_group, _("OFF"));
  msg_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item3));
  gtk_widget_ref (item3);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item3", item3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item3);
  gtk_container_add (GTK_CONTAINER (item1_menu), item3);

  __________1 = gtk_menu_item_new ();
  gtk_widget_ref (__________1);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "__________1", __________1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (__________1);
  gtk_container_add (GTK_CONTAINER (menu1), __________1);
  gtk_widget_set_sensitive (__________1, FALSE);

  item4 = gtk_menu_item_new_with_label (_("MouseAutoMove"));
  gtk_widget_ref (item4);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item4", item4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item4);
  gtk_container_add (GTK_CONTAINER (menu1), item4);

  item4_menu = gtk_menu_new ();
  gtk_widget_ref (item4_menu);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item4_menu", item4_menu,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item4), item4_menu);
#ifdef GTKV12
#ifdef GTKV13
#else
  item4_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (item4_menu));
#endif
#endif
  item5 = gtk_radio_menu_item_new_with_label (mouse_group, _("ON"));
  mouse_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item5));
  gtk_widget_ref (item5);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item5", item5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item5);
  gtk_container_add (GTK_CONTAINER (item4_menu), item5);

  item6 = gtk_radio_menu_item_new_with_label (mouse_group, _("OFF"));
  mouse_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item6));
  gtk_widget_ref (item6);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item6", item6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item6);
  gtk_container_add (GTK_CONTAINER (item4_menu), item6);

  __________2 = gtk_menu_item_new ();
  gtk_widget_ref (__________2);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "__________2", __________2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (__________2);
  gtk_container_add (GTK_CONTAINER (menu1), __________2);
  gtk_widget_set_sensitive (__________2, FALSE);

  item7 = gtk_menu_item_new_with_label (_("VolumeValance"));
  gtk_widget_ref (item7);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item7", item7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item7);
  gtk_container_add (GTK_CONTAINER (menu1), item7);

#if 0
  menu_item_pcm = item7 = gtk_menu_item_new_with_label (_("PCM-audio"));
  gtk_widget_ref (item7);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item7", item7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item7);
  gtk_container_add (GTK_CONTAINER (menu1), item7);

  item7_menu = gtk_menu_new ();
  gtk_widget_ref (item7_menu);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item7_menu", item7_menu,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item7), item7_menu);
#ifdef GTKV12
#ifdef GTKV13
#else
  item7_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (item7_menu));
#endif
#endif
  item8 = gtk_radio_menu_item_new_with_label (pcm_group, _("ON"));
  pcm_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item8));
  gtk_widget_ref (item8);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item8", item8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item8);
  gtk_container_add (GTK_CONTAINER (item7_menu), item8);

  item9 = gtk_radio_menu_item_new_with_label (pcm_group, _("OFF"));
  pcm_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item9));
  gtk_widget_ref (item9);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item9", item9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item9);
  gtk_container_add (GTK_CONTAINER (item7_menu), item9);

  menu_item_cdrom = item10 = gtk_menu_item_new_with_label (_("CDROM-audio"));
  gtk_widget_ref (item10);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item10", item10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item10);
  gtk_container_add (GTK_CONTAINER (menu1), item10);

  item10_menu = gtk_menu_new ();
  gtk_widget_ref (item10_menu);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item10_menu", item10_menu,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item10), item10_menu);
#ifdef GTKV12
#ifdef GTKV13
#else
  item10_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (item10_menu));
#endif
#endif
  item11 = gtk_radio_menu_item_new_with_label (cdrom_group, _("ON"));
  cdrom_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item11));
  gtk_widget_ref (item11);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item11", item11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item11);
  gtk_container_add (GTK_CONTAINER (item10_menu), item11);

  item12 = gtk_radio_menu_item_new_with_label (cdrom_group, _("OFF"));
  cdrom_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item12));
  gtk_widget_ref (item12);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item12", item12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item12);
  gtk_container_add (GTK_CONTAINER (item10_menu), item12);

  menu_item_midi = item13 = gtk_menu_item_new_with_label (_("MIDI-audio"));
  gtk_widget_ref (item13);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item13", item13,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item13);
  gtk_container_add (GTK_CONTAINER (menu1), item13);

  item13_menu = gtk_menu_new ();
  gtk_widget_ref (item13_menu);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item13_menu", item13_menu,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item13), item13_menu);
#ifdef GTKV12
#ifdef GTKV13
#else
  item13_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (item13_menu));
#endif
#endif
  item14 = gtk_radio_menu_item_new_with_label (midi_group, _("ON"));
  midi_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item14));
  gtk_widget_ref (item14);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item14", item14,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item14);
  gtk_container_add (GTK_CONTAINER (item13_menu), item14);

  item15 = gtk_radio_menu_item_new_with_label (midi_group, _("OFF"));
  midi_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item15));
  gtk_widget_ref (item15);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item15", item15,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item15);
  gtk_container_add (GTK_CONTAINER (item13_menu), item15);

  __________3 = gtk_menu_item_new ();
  gtk_widget_ref (__________3);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "__________3", __________3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (__________3);
  gtk_container_add (GTK_CONTAINER (menu1), __________3);
  gtk_widget_set_sensitive (__________3, FALSE);
#endif

  item16 = gtk_menu_item_new_with_label (_("About"));
  gtk_widget_ref (item16);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item16", item16,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item16);
  gtk_container_add (GTK_CONTAINER (menu1), item16);

  __________4 = gtk_menu_item_new ();
  gtk_widget_ref (__________4);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "__________4", __________4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (__________4);
  gtk_container_add (GTK_CONTAINER (menu1), __________4);
  gtk_widget_set_sensitive (__________4, FALSE);

  item17 = gtk_menu_item_new_with_label (_("Exit"));
  gtk_widget_ref (item17);
  gtk_object_set_data_full (GTK_OBJECT (menu1), "item17", item17,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item17);
  gtk_container_add (GTK_CONTAINER (menu1), item17);

  gtk_signal_connect (GTK_OBJECT (menu1), "deactivate",
                      GTK_SIGNAL_FUNC (on_menu1_deactivate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (item2), "toggled",
                      GTK_SIGNAL_FUNC (on_item2_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (item3), "toggled",
                      GTK_SIGNAL_FUNC (on_item3_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (item5), "activate",
                      GTK_SIGNAL_FUNC (on_item5_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (item6), "activate",
                      GTK_SIGNAL_FUNC (on_item6_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (item7), "activate",
                      GTK_SIGNAL_FUNC (on_item7_activate),
                      NULL);
#if 0
  gtk_signal_connect (GTK_OBJECT (item8), "activate",
                      GTK_SIGNAL_FUNC (on_item8_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (item9), "activate",
                      GTK_SIGNAL_FUNC (on_item9_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (item11), "activate",
                      GTK_SIGNAL_FUNC (on_item11_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (item12), "activate",
                      GTK_SIGNAL_FUNC (on_item12_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (item14), "activate",
                      GTK_SIGNAL_FUNC (on_item14_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (item15), "activate",
                      GTK_SIGNAL_FUNC (on_item15_activate),
                      NULL);
#endif
  gtk_signal_connect (GTK_OBJECT (item16), "activate",
                      GTK_SIGNAL_FUNC (on_item16_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (item17), "activate",
                      GTK_SIGNAL_FUNC (on_item17_activate),
                      NULL);

  return menu1;
}

GtkWidget*
create_window5 (void)
{
  GtkWidget *window5;
  GtkWidget *entry2;

  window5 = gtk_window_new (GTK_WINDOW_DIALOG);
  gtk_object_set_data (GTK_OBJECT (window5), "window5", window5);
  gtk_window_position (GTK_WINDOW (window5), GTK_WIN_POS_MOUSE);
  
  menu_textentry2 = entry2 = gtk_entry_new_with_max_length (16);
  gtk_widget_ref (entry2);
  gtk_object_set_data_full (GTK_OBJECT (window5), "entry2", entry2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry2);
  gtk_container_add (GTK_CONTAINER (window5), entry2);
  gtk_widget_set_usize (entry2, 100, 28);
  gtk_entry_set_text (GTK_ENTRY (entry2), _("default"));

  gtk_signal_connect (GTK_OBJECT (entry2), "activate",
                      GTK_SIGNAL_FUNC (on_entry2_activate),
                      NULL);

  return window5;
}

GtkWidget*
create_window6 (void)
{
  GtkWidget *window6;
  GtkWidget *vbox5;
  GtkWidget *label51;
  GtkWidget *button16;

  window6 = menu_window_msgbox = gtk_window_new (GTK_WINDOW_DIALOG);
  gtk_object_set_data (GTK_OBJECT (window6), "window6", window6);
  gtk_window_set_title (GTK_WINDOW (window6), _("MessageBox"));
  gtk_window_position (GTK_WINDOW (window6), GTK_WIN_POS_MOUSE);

  vbox5 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox5);
  gtk_object_set_data_full (GTK_OBJECT (window6), "vbox5", vbox5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox5);
  gtk_container_add (GTK_CONTAINER (window6), vbox5);

  label51 = menu_label_msgbox = gtk_label_new (_("Messge"));
  gtk_widget_ref (label51);
  gtk_object_set_data_full (GTK_OBJECT (window6), "label51", label51,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label51);
  gtk_box_pack_start (GTK_BOX (vbox5), label51, FALSE, FALSE, 0);

  button16 = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (button16);
  gtk_object_set_data_full (GTK_OBJECT (window6), "button16", button16,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button16);
  gtk_box_pack_start (GTK_BOX (vbox5), button16, FALSE, FALSE, 0);

  gtk_signal_connect (GTK_OBJECT (button16), "clicked",
                      GTK_SIGNAL_FUNC (on_button16_clicked),
                      NULL);

  return window6;
}
