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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**  
*/


#include "libraries.h"
#include <glib.h>
#include "dialogresourceconverter2.h"
#include "help.h"
#include "gtkwrappers.h"
#include "utilities.h"
#include "progresswindow.h"
#include "unixwrappers.h"
#include "bible.h"
#include "books.h"
#include "html.h"
#include "gui.h"
#include "dialogreviewanchors.h"
#include "resource_utils.h"
#include "resource_conversion_utils.h"
#include "tiny_utilities.h"
#include "dialoglistview.h"
#include "gwrappers.h"
#include "directories.h"
#include "screen.h"
#include "htmlbrowser.h"
#include "roman.h"
#include "combobox.h"


ResourceConverter2Dialog::ResourceConverter2Dialog (const ustring& working_directory)
{
  // Initialize variables.
  workingdirectory = working_directory;
  resource_conversion_type = rctEnd;
  chapter_pattern_index = 0;
  verse_pattern_index = 0;
  anchors_written = false;
  table_attachment_offset = 0;
  table1 = NULL;
  
  // Shortcuts.
  Shortcuts shortcuts (0);

  // Dialog.  
  resourceconverterdialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (resourceconverterdialog), "Resource Converter");
  gtk_window_set_position (GTK_WINDOW (resourceconverterdialog), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_modal (GTK_WINDOW (resourceconverterdialog), TRUE);

  dialog_vbox1 = GTK_DIALOG (resourceconverterdialog)->vbox;
  gtk_widget_show (dialog_vbox1);

  label1 = gtk_label_new ("");
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), label1, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

  build_table_and_type (shortcuts);

  // Dialog action.
  dialog_action_area1 = GTK_DIALOG (resourceconverterdialog)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  new InDialogHelp (resourceconverterdialog, &shortcuts, "resource_converter");

  helpbutton = gtk_button_new_from_stock ("gtk-help");
  gtk_dialog_add_action_widget (GTK_DIALOG (resourceconverterdialog), helpbutton, GTK_RESPONSE_NONE);

  cancelbutton = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelbutton);
  gtk_dialog_add_action_widget (GTK_DIALOG (resourceconverterdialog), cancelbutton, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton, GTK_CAN_DEFAULT);
  shortcuts.stockbutton (cancelbutton);
  
  okbutton = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton);
  gtk_dialog_add_action_widget (GTK_DIALOG (resourceconverterdialog), okbutton, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton, GTK_CAN_DEFAULT);
  shortcuts.stockbutton (okbutton);
  
  gtk_widget_set_sensitive (okbutton, false);

  g_signal_connect ((gpointer) okbutton, "clicked", G_CALLBACK (on_okbutton_clicked), gpointer (this));

  gtk_widget_grab_focus (okbutton);
  gtk_widget_grab_default (okbutton);

  build_gui (shortcuts);
  shortcuts.process ();
  if (type_gui ()) {
    gui ();
    on_type_button (true);
  }
}


ResourceConverter2Dialog::~ResourceConverter2Dialog ()
{
  gtk_widget_destroy (resourceconverterdialog);
}


int ResourceConverter2Dialog::run ()
{
  return gtk_dialog_run (GTK_DIALOG (resourceconverterdialog));
}


void ResourceConverter2Dialog::build_table_and_type (Shortcuts& shortcuts)
{
  // Destroy anything old.
  if (table1) gtk_widget_destroy (table1);
    
  // The table that contains the widgets.
  table1 = gtk_table_new (6, 4, FALSE);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), table1, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 3);
  
  // Type of the resource.
  build_button (image_type_ok, label_type_ok, label_type_short, button_type, "Type", shortcuts, G_CALLBACK (on_type_button_clicked), label_type_long);
}


