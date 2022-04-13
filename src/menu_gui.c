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

#include "menu_callback.h"
#include "menu_gui.h"

GtkWidget*
create_window1 (void)
{
  GtkWidget *window1;
  GtkWidget *hbox1;
  GtkWidget *vbox1;
  GtkAdjustment *spinbutton1_adj;
  GtkWidget *spinbutton1;
  GtkWidget *grid1;
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

  window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_object_set_data(G_OBJECT(window1), "window1", window1);
  gtk_window_set_title (GTK_WINDOW (window1), _("InputNumber"));
  gtk_window_set_resizable(GTK_WINDOW (window1), FALSE);
  gtk_window_set_position(GTK_WINDOW (window1), GTK_WIN_POS_MOUSE);

  hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  g_object_ref(hbox1);
  g_object_set_data_full(G_OBJECT(window1), "hbox1", hbox1,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (window1), hbox1);

  vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  g_object_ref(vbox1);
  g_object_set_data_full(G_OBJECT(window1), "vbox1", vbox1,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 0);
  gtk_widget_set_size_request(vbox1, 128, -2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 10);

  spinbutton1_adj = gtk_adjustment_new (100, 0, 100, 1, 1, 100);
  menu_spinbutton = spinbutton1 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton1_adj), 1, 0);
  g_object_ref(spinbutton1);
  g_object_set_data_full(G_OBJECT(window1), "spinbutton1", spinbutton1,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (spinbutton1);
  gtk_box_pack_start (GTK_BOX (vbox1), spinbutton1, FALSE, FALSE, 0);

  grid1 = gtk_grid_new();
  g_object_ref(grid1);
  g_object_set_data_full(G_OBJECT(window1), "grid1", grid1,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (grid1);
  gtk_box_pack_start (GTK_BOX (vbox1), grid1, FALSE, FALSE, 0);
  gtk_grid_set_row_spacing(GTK_GRID(grid1), 5);
  gtk_grid_set_column_spacing(GTK_GRID(grid1), 3);

  label3 = gtk_label_new (_("default"));
  g_object_ref(label3);
  g_object_set_data_full(G_OBJECT(window1), "label3", label3,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label3);
  gtk_grid_attach(GTK_GRID(grid1), label3, 0, 0, 1, 1);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);

  label9 = gtk_label_new (_("min"));
  g_object_ref(label9);
  g_object_set_data_full(G_OBJECT(window1), "label9", label9,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label9);
  gtk_grid_attach(GTK_GRID(grid1), label9, 0, 1, 1, 1);
  gtk_label_set_justify (GTK_LABEL (label9), GTK_JUSTIFY_LEFT);

  label10 = gtk_label_new (_("max"));
  g_object_ref(label10);
  g_object_set_data_full(G_OBJECT(window1), "label10", label10,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label10);
  gtk_grid_attach(GTK_GRID(grid1), label10, 0, 2, 1, 1);
  gtk_label_set_justify (GTK_LABEL (label10), GTK_JUSTIFY_LEFT);

  menu_label_inputnum_def = label11 = gtk_label_new (_("0"));
  g_object_ref(label11);
  g_object_set_data_full(G_OBJECT(window1), "label11", label11,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label11);
  gtk_grid_attach(GTK_GRID(grid1), label11, 1, 0, 1, 1);
  gtk_label_set_justify (GTK_LABEL (label11), GTK_JUSTIFY_LEFT);

  menu_label_inputnum_min = label12 = gtk_label_new (_("0"));
  g_object_ref(label12);
  g_object_set_data_full(G_OBJECT(window1), "label12", label12,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label12);
  gtk_grid_attach(GTK_GRID(grid1), label12, 1, 1, 1, 1);
  gtk_label_set_justify (GTK_LABEL (label12), GTK_JUSTIFY_LEFT);

  menu_label_inputnum_max = label13 = gtk_label_new (_("0"));
  g_object_ref(label13);
  g_object_set_data_full(G_OBJECT(window1), "label13", label13,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label13);
  gtk_grid_attach(GTK_GRID(grid1), label13, 1, 2, 1, 1);
  gtk_label_set_justify (GTK_LABEL (label13), GTK_JUSTIFY_LEFT);

  vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_set_homogeneous(GTK_BOX(vbox2), TRUE);
  g_object_ref(vbox2);
  g_object_set_data_full(G_OBJECT(window1), "vbox2", vbox2,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox2, FALSE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 10);
  button1 = gtk_button_new_with_label (_("OK"));
  g_object_ref(button1);
  g_object_set_data_full(G_OBJECT(window1), "button1", button1,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (button1);
  gtk_box_pack_start (GTK_BOX (vbox2), button1, FALSE, FALSE, 0);

  button2 = gtk_button_new_with_label (_("Cancel"));
  g_object_ref(button2);
  g_object_set_data_full(G_OBJECT(window1), "button2", button2,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (button2);
  gtk_box_pack_start (GTK_BOX (vbox2), button2, FALSE, FALSE, 0);

  label4 = gtk_label_new ("");
  g_object_ref(label4);
  g_object_set_data_full(G_OBJECT(window1), "label4", label4,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (vbox2), label4, FALSE, FALSE, 0);

  g_signal_connect(spinbutton1, "activate",
                      G_CALLBACK(on_spinbutton1_activate),
                      NULL);
  g_signal_connect(button1, "clicked",
                      G_CALLBACK(on_button1_clicked),
                      NULL);
  g_signal_connect(button2, "clicked",
                      G_CALLBACK(on_button2_clicked),
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

  window2 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_object_set_data(G_OBJECT(window2), "window2", window2);
  gtk_container_set_border_width (GTK_CONTAINER (window2), 10);
  gtk_window_set_title (GTK_WINDOW (window2), _("InputString"));
  gtk_window_set_resizable(GTK_WINDOW (window2), FALSE);
  gtk_window_set_position(GTK_WINDOW (window2), GTK_WIN_POS_MOUSE);

  vbox3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_set_homogeneous(GTK_BOX(vbox3), TRUE);
  g_object_ref(vbox3);
  g_object_set_data_full(G_OBJECT(window2), "vbox3", vbox3,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (vbox3);
  gtk_container_add (GTK_CONTAINER (window2), vbox3);

  menu_label_inputstring_title = label5 = gtk_label_new (_("title"));
  g_object_ref(label5);
  g_object_set_data_full(G_OBJECT(window2), "label5", label5,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label5);
  gtk_box_pack_start (GTK_BOX (vbox3), label5, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign(GTK_LABEL(label5), 0.00);

  menu_textentry = entry1 = gtk_entry_new ();
  g_object_ref(entry1);
  g_object_set_data_full(G_OBJECT(window2), "entry1", entry1,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (entry1);
  gtk_box_pack_start (GTK_BOX (vbox3), entry1, FALSE, FALSE, 0);

  menu_label_inputstring_maxchar = label6 = gtk_label_new (_("MAX charater"));
  g_object_ref(label6);
  g_object_set_data_full(G_OBJECT(window2), "label6", label6,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label6);
  gtk_box_pack_start (GTK_BOX (vbox3), label6, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign(GTK_LABEL(label6), 0.00);

  label7 = gtk_label_new (_("Notice) HANKAKU is not available"));
  g_object_ref(label7);
  g_object_set_data_full(G_OBJECT(window2), "label7", label7,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label7);
  gtk_box_pack_start (GTK_BOX (vbox3), label7, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label7), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign(GTK_LABEL(label7), 0.00);

  hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  g_object_ref(hbox2);
  g_object_set_data_full(G_OBJECT(window2), "hbox2", hbox2,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox2, TRUE, TRUE, 0);

  button4 = gtk_button_new_with_label (_("Cancel"));
  g_object_ref(button4);
  g_object_set_data_full(G_OBJECT(window2), "button4", button4,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (button4);
  gtk_box_pack_end (GTK_BOX (hbox2), button4, FALSE, FALSE, 0);

  button3 = gtk_button_new_with_label (_("OK"));
  g_object_ref(button3);
  g_object_set_data_full(G_OBJECT(window2), "button3", button3,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (button3);
  gtk_box_pack_end (GTK_BOX (hbox2), button3, FALSE, FALSE, 0);

  g_signal_connect(button4, "clicked",
                      G_CALLBACK(on_button4_clicked),
                      NULL);
  g_signal_connect(button3, "clicked",
                      G_CALLBACK(on_button3_clicked),
                      NULL);
  g_signal_connect(entry1, "activate",
                      G_CALLBACK(on_entry1_activate),
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

  window3 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_object_set_data(G_OBJECT(window3), "window3", window3);
  gtk_container_set_border_width (GTK_CONTAINER (window3), 10);
  gtk_window_set_title (GTK_WINDOW (window3), _("Exit"));
  gtk_window_set_resizable(GTK_WINDOW (window3), FALSE);
  gtk_window_set_position(GTK_WINDOW (window3), GTK_WIN_POS_MOUSE);

  vbox4 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_box_set_homogeneous(GTK_BOX(vbox4), TRUE);
  g_object_ref(vbox4);
  g_object_set_data_full(G_OBJECT(window3), "vbox4", vbox4,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (vbox4);
  gtk_container_add (GTK_CONTAINER (window3), vbox4);
  gtk_container_set_border_width (GTK_CONTAINER (vbox4), 5);
  label8 = gtk_label_new (_("Exit System35 ?"));
  g_object_ref(label8);
  g_object_set_data_full(G_OBJECT(window3), "label8", label8,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label8);
  gtk_box_pack_start (GTK_BOX (vbox4), label8, FALSE, FALSE, 0);

  hbox3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_box_set_homogeneous(GTK_BOX(hbox3), TRUE);
  g_object_ref(hbox3);
  g_object_set_data_full(G_OBJECT(window3), "hbox3", hbox3,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox4), hbox3, TRUE, TRUE, 0);

  button5 = gtk_button_new_with_label (_("Yes"));
  g_object_ref(button5);
  g_object_set_data_full(G_OBJECT(window3), "button5", button5,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (button5);
  gtk_box_pack_start (GTK_BOX (hbox3), button5, FALSE, TRUE, 0);

  button6 = gtk_button_new_with_label (_("No"));
  g_object_ref(button6);
  g_object_set_data_full(G_OBJECT(window3), "button6", button6,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (button6);
  gtk_box_pack_start (GTK_BOX (hbox3), button6, FALSE, TRUE, 0);

  g_signal_connect(window3, "destroy",
                      G_CALLBACK(on_window3_destroy),
                      NULL);
  g_signal_connect(button5, "clicked",
                      G_CALLBACK(on_button5_clicked),
                      NULL);
  g_signal_connect(button6, "clicked",
                      G_CALLBACK(on_button6_clicked),
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

  window4 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_object_set_data(G_OBJECT(window4), "window4", window4);
  gtk_window_set_title (GTK_WINDOW (window4), _("About"));
  gtk_window_set_position(GTK_WINDOW (window4), GTK_WIN_POS_MOUSE);

  fixed1 = gtk_fixed_new ();
  g_object_ref(fixed1);
  g_object_set_data_full(G_OBJECT(window4), "fixed1", fixed1,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (fixed1);
  gtk_container_add (GTK_CONTAINER (window4), fixed1);

  notebook1 = gtk_notebook_new ();
  g_object_ref(notebook1);
  g_object_set_data_full(G_OBJECT(window4), "notebook1", notebook1,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (notebook1);
  gtk_fixed_put (GTK_FIXED (fixed1), notebook1, 0, 0);
  gtk_widget_set_size_request(notebook1, 248, 104);

  label26 = gtk_label_new ("System 3.5 scenario decoder\nfor X\nVersion "VERSION" [proj. RainyMoon]");
  g_object_ref(label26);
  g_object_set_data_full(G_OBJECT(window4), "label26", label26,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label26);
  gtk_container_add (GTK_CONTAINER (notebook1), label26);

  label23 = gtk_label_new (_("Information1"));
  g_object_ref(label23);
  g_object_set_data_full(G_OBJECT(window4), "label23", label23,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label23);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label23);
  empty_notebook_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_show (empty_notebook_page);
  gtk_container_add (GTK_CONTAINER (notebook1), empty_notebook_page);

  label24 = gtk_label_new (_("Information2"));
  g_object_ref(label24);
  g_object_set_data_full(G_OBJECT(window4), "label24", label24,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label24);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label24);
  empty_notebook_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_show (empty_notebook_page);
  gtk_container_add (GTK_CONTAINER (notebook1), empty_notebook_page);

  label25 = gtk_label_new (_("Information3"));
  g_object_ref(label25);
  g_object_set_data_full(G_OBJECT(window4), "label25", label25,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label25);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), label25);
  button11 = gtk_button_new_with_label (_("OK"));
  g_object_ref(button11);
  g_object_set_data_full(G_OBJECT(window4), "button11", button11,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (button11);
  gtk_fixed_put (GTK_FIXED (fixed1), button11, 184, 104);
  gtk_widget_set_size_request(button11, 64, 24);

  g_signal_connect(button11, "clicked",
                      G_CALLBACK(on_button11_clicked),
                      NULL);
  return window4;
}

GtkWidget*
create_menu1 (void)
{
  GtkWidget *menu1;
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
  GtkWidget *item16;
  GtkWidget *__________4;
  GtkWidget *item17;

  menu1 = gtk_menu_new ();
  g_object_set_data(G_OBJECT(menu1), "menu1", menu1);
  item1 = gtk_menu_item_new_with_label (_("MessageSkip"));
  g_object_ref(item1);
  g_object_set_data_full(G_OBJECT(menu1), "item1", item1,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (item1);
  gtk_container_add (GTK_CONTAINER (menu1), item1);

  item1_menu = gtk_menu_new ();
  g_object_ref(item1_menu);
  g_object_set_data_full(G_OBJECT(menu1), "item1_menu", item1_menu,
                            (GDestroyNotify)g_object_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item1), item1_menu);
  menu_item_msgskip_on = item2 = gtk_radio_menu_item_new_with_label (msg_group, _("ON"));
  msg_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM (item2));
  g_object_ref(item2);
  g_object_set_data_full(G_OBJECT(menu1), "item2", item2,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (item2);
  gtk_container_add (GTK_CONTAINER (item1_menu), item2);

  menu_item_msgskip_off = item3 = gtk_radio_menu_item_new_with_label (msg_group, _("OFF"));
  msg_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM (item3));
  g_object_ref(item3);
  g_object_set_data_full(G_OBJECT(menu1), "item3", item3,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (item3);
  gtk_container_add (GTK_CONTAINER (item1_menu), item3);

  __________1 = gtk_menu_item_new ();
  g_object_ref(__________1);
  g_object_set_data_full(G_OBJECT(menu1), "__________1", __________1,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (__________1);
  gtk_container_add (GTK_CONTAINER (menu1), __________1);
  gtk_widget_set_sensitive (__________1, FALSE);

  item4 = gtk_menu_item_new_with_label (_("MouseAutoMove"));
  g_object_ref(item4);
  g_object_set_data_full(G_OBJECT(menu1), "item4", item4,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (item4);
  gtk_container_add (GTK_CONTAINER (menu1), item4);

  item4_menu = gtk_menu_new ();
  g_object_ref(item4_menu);
  g_object_set_data_full(G_OBJECT(menu1), "item4_menu", item4_menu,
                            (GDestroyNotify)g_object_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item4), item4_menu);
  item5 = gtk_radio_menu_item_new_with_label (mouse_group, _("ON"));
  mouse_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM (item5));
  g_object_ref(item5);
  g_object_set_data_full(G_OBJECT(menu1), "item5", item5,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (item5);
  gtk_container_add (GTK_CONTAINER (item4_menu), item5);

  item6 = gtk_radio_menu_item_new_with_label (mouse_group, _("OFF"));
  mouse_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM (item6));
  g_object_ref(item6);
  g_object_set_data_full(G_OBJECT(menu1), "item6", item6,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (item6);
  gtk_container_add (GTK_CONTAINER (item4_menu), item6);

  __________2 = gtk_menu_item_new ();
  g_object_ref(__________2);
  g_object_set_data_full(G_OBJECT(menu1), "__________2", __________2,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (__________2);
  gtk_container_add (GTK_CONTAINER (menu1), __________2);
  gtk_widget_set_sensitive (__________2, FALSE);

  item7 = gtk_menu_item_new_with_label (_("VolumeValance"));
  g_object_ref(item7);
  g_object_set_data_full(G_OBJECT(menu1), "item7", item7,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (item7);
  gtk_container_add (GTK_CONTAINER (menu1), item7);

  item16 = gtk_menu_item_new_with_label (_("About"));
  g_object_ref(item16);
  g_object_set_data_full(G_OBJECT(menu1), "item16", item16,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (item16);
  gtk_container_add (GTK_CONTAINER (menu1), item16);

  __________4 = gtk_menu_item_new ();
  g_object_ref(__________4);
  g_object_set_data_full(G_OBJECT(menu1), "__________4", __________4,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (__________4);
  gtk_container_add (GTK_CONTAINER (menu1), __________4);
  gtk_widget_set_sensitive (__________4, FALSE);

  item17 = gtk_menu_item_new_with_label (_("Exit"));
  g_object_ref(item17);
  g_object_set_data_full(G_OBJECT(menu1), "item17", item17,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (item17);
  gtk_container_add (GTK_CONTAINER (menu1), item17);

  g_signal_connect(menu1, "deactivate",
                      G_CALLBACK(on_menu1_deactivate),
                      NULL);
  g_signal_connect(item2, "toggled",
                      G_CALLBACK(on_item2_activate),
                      NULL);
  g_signal_connect(item3, "toggled",
                      G_CALLBACK(on_item3_activate),
                      NULL);
  g_signal_connect(item5, "activate",
                      G_CALLBACK(on_item5_activate),
                      NULL);
  g_signal_connect(item6, "activate",
                      G_CALLBACK(on_item6_activate),
                      NULL);
  g_signal_connect(item7, "activate",
                      G_CALLBACK(on_item7_activate),
                      NULL);
  g_signal_connect(item16, "activate",
                      G_CALLBACK(on_item16_activate),
                      NULL);
  g_signal_connect(item17, "activate",
                      G_CALLBACK(on_item17_activate),
                      NULL);

  return menu1;
}

GtkWidget*
create_window5 (void)
{
  GtkWidget *window5;
  GtkWidget *entry2;

  window5 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_object_set_data(G_OBJECT(window5), "window5", window5);
  gtk_window_set_position(GTK_WINDOW (window5), GTK_WIN_POS_MOUSE);
  
  menu_textentry2 = entry2 = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entry2), 16);
  g_object_ref(entry2);
  g_object_set_data_full(G_OBJECT(window5), "entry2", entry2,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (entry2);
  gtk_container_add (GTK_CONTAINER (window5), entry2);
  gtk_widget_set_size_request(entry2, 100, 28);
  gtk_entry_set_text (GTK_ENTRY (entry2), _("default"));

  g_signal_connect(entry2, "activate",
                      G_CALLBACK(on_entry2_activate),
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

  window6 = menu_window_msgbox = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_object_set_data(G_OBJECT(window6), "window6", window6);
  gtk_window_set_title (GTK_WINDOW (window6), _("MessageBox"));
  gtk_window_set_position(GTK_WINDOW (window6), GTK_WIN_POS_MOUSE);

  vbox5 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  g_object_ref(vbox5);
  g_object_set_data_full(G_OBJECT(window6), "vbox5", vbox5,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (vbox5);
  gtk_container_add (GTK_CONTAINER (window6), vbox5);

  label51 = menu_label_msgbox = gtk_label_new (_("Messge"));
  g_object_ref(label51);
  g_object_set_data_full(G_OBJECT(window6), "label51", label51,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (label51);
  gtk_box_pack_start (GTK_BOX (vbox5), label51, FALSE, FALSE, 0);

  button16 = gtk_button_new_with_label (_("OK"));
  g_object_ref(button16);
  g_object_set_data_full(G_OBJECT(window6), "button16", button16,
                            (GDestroyNotify)g_object_unref);
  gtk_widget_show (button16);
  gtk_box_pack_start (GTK_BOX (vbox5), button16, FALSE, FALSE, 0);

  g_signal_connect(button16, "clicked",
                      G_CALLBACK(on_button16_clicked),
                      NULL);

  return window6;
}
