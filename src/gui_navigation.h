/*
** Copyright (©) 2003-2008 Teus Benschop.
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**  
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**  
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc.,  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**  
*/


#ifndef INCLUDED_GUI_NAVIGATION_H
#define INCLUDED_GUI_NAVIGATION_H


#include "libraries.h"
#include <glib.h>
#include <gtk/gtk.h>
#include "reference.h"
#include "track.h"


class GuiNavigation
{
public:
  GuiNavigation (int dummy);
  ~GuiNavigation ();
  void build (GtkWidget * toolbar, GtkWidget * menu_next_reference, GtkWidget * menu_previous_reference);
  void sensitive (bool sensitive);
  void set_project (const ustring& value, bool force);
  void clamp (Reference& reference);
  void display (const Reference& ref);
  void nextbook ();
  void previousbook ();
  void nextchapter ();
  void previouschapter ();
  void nextverse ();
  void previousverse ();
  GtkWidget * reference_signal_immediate;
  GtkWidget * reference_signal_delayed;
  GtkWidget * reference_signal_late;
  Reference reference;
  void display_delayed (const ustring& verse);
  void on_back ();
  void on_forward ();
private:
  GtkWidget *menu_back;
  GtkWidget *menu_forward;
  GtkWidget *button_back;
  GtkWidget *image1;
  GtkWidget *button_forward;
  GtkWidget *image2;
  GtkWidget *combo_book;
  GtkObject *spinbutton_book_adj;
  GtkWidget *spinbutton_book;
  GtkWidget *combo_chapter;
  GtkObject *spinbutton_chapter_adj;
  GtkWidget *spinbutton_chapter;
  GtkWidget *combo_verse;
  GtkObject *spinbutton_verse_adj;
  GtkWidget *spinbutton_verse;

  static void on_menu_back_activate (GtkMenuItem *menuitem, gpointer user_data);
  static void on_menu_forward_activate (GtkMenuItem *menuitem, gpointer user_data);
  static void on_button_back_clicked (GtkButton *button, gpointer user_data);
  static void on_button_forward_clicked (GtkButton *button, gpointer user_data);
  static void on_combo_book_changed (GtkComboBox *combobox, gpointer user_data);
  static void on_combo_chapter_changed (GtkComboBox *combobox, gpointer user_data);
  static void on_combo_verse_changed (GtkComboBox *combobox, gpointer user_data);
  static void on_spinbutton_book_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
  static void on_spinbutton_chapter_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
  static void on_spinbutton_verse_value_changed (GtkSpinButton *spinbutton, gpointer user_data);
  void on_combo_book ();
  void on_combo_chapter ();
  void on_combo_verse ();
  void on_spinbutton_book ();
  void on_spinbutton_chapter ();
  void on_spinbutton_verse ();

  bool reference_exists (Reference& reference);
  void load_books ();
  void set_book (unsigned int book);
  void load_chapters (unsigned int book);
  void set_chapter (unsigned int chapter);
  void load_verses (unsigned int book, unsigned int chapter);
  void set_verse (const ustring& verse);
  void signal (bool track = true);
  static bool signal_delayer (gpointer user_data);
  void signal_delayed ();
  static bool signal_later (gpointer user_data);
  void signal_late ();
  static bool signal_track (gpointer user_data);
  void signal_tracking ();
  void crossboundariesverse (bool forward);
  void crossboundarieschapter (bool forward);
  void tracker_sensitivity ();

  ustring project;
  ustring language;
  bool settingcombos;
  int spinbutton_book_previous_value;
  int spinbutton_chapter_previous_value;
  int spinbutton_verse_previous_value;
  guint delayer_event_id;
  guint later_event_id;
  guint track_event_id;
  Track track;
  
  guint goto_reference_event_id;
  static bool on_goto_reference_timeout (gpointer user_data);
  void on_goto_reference ();
  ustring goto_reference_verse;
};



#endif