void ResourceConverter2Dialog::build_entry (GtkWidget *& image_ok, GtkWidget *& label_ok, GtkWidget *& label, gchar * label_text, GtkWidget *& entry, const ustring& entry_text, GCallback handler)
{
  GtkWidget *hseparator;
  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 0, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  table_attachment_offset++;

  image_ok = gtk_image_new_from_icon_name ("gtk-apply", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image_ok);
  gtk_table_attach (GTK_TABLE (table1), image_ok, 0, 1, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  label_ok = gtk_label_new ("");
  gtk_widget_show (label_ok);
  gtk_table_attach (GTK_TABLE (table1), label_ok, 1, 2, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_ok), 0, 0.5);

  label = gtk_label_new (label_text);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table1), label, 2, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  table_attachment_offset++;

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_table_attach (GTK_TABLE (table1), entry, 0, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  table_attachment_offset++;
  
  gtk_entry_set_text (GTK_ENTRY (entry), entry_text.c_str());

  g_signal_connect ((gpointer) entry, "changed", handler, gpointer (this));
}


void ResourceConverter2Dialog::build_button (GtkWidget *& image_ok, GtkWidget *& label_ok, GtkWidget *& label_short, GtkWidget *& button, gchar * button_text, Shortcuts& shortcuts, GCallback handler, GtkWidget *& label_long)
{
  GtkWidget *hseparator;
  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 0, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  table_attachment_offset++;

  image_ok = gtk_image_new_from_icon_name ("gtk-apply", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image_ok);
  gtk_table_attach (GTK_TABLE (table1), image_ok, 0, 1, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label_ok = gtk_label_new ("");
  gtk_widget_show (label_ok);
  gtk_table_attach (GTK_TABLE (table1), label_ok, 1, 2, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_ok), 0, 0.5);

  label_short = gtk_label_new ("");
  gtk_widget_show (label_short);
  gtk_table_attach (GTK_TABLE (table1), label_short, 2, 3, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_short), 0, 0.5);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_table_attach (GTK_TABLE (table1), button, 3, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  table_attachment_offset++;

  GtkWidget *alignment;
  alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (button), alignment);

  GtkWidget *hbox;
  hbox = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (alignment), hbox);

  GtkWidget *image_button_surface;
  image_button_surface = gtk_image_new_from_stock ("gtk-index", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image_button_surface);
  gtk_box_pack_start (GTK_BOX (hbox), image_button_surface, FALSE, FALSE, 0);

  GtkWidget *label_button_surface;
  label_button_surface = gtk_label_new_with_mnemonic (button_text);
  gtk_widget_show (label_button_surface);
  gtk_box_pack_start (GTK_BOX (hbox), label_button_surface, FALSE, FALSE, 0);

  shortcuts.label (label_button_surface);
  
  label_long = gtk_label_new ("");
  gtk_widget_show (label_long);
  gtk_table_attach (GTK_TABLE (table1), label_long, 0, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_long), 0, 0.5);
  table_attachment_offset++;

  g_signal_connect ((gpointer) button, "clicked", handler, gpointer (this));
}


void ResourceConverter2Dialog::build_checkbutton_button (GtkWidget *& image_ok, GtkWidget *& label_ok, 
                                                         GtkWidget *& checkbutton, gchar * checkbutton_text, GCallback checkbutton_handler, 
                                                         GtkWidget *& button, gchar * button_text, GCallback button_handler, 
                                                         Shortcuts& shortcuts, 
                                                         GtkWidget *& label)
{
  GtkWidget * hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 0, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  table_attachment_offset++;

  image_ok = gtk_image_new_from_icon_name ("gtk-apply", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image_ok);
  gtk_table_attach (GTK_TABLE (table1), image_ok, 0, 1, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  label_ok = gtk_label_new ("Done");
  gtk_widget_show (label_ok);
  gtk_table_attach (GTK_TABLE (table1), label_ok, 1, 2, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_ok), 0, 0.5);

  checkbutton = gtk_check_button_new_with_mnemonic (checkbutton_text);
  gtk_widget_show (checkbutton);
  gtk_table_attach (GTK_TABLE (table1), checkbutton, 2, 3, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  shortcuts.button (checkbutton);

  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_table_attach (GTK_TABLE (table1), button, 3, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  table_attachment_offset++;

  GtkWidget * alignment2 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (button), alignment2);

  GtkWidget * hbox2 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox2);
  gtk_container_add (GTK_CONTAINER (alignment2), hbox2);

  GtkWidget * image2 = gtk_image_new_from_stock ("gtk-index", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image2);
  gtk_box_pack_start (GTK_BOX (hbox2), image2, FALSE, FALSE, 0);

  GtkWidget * label2 = gtk_label_new_with_mnemonic (button_text);
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (hbox2), label2, FALSE, FALSE, 0);
  
  shortcuts.label (label2);

  label = gtk_label_new ("");
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table1), label, 0, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  table_attachment_offset++;

  g_signal_connect ((gpointer) checkbutton, "toggled", checkbutton_handler, gpointer (this));
  g_signal_connect ((gpointer) button, "clicked", button_handler, gpointer (this));
}


