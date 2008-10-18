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
#include "windowshowquickrefs.h"
#include "help.h"
#include "windows.h"
#include "keyterms.h"
#include "tiny_utilities.h"

WindowShowQuickReferences::WindowShowQuickReferences(bool startup) :
  myreference(0)
  {
    window1 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW (window1), "Quick references");

    scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_show(scrolledwindow1);
    gtk_container_add(GTK_CONTAINER (window1), scrolledwindow1);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);

    textview1 = gtk_text_view_new();
    gtk_widget_show(textview1);
    gtk_container_add(GTK_CONTAINER (scrolledwindow1), textview1);
    gtk_text_view_set_editable(GTK_TEXT_VIEW (textview1), FALSE);
    gtk_text_view_set_accepts_tab(GTK_TEXT_VIEW (textview1), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (textview1), GTK_WRAP_WORD);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW (textview1), FALSE);

    signal_button = gtk_button_new();

    g_signal_connect ((gpointer) window1, "delete_event",
        G_CALLBACK (on_window_delete_event),
        gpointer (this));

    // The window needs to be positioned before it shows.
    window_display(window1, widShowQuickReferences, "", startup);

    gtk_widget_show_all(window1);
  }

WindowShowQuickReferences::~WindowShowQuickReferences()
  {
    window_delete(window1, widShowQuickReferences, "");
    gtk_widget_destroy(window1);
    gtk_widget_destroy(signal_button);
  }

bool WindowShowQuickReferences::on_window_delete_event(GtkWidget *widget,
    GdkEvent *event, gpointer user_data)
  {
    return ((WindowShowQuickReferences *) user_data)->on_window_delete();
  }

bool WindowShowQuickReferences::on_window_delete()
  {
    gtk_button_clicked(GTK_BUTTON (signal_button));
    return false;
  }

void WindowShowQuickReferences::go_to(const ustring& project, // Todo
    const Reference& reference)
  {
    // Bail out if there's no change in the reference.
    if (myreference.equals(reference))
      return;


    GtkTextBuffer * buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview1));
    gtk_text_buffer_set_text(buffer, "", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "Quick references\n", -1);

    // Store the new reference in the object.
    myreference.assign(reference);
  }


// Todo it needs to show a bunch of references.