void ResourceConverter2Dialog::build_textview (GtkWidget *& image_ok, GtkWidget *& label_ok, GtkWidget *& label,
                                               GtkWidget *& textview, gchar * text, GCallback handler)
{
  GtkWidget *hseparator;
  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 0, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  table_attachment_offset++;

  image_ok = gtk_image_new_from_icon_name ("gtk-apply", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image_ok);
  gtk_table_attach (GTK_TABLE (table1), image_ok, 0, 1, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  label_ok = gtk_label_new ("Done");
  gtk_widget_show (label_ok);
  gtk_table_attach (GTK_TABLE (table1), label_ok, 1, 2, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_ok), 0, 0.5);

  label = gtk_label_new ("Title");
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table1), label, 2, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  table_attachment_offset++;
  
  GtkWidget *scrolledwindow;
  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_table_attach (GTK_TABLE (table1), scrolledwindow, 0, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);

  textview = gtk_text_view_new ();
  gtk_widget_show (textview);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), textview);
  table_attachment_offset++;

  if (text) {
    GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    gtk_text_buffer_set_text (buffer, text, -1);
  }
  
  g_signal_connect ((gpointer) textview, "key_press_event", handler, gpointer (this));
}


// Entry, combo, entry.
void ResourceConverter2Dialog::build_entry_combo_entry (GtkWidget *& image_ok, GtkWidget *& label_ok,
                                                        GtkWidget *& label,
                                                        GtkWidget *& label_entry_1, GtkWidget *& entry_1,
                                                        GtkWidget *& label_combo, GtkWidget *& combo,
                                                        GtkWidget *& label_entry_2, GtkWidget *& entry_2,
                                                        GCallback entry_handler, Shortcuts& shortcuts)
{
  GtkWidget *hseparator;
  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_table_attach (GTK_TABLE (table1), hseparator, 0, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  
  table_attachment_offset++;

  image_ok = gtk_image_new_from_icon_name ("gtk-apply", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image_ok);
  gtk_table_attach (GTK_TABLE (table1), image_ok, 0, 1, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  label_ok = gtk_label_new ("Done");
  gtk_widget_show (label_ok);
  gtk_table_attach (GTK_TABLE (table1), label_ok, 1, 2, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_ok), 0, 0.5);

  label = gtk_label_new ("");
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table1), label, 2, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  
  table_attachment_offset++;

  GtkWidget *hbox;
  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (hbox);
  gtk_table_attach (GTK_TABLE (table1), hbox, 0, 4, table_attachment_offset, table_attachment_offset + 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
                    
  table_attachment_offset++;                    

  label_entry_1 = gtk_label_new ("");
  gtk_widget_show (label_entry_1);
  gtk_box_pack_start (GTK_BOX (hbox), label_entry_1, FALSE, FALSE, 0);
  
  entry_1 = gtk_entry_new ();
  gtk_widget_show (entry_1);
  gtk_box_pack_start (GTK_BOX (hbox), entry_1, TRUE, TRUE, 0);

  shortcuts.label (label_entry_1);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label_entry_1), entry_1);
  
  label_combo = gtk_label_new ("");
  gtk_widget_show (label_combo);
  gtk_box_pack_start (GTK_BOX (hbox), label_combo, FALSE, FALSE, 0);

  combo = gtk_combo_box_new_text ();
  gtk_widget_show (combo);
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);

  shortcuts.label (label_combo);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label_combo), combo);
  
  label_entry_2 = gtk_label_new ("");
  gtk_widget_show (label_entry_2);
  gtk_box_pack_start (GTK_BOX (hbox), label_entry_2, FALSE, FALSE, 0);

  entry_2 = gtk_entry_new ();
  gtk_widget_show (entry_2);
  gtk_box_pack_start (GTK_BOX (hbox), entry_2, TRUE, TRUE, 0);

  shortcuts.label (label_entry_2);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label_entry_2), entry_2);
  
  g_signal_connect ((gpointer) entry_1, "changed", entry_handler, gpointer (this));
  g_signal_connect ((gpointer) entry_2, "changed", entry_handler, gpointer (this));
}


// Resource conversion type.


void ResourceConverter2Dialog::on_type_button_clicked (GtkButton *button, gpointer user_data)
{
  ((ResourceConverter2Dialog *) user_data)->on_type_button (false);
}


void ResourceConverter2Dialog::on_type_button (bool no_ask)
{
  bool dialog_ok = false;
  if (!no_ask) {
    vector <ustring> types;
    for (unsigned int i = 0; i < rctEnd; i++) {
      types.push_back (resource_conversion_type_to_text ((ResourceConversionType)i));
    }
    ListviewDialog dialog ("Select resource conversion type", types, resource_conversion_type_to_text (resource_conversion_type), false, NULL);
    if (dialog.run () == GTK_RESPONSE_OK) {
      dialog_ok = true;
      resource_conversion_type = resource_conversion_text_to_type (dialog.focus);
    }
  }
  if (dialog_ok || no_ask) {
    Shortcuts shortcuts (0);
    shortcuts.stockbutton (okbutton);
    shortcuts.stockbutton (cancelbutton);
    shortcuts.stockbutton (helpbutton);
    build_table_and_type (shortcuts);
    build_gui (shortcuts);
    shortcuts.process ();
  }
  if (type_gui ()) {
    gui ();
  }
}


bool ResourceConverter2Dialog::type_gui ()
{
  ustring type = resource_conversion_type_to_text (resource_conversion_type);
  bool ok = !type.empty ();
  gtk_label_set_text (GTK_LABEL (label_type_short), "Select the resource conversion type");
  gtk_label_set_text (GTK_LABEL (label_type_long), type.c_str());
  gui_okay (image_type_ok, label_type_ok, ok);
  return ok;
}


void ResourceConverter2Dialog::build_gui (Shortcuts& shortcuts)
{
  switch (resource_conversion_type) {
    case rctChapterStartsAtPatternVerseOneStartsAtChapterVerseStartsAtPattern:
    {
      build_button (image_open_file_ok, label_open_file_ok, label_open_file_short, button_open_file, "Open", shortcuts, G_CALLBACK (on_open_file_button_clicked), label_open_file_long);

      build_button (image_view_file_ok, label_view_file_ok, label_view_file_short, button_view_file_html, "View", shortcuts, G_CALLBACK (on_view_file_button_clicked), label_view_file_long);

      build_entry_combo_entry (image_chapter_pattern_ok, label_chapter_pattern_ok, label_chapter_pattern, label_chapter_pattern_1, entry_chapter_pattern_1, label_chapter_pattern_combo, combo_chapter_pattern, label_chapter_pattern_2, entry_chapter_pattern_2, G_CALLBACK (on_chapter_pattern_entry_changed), shortcuts);
      gtk_entry_set_text (GTK_ENTRY (entry_chapter_pattern_1), chapter_pattern_prefix.c_str());
      vector <ustring> chapters; 
      chapters.push_back (integer_or_roman_sample (0));
      chapters.push_back (integer_or_roman_sample (1));
      combobox_set_strings (combo_chapter_pattern, chapters);
      combobox_set_index (combo_chapter_pattern, chapter_pattern_index);
      gtk_entry_set_text (GTK_ENTRY (entry_chapter_pattern_2), chapter_pattern_suffix.c_str());
      gtk_label_set_text (GTK_LABEL (label_chapter_pattern), "Enter the text pattern to detect the chapter");
      gtk_label_set_text (GTK_LABEL (label_chapter_pattern_1), "Prefix");
      gtk_label_set_text (GTK_LABEL (label_chapter_pattern_combo), "Chapter number");
      gtk_label_set_text (GTK_LABEL (label_chapter_pattern_2), "Suffix");

      build_entry_combo_entry (image_verse_pattern_ok, label_verse_pattern_ok, label_verse_pattern, label_verse_pattern_1, entry_verse_pattern_1, label_verse_pattern_combo, combo_verse_pattern, label_verse_pattern_2, entry_verse_pattern_2, G_CALLBACK (on_verse_pattern_entry_changed), shortcuts);
      gtk_entry_set_text (GTK_ENTRY (entry_verse_pattern_1), verse_pattern_prefix.c_str());
      vector <ustring> verses;
      verses.push_back (integer_or_roman_sample (0));
      verses.push_back (integer_or_roman_sample (1));
      combobox_set_strings (combo_verse_pattern, verses);
      combobox_set_index (combo_verse_pattern, verse_pattern_index);
      gtk_entry_set_text (GTK_ENTRY (entry_verse_pattern_2), verse_pattern_suffix.c_str());
      gtk_label_set_text (GTK_LABEL (label_verse_pattern), "Enter the text pattern to detect the verse");
      gtk_label_set_text (GTK_LABEL (label_verse_pattern_1), "Prefix");
      gtk_label_set_text (GTK_LABEL (label_verse_pattern_combo), "Verse number");
      gtk_label_set_text (GTK_LABEL (label_verse_pattern_2), "Suffix");
      
      build_button (image_write_anchors_ok, label_write_anchors_ok, label_write_anchors_short, button_write_anchors, "Write", shortcuts, G_CALLBACK (on_write_anchors_button_clicked), label_write_anchors_long);

      break;
    }
    case rctEnd:
    {
      break;
    }
  }
}


void ResourceConverter2Dialog::on_okbutton_clicked (GtkButton *button, gpointer user_data)
{
  ((ResourceConverter2Dialog *) user_data)->on_okbutton ();
}


void ResourceConverter2Dialog::on_okbutton ()
{
  chapter_pattern_gui ();
}


void ResourceConverter2Dialog::gui ()
{
  bool ok = true;
  if (!type_gui ()) ok = false;
  switch (resource_conversion_type) {
    case rctChapterStartsAtPatternVerseOneStartsAtChapterVerseStartsAtPattern:
    {
      if (!open_file_gui ()) ok = false;
      if (!view_file_gui ()) ok = false;
      if (!chapter_pattern_gui ()) ok = false;
      if (!verse_pattern_gui ()) ok = false;
      if (!write_anchors_gui ()) ok = false;
      break;
    }
    case rctEnd:
    {
      break;
    }
  }
  gtk_widget_set_sensitive (okbutton, ok);
}


// Open file.


void ResourceConverter2Dialog::on_open_file_button_clicked (GtkButton *button, gpointer user_data)
{
  ((ResourceConverter2Dialog *) user_data)->on_open_file_button ();
}


void ResourceConverter2Dialog::on_open_file_button ()
{
  // List all files in the working directory.
  ReadFiles rf (workingdirectory, "", "");
  // Select a file.
  ListviewDialog dialog ("Select file", rf.files, filename, true, NULL);
  if (dialog.run () == GTK_RESPONSE_OK) {
    // Store filename, read it.
    filename = dialog.focus;
    ReadText rt (gw_build_filename (workingdirectory, filename), true, false);
    lines = rt.lines;
    anchors_written = false;
  }
  if (open_file_gui ()) {
    gui ();
  }
}


bool ResourceConverter2Dialog::open_file_gui ()
{
  bool ok = !filename.empty ();
  ustring s (filename);
  if (ok) {
    s.append (": ");
    s.append (convert_to_string (lines.size()));
    s.append (" lines");
  }
  gtk_label_set_text (GTK_LABEL (label_open_file_short), "Open a file for conversion");
  gtk_label_set_text (GTK_LABEL (label_open_file_long), s.c_str());
  gui_okay (image_open_file_ok, label_open_file_ok, ok);
  return ok;
}


// View file.


void ResourceConverter2Dialog::on_view_file_button_clicked (GtkButton *button, gpointer user_data)
{
  ((ResourceConverter2Dialog *) user_data)->on_view_file_button ();
}


void ResourceConverter2Dialog::on_view_file_button ()
{
  ustring tempfile = gw_build_filename (directories_get_temp (), "resource-converter-view-file");
  write_lines (tempfile, lines);
  htmlbrowser (tempfile, false);
}


bool ResourceConverter2Dialog::view_file_gui ()
{
  bool ok = !filename.empty ();
  gtk_label_set_text (GTK_LABEL (label_view_file_short), "View the file");
  gui_okay (image_view_file_ok, label_view_file_ok, ok);
  return ok;
}


// Chapter detection pattern.


void ResourceConverter2Dialog::on_chapter_pattern_entry_changed (GtkEditable *editable, gpointer user_data)
{
  ((ResourceConverter2Dialog *) user_data)->on_chapter_pattern_entry ();
}


void ResourceConverter2Dialog::on_chapter_pattern_entry ()
{
  if (chapter_pattern_gui ())
    gui ();
}


bool ResourceConverter2Dialog::chapter_pattern_gui ()
{
  bool ok = true;
  chapter_pattern_prefix = gtk_entry_get_text (GTK_ENTRY (entry_chapter_pattern_1));
  chapter_pattern_index = combobox_get_active_index (combo_chapter_pattern);
  chapter_pattern_suffix = gtk_entry_get_text (GTK_ENTRY (entry_chapter_pattern_2));
  if (chapter_pattern_prefix.empty () && chapter_pattern_suffix.empty ()) ok = false;  
  gui_okay (image_chapter_pattern_ok, label_chapter_pattern_ok, ok);
  return ok;
}


// Verse detection pattern.


void ResourceConverter2Dialog::on_verse_pattern_entry_changed (GtkEditable *editable, gpointer user_data)
{
  ((ResourceConverter2Dialog *) user_data)->on_verse_pattern_entry ();
}


void ResourceConverter2Dialog::on_verse_pattern_entry ()
{
  if (verse_pattern_gui ())
    gui ();
}


bool ResourceConverter2Dialog::verse_pattern_gui ()
{
  bool ok = true;
  verse_pattern_prefix = gtk_entry_get_text (GTK_ENTRY (entry_verse_pattern_1));
  verse_pattern_index = combobox_get_active_index (combo_verse_pattern);
  verse_pattern_suffix = gtk_entry_get_text (GTK_ENTRY (entry_verse_pattern_2));
  if (verse_pattern_prefix.empty () && verse_pattern_suffix.empty ()) ok = false;  
  gui_okay (image_verse_pattern_ok, label_verse_pattern_ok, ok);
  return ok;
}


// Write anchors.


void ResourceConverter2Dialog::on_write_anchors_button_clicked (GtkButton *button, gpointer user_data)
{
  ((ResourceConverter2Dialog *) user_data)->on_write_anchors_button ();
}


void ResourceConverter2Dialog::on_write_anchors_button ()
{
  // Remove old anchors.
  resource_conversion_remove_anchors (lines);

  // Flags.
  anchors_written = true;
  
  // Do the conversion.
  switch (resource_conversion_type) {
    case rctChapterStartsAtPatternVerseOneStartsAtChapterVerseStartsAtPattern:
    {
      resource_conversion_insert_anchors (lines, 
                                          chapter_pattern_prefix, chapter_pattern_index, chapter_pattern_suffix,
                                            verse_pattern_prefix,   verse_pattern_index,   verse_pattern_suffix);
      break;
    }
    case rctEnd:
    {
      break;
    }
  }
  
  // Write text to file.
  write_lines (gw_build_filename (workingdirectory, filename), lines);

  // Update gui.
  if (write_anchors_gui ()) {
    gui ();
  }
}


bool ResourceConverter2Dialog::write_anchors_gui ()
{
  bool ok = true;
  if (!anchors_written) ok = false;
  gtk_label_set_text (GTK_LABEL (label_write_anchors_short), "Write anchors");
  unsigned int anchor_count = 0;
  for (unsigned int i = 0; i < lines.size (); i++) {
    ustring s (lines[i]);
    size_t pos;
    do {
      pos = s.find (resource_conversion_anchor_prefix ());
      if (pos != string::npos) {
        anchor_count++;
        pos += 2;
        s.erase (0, pos);
      }
    } while (pos != string::npos);
  }
  if (anchor_count == 0) ok = false;
  ustring s = "Anchors in file: " + convert_to_string (anchor_count);
  gtk_label_set_text (GTK_LABEL (label_write_anchors_long), s.c_str());
  gui_okay (image_write_anchors_ok, label_write_anchors_ok, ok);
  return ok;
}


/*
Code from the previous resource converter:


void ResourceConverterDialog::on_examinebutton ()
{
  // Clear any previous anchors.
  anchors_books.clear();
  anchors_chapters.clear();
  anchors_verses.clear();
  anchors_positions.clear ();
  currentbook = 0;
  currentchapter = 0;
  
  // Go through all the lines
  // Show progress. Allow cancel.
  ProgressWindow progress ("Examining data", true);
  progress.set_iterate (0, 1, lines.size());
  for (unsigned int i = 0; i < lines.size(); i++) {
    progress.iterate ();
    if (progress.cancel) return;
      
    // Produce a line to work on.
    // If the next line does not start with a "<", add it to the main one.
    // This is because at times the html editor may cut a signature in the middle.
    // Making one clean line also greatly reduces the amount of spurious 
    // anchors that would otherwise be found.
    ustring line (lines[i]);
    while (   (i + 1 != lines.size()) 
           && (!lines[i + 1].empty()) 
           && (lines[i + 1].substr (0, 1) != "<")) {
      i++;
      progress.iterate ();
      line.append (" ");
      line.append (lines[i]);
    }
    
    // Produce a clean line, without the html code, just the pure text only.
    line = html_remove_code_between_less_than_and_greater_than_signs (line);
    // Replace the long with the short hyphen. Important when recognizing
    // ranges of verses.
    replace_text (line, "–", "-");    

    // If there is no text, skip rest.
    if (line.empty ()) continue;
    
    // Do the appropriate examination.
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radiobutton_signature_book_space_chapter_colon_verse_line_start)))
      examine_book_space_chapter_colon_verse_line_start (i, line);
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radiobutton_signature_book_and_chapter_on_a_line_verses_follow)))
      examine_book_and_chapter_on_a_line_verses_follow (i, line);
  }

  // Set gui.
  ustring label ("Possible locations for anchors found: " + convert_to_string (anchors_books.size()));
  gtk_label_set_text (GTK_LABEL (label_examine_results), label.c_str());
  examined = true;  
  reviewed = false;
  gui ();
  gtk_widget_grab_focus (button_review);
}


void ResourceConverterDialog::on_reviewbutton ()
{
  ReviewAnchorsDialog dialog (&anchors_books, &anchors_chapters, &anchors_verses, &anchors_positions);
  if (dialog.run () == GTK_RESPONSE_OK) {
    ustring label ("Anchors that will be placed: " + convert_to_string (anchors_books.size()));
    gtk_label_set_text (GTK_LABEL (label_review_result), label.c_str());
    reviewed = true;
    written = false;
    gui ();
    gtk_widget_grab_focus (button_write);
  }
}


void ResourceConverterDialog::on_writebutton ()
{
  // Ask whether to write the new data. If not, bail out.
  ustring question;
  question.append ("Would you like to write the anchors to the file?");
  question.append ("\nWarning: This will overwrite the original file.");
  if (gtkw_dialog_question (resourceconverterdialog, question) != GTK_RESPONSE_YES) return;

  // Insert the anchors.
  for (unsigned int i = 0; i < anchors_positions.size(); i++) {
    insert_anchor (lines[anchors_positions[i]], html_create_anchor (resource_viewer_produce_anchor (anchors_books[i], anchors_chapters[i], anchors_verses[i]), ""));
  }

  // Write the data.
  write_lines (filename, lines);
  
  // GUI.
  gtk_label_set_text (GTK_LABEL (label_write_result), "The anchors were written");
  written = true;
  gui ();
  gtk_widget_grab_focus (okbutton);
  
}


void ResourceConverterDialog::examine_book_space_chapter_colon_verse_line_start (unsigned int linenumber, const ustring& line)
{
  // Parse the line into words separated by spaces,
  // and go through the second, third, and fourth word.
  Parse parse (line, false);
  unsigned int fourth_word = CLAMP (4, 0, parse.words.size());
  for (unsigned int i = 1; i < fourth_word; i++) {
    // Look for the "<chapter>:<verse>".
    if (unix_fnmatch ("*[0-9][:.][0-9]*", parse.words[i])) {

      // Find the position of this chapter:verse in the line.
      size_t chapter_verse_pos = line.find (parse.words[i]);
      
      // See if a sensible reference can be found in the raw text, from the
      // beginning of the line.
      ustring rawbook = line.substr (0, chapter_verse_pos - 1);
      unsigned int book = book_find_valid (rawbook);
      if (book) {
        unsigned int chapter;
        ustring verse;
        if (reference_discover (book, 0, "", parse.words[i], book, chapter, verse)) {
          // Store the anchor(s).
          // We put "anchors" in a possible plural, because ranges and sequences
          // are covered too.
          vector <unsigned int> verses = verse_range_sequence (verse);
          for (unsigned int i2 = 0; i2 < verses.size(); i2++) {
            anchors_books.push_back (book);
            anchors_chapters.push_back (chapter);
            anchors_verses.push_back (verses[i2]);
            anchors_positions.push_back (linenumber);
          }
          break;
        }
      }
    }    
  }
}


void ResourceConverterDialog::examine_book_and_chapter_on_a_line_verses_follow (unsigned int linenumber, const ustring& line)
{
  // See if the whole line is a book/chapter.
  // If so, store them, and bail out.
  unsigned int book;
  unsigned int chapter;
  ustring verse;
  if (line.length() < 25) {
    if (!number_in_string (line).empty()) {
      if (reference_discover (0, 0, "", line, book, chapter, verse)) {
        if (book && chapter) {
          currentbook = book;
          currentchapter = chapter;
          return;
        } 
      }
    }
  }

  // If there is no book or chapter yet, bail out.
  if (!currentbook) return;
  if (!currentchapter) return;
  
  // Check if a line starts with a verse number. If not, bail out.
  verse = number_in_string (line);
  if (verse.empty()) return;
  if (line.find (verse) != 0) return;
  
  // Store the anchor.
  anchors_books.push_back (currentbook);
  anchors_chapters.push_back (currentchapter);
  anchors_verses.push_back (convert_to_int (verse));
  anchors_positions.push_back (linenumber);
}


*/
