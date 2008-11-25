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

#include <config.h>
#include "mainwindow.h"
#include "libraries.h"
#include "dialoglistview.h"
#include "dialogshowscript.h"
#include "constants.h"
#include "utilities.h"
#include "htmlbrowser.h"
#include "usfmtools.h"
#include "dialogreplace.h"
#include "dialoggotoreference.h"
#include <signal.h>
#include "bible.h"
#include "projectutils.h"
#include "dialogproject.h"
#include "usfm.h"
#include "dialogimporttext.h"
#include "dialogreplacing.h"
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include "dialogsearchspecial.h"
#include "referenceutils.h"
#include "references.h"
#include "pdfviewer.h"
#include "notes_utils.h"
#include "dialogfindnote.h"
#include "dialogimportnotes.h"
#include "date_time_utils.h"
#include "dialognotes.h"
#include "dialogentry.h"
#include "projectutils.h"
#include "directories.h"
#include "dialogcompare.h"
#include "export_utils.h"
#include "dialogprintprefs.h"
#include "dialogprintproject.h"
#include "printproject.h"
#include "compareutils.h"
#include "dialogexportnotes.h"
#include "dialogshownotes.h"
#include "dialogentry3.h"
#include "gwrappers.h"
#include "gtkwrappers.h"
#include "search_utils.h"
#include "combobox.h"
#include "scripturechecks.h"
#include "dialogrefexchange.h"
#include "dialogeditlist.h"
#include <sqlite3.h>
#include "sqlite_reader.h"
#include "highlight.h"
#include "stylesheetutils.h"
#include "keyboard.h"
#include "dialoginsertnote.h"
#include "dialogparallelbible.h"
#include "print_parallel_bible.h"
#include "editor.h"
#include "layout.h"
#include "dialogbook.h"
#include "books.h"
#include "screen.h"
#include "dialoglinecutter.h"
#include "dialogoutpost.h"
#include "dialognotestransfer.h"
#include "dialogchapternumber.h"
#include "versification.h"
#include "dialogmychecks.h"
#include "help.h"
#include "dialogoriginreferences.h"
#include "dialogtidy.h"
#include "dialognotesupdate.h"
#include "dialogbackup.h"
#include "backup.h"
#include "dialogviewgit.h"
#include "dialoggitsetup.h"
#include "dialogchanges.h"
#include "dialogrevert.h"
#include "resource_utils.h"
#include "dialogeditnote.h"
#include "dialogwordlist.h"
#include "dialogfontcolor.h"
#include "color.h"
#include "dialognewstylesheet.h"
#include "settings.h"
#include "ipc.h"
#include "dialogfeatures.h"
#include "password.h"
#include "gui_features.h"
#include "dialogprintreferences.h"
#include "print_parallel_references.h"
#include "dialogfixmarkers.h"
#include "dialogtextreplacement.h"
#include "temporal.h"
#include "parallel_passages.h"
#include "dialogpdfviewer.h"
#include "dialogviewusfm.h"
#include "dialoginserttable.h"
#include "tiny_utilities.h"
#include "hyphenate.h"
#include "dialogreportingsetup.h"
#include "dialogeditstatus.h"
#include "dialogviewstatus.h"
#include "planning.h"
#include "dialogplanningsetup.h"
#include "dialoginterfaceprefs.h"
#include "dialognewresource.h"
#include "shutdown.h"
#include "dialogfilters.h"
#include "dialogradiobutton.h"
#include "import.h"
#include "dialogimportrawtext.h"
#include "dialogxfernotes2text.h"
#include "htmlcolor.h"
#include "text2pdf.h"
#include "windows.h"
#include "unixwrappers.h"
#include "accelerators.h"

/*
 |
 |
 |
 |
 |
 Construction and destruction
 |
 |
 |
 |
 |
 */

MainWindow::MainWindow(unsigned long xembed) :
  navigation(0), bibletime(true), httpd(0) {
  // Set some pointers to NULL.  
  // To save memory, we only create the object when actually needed.
  window_menu = NULL;
  window_screen_layout = NULL;
  window_show_keyterms = NULL;
  window_show_quick_references = NULL;
  window_merge = NULL;
  window_outline = NULL;
  window_check_keyterms = NULL;
  window_styles = NULL;
  window_notes = NULL;
  window_references = NULL;

  // Initialize some variables.
  git_reopen_project = false;
  windows_startup_pointer = 0;
  now_focused_signal_button = NULL;
  last_focused_signal_button = NULL;

  // Gui Features object.
  GuiFeatures guifeatures(0);
  project_notes_enabled = guifeatures.project_notes();

  // Accelerators.
  accelerator_group = gtk_accel_group_new();
  gtk_accel_group_connect(accelerator_group, GDK_X, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_cut_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_C, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_copy_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_V, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_paste_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_Z, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_undo_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_Z, GdkModifierType(GDK_CONTROL_MASK | GDK_SHIFT_MASK), GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_redo_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_1, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_standard_text_1_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_2, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_standard_text_2_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_3, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_standard_text_3_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_4, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_standard_text_4_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_N, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_new_project_note_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_Down, GDK_MOD1_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_next_verse_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_Up, GDK_MOD1_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_previous_verse_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_Page_Down, GDK_MOD1_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_next_chapter_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_Page_Up, GDK_MOD1_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_previous_chapter_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_Page_Down, (GdkModifierType) (GDK_CONTROL_MASK | GDK_MOD1_MASK), GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_next_book_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_Page_Up, (GdkModifierType) (GDK_CONTROL_MASK | GDK_MOD1_MASK), GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_previous_book_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_Right, GDK_MOD1_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_next_reference_in_history_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_Left, GDK_MOD1_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_previous_reference_in_history_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_G, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_go_to_reference_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_W, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_close_window_callback), gpointer(this), NULL));
  if (guifeatures.styles()) {
    gtk_accel_group_connect(accelerator_group, GDK_S, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_goto_styles_area_callback), gpointer(this), NULL));
  }
  gtk_accel_group_connect(accelerator_group, GDK_Q, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_quit_program_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_F5, GdkModifierType(0), GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_activate_text_area_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_F5, GDK_SHIFT_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_activate_tools_area_callback), gpointer(this), NULL));
  if (guifeatures.project_notes()) {
    gtk_accel_group_connect(accelerator_group, GDK_F5, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_activate_notes_area_callback), gpointer(this), NULL));
  }
  if (guifeatures.references_and_find()) {
    gtk_accel_group_connect(accelerator_group, GDK_F6, GdkModifierType(0), GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_next_reference_in_reference_area_callback), gpointer(this), NULL));
    gtk_accel_group_connect(accelerator_group, GDK_F6, GDK_SHIFT_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_previous_reference_in_reference_area_callback), gpointer(this), NULL));
  }
  gtk_accel_group_connect(accelerator_group, GDK_Right, (GdkModifierType) (GDK_CONTROL_MASK | GDK_MOD1_MASK), GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_next_project_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_Left, (GdkModifierType) (GDK_CONTROL_MASK | GDK_MOD1_MASK), GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_previous_project_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_O, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_open_project_callback), gpointer(this), NULL));
  if (guifeatures.printing()) {
    gtk_accel_group_connect(accelerator_group, GDK_P, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_print_callback), gpointer(this), NULL));
  }
  gtk_accel_group_connect(accelerator_group, GDK_C, (GdkModifierType) (GDK_CONTROL_MASK | GDK_SHIFT_MASK), GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_copy_without_formatting_callback), gpointer(this), NULL));
  if (guifeatures.references_and_find()) {
    gtk_accel_group_connect(accelerator_group, GDK_F, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_find_callback), gpointer(this), NULL));
  }
  if (guifeatures.replace()) {
    gtk_accel_group_connect(accelerator_group, GDK_R, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_replace_callback), gpointer(this), NULL));
  }
  gtk_accel_group_connect(accelerator_group, GDK_F1, GdkModifierType(0), GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_main_help_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_H, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_context_help_callback), gpointer(this), NULL));
  gtk_accel_group_connect(accelerator_group, GDK_M, GDK_CONTROL_MASK, GtkAccelFlags(0), g_cclosure_new_swap(G_CALLBACK(accelerator_menu_callback), gpointer(this), NULL));

  // GUI build.
  if (xembed) {
    mainwindow = gtk_plug_new(GdkNativeWindow(xembed));
  } else {
    mainwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  }
  gtk_window_set_default_icon_from_file(gw_build_filename (directories_get_package_data (), "bibledit.xpm").c_str(), NULL);
  gtk_window_set_gravity(GTK_WINDOW (mainwindow), GDK_GRAVITY_STATIC);

  // Menu window.
  display_menu_window();

  // Pointer to the settings.
  extern Settings * settings;

  g_set_application_name("Bibledit");

  // Start Outpost.
  windowsoutpost = new WindowsOutpost (true);
  if (settings->genconfig.use_outpost_get())
    windowsoutpost->Start();

  // Store project of last session because it gets affected when the editors build.
  focused_project_last_session = settings->genconfig.project_get();

  // Appearance of text in editor.
  set_fonts();

  // Size and position of window and screen layout.
  {
    ScreenLayoutDimensions dimensions(mainwindow);
    dimensions.verify();
  }

  g_signal_connect ((gpointer) mainwindow, "destroy", G_CALLBACK (gtk_main_quit), gpointer(this));
  g_signal_connect ((gpointer) mainwindow, "delete_event", G_CALLBACK (on_mainwindow_delete_event), gpointer(this));
  g_signal_connect ((gpointer) mainwindow, "focus_in_event", G_CALLBACK (on_mainwindow_focus_in_event), gpointer(this));
  g_signal_connect ((gpointer) mainwindow, "window_state_event", G_CALLBACK (on_mainwindow_window_state_event), gpointer(this));

  gtk_window_add_accel_group(GTK_WINDOW (mainwindow), accelerator_group);

  // Communication with BibleTime
  got_new_bt_reference = 0;
  g_timeout_add(100, GSourceFunc(mainwindow_on_external_programs_timeout), gpointer(this));

#ifndef WIN32
  // Signal handling.
  // Block the signal of a pipe error that otherwise would kill bibledit.
  signal(SIGPIPE, SIG_IGN);
  // USR1, ignore it.
  signal(SIGUSR1, SIG_IGN);
#endif

  // Display project notes.
  notes_redisplay();

  // Start the GUI updater.
  g_timeout_add(100, GSourceFunc(on_gui_timeout), gpointer(this));

  // Start bibledit http responder.
  g_timeout_add(300, GSourceFunc(on_check_httpd_timeout), gpointer(this));

  // Initialize content manager subsystem.
  git_initialize_subsystem();
  git_update_intervals_initialize();

  // Show main window iconified.
  gtk_widget_show(mainwindow);
  // Todo this works badly gtk_window_iconify(GTK_WINDOW(mainwindow));  

  // Interprocess communications.
  extern InterprocessCommunication * ipc;
  ipc->methodcall_add_signal(ipcctGitJobDescription);
  ipc->methodcall_add_signal(ipcctGitTaskDone);
  g_signal_connect ((gpointer) ipc->method_called_signal, "clicked", G_CALLBACK (on_ipc_method_called), gpointer(this));

  // Show open windows.
  g_timeout_add(300, GSourceFunc(on_windows_startup_timeout), gpointer(this));
}

MainWindow::~MainWindow() {
  // Destroy the accelerator system.
  g_object_unref(G_OBJECT (accelerator_group));

  // Shut the separate windows down.
  shutdown_windows();

  // No ipc signals anymore.
  extern InterprocessCommunication * ipc;
  ipc->methodcall_remove_all_signals();

  // Destroy the Outpost
  delete windowsoutpost;
  // Finalize content manager subsystem.
  git_finalize_subsystem();
  // Do shutdown actions.
  shutdown_actions();
  // Destroying the window is done by gtk itself.
}

int MainWindow::run() {
  return gtk_dialog_run(GTK_DIALOG (mainwindow));
}

/*
 |
 |
 |
 |
 |
 Initialization
 |
 |
 |
 |
 |
 */

void MainWindow::enable_or_disable_widgets(bool enable) {
  // Set some widgets (in)sensitive depending on whether a project is open.
  if (window_menu && window_menu->properties1)
    gtk_widget_set_sensitive(window_menu->properties1, enable);
  if (window_menu && window_menu->import1)
    gtk_widget_set_sensitive(window_menu->import1, enable);
  if (window_menu && window_menu->notes2)
    gtk_widget_set_sensitive(window_menu->notes2, enable);
  if (window_menu && window_menu->menuitem_edit)
    gtk_widget_set_sensitive(window_menu->menuitem_edit, enable);
  if (window_menu && window_menu->file_references)
    gtk_widget_set_sensitive(window_menu->file_references, enable);
  if (window_menu && window_menu->export_project)
    gtk_widget_set_sensitive(window_menu->export_project, enable);
  if (window_menu && window_menu->print)
    gtk_widget_set_sensitive(window_menu->print, enable);
  if (window_menu && window_menu->project_changes)
    gtk_widget_set_sensitive(window_menu->project_changes, enable);
  if (window_menu && window_menu->menuitem_view)
    gtk_widget_set_sensitive(window_menu->menuitem_view, enable);
  if (window_menu && window_menu->menuitem_goto)
    gtk_widget_set_sensitive(window_menu->menuitem_goto, enable);
  if (window_menu && window_menu->compare_with1)
    gtk_widget_set_sensitive(window_menu->compare_with1, enable);
  if (window_menu && window_menu->copy_project_to)
    gtk_widget_set_sensitive(window_menu->copy_project_to, enable);
  if (window_menu && window_menu->insert1)
    gtk_widget_set_sensitive(window_menu->insert1, enable);
  if (window_menu && window_menu->check1)
    gtk_widget_set_sensitive(window_menu->check1, enable);
  if (window_menu && window_menu->menutools)
    gtk_widget_set_sensitive(window_menu->menutools, enable);
  if (window_menu && window_menu->preferences_remote_git_repository)
    gtk_widget_set_sensitive(window_menu->preferences_remote_git_repository, enable);
  if (window_menu && window_menu->preferences_planning)
    gtk_widget_set_sensitive(window_menu->preferences_planning, enable);
  if (window_menu && window_menu->project_backup)
    gtk_widget_set_sensitive(window_menu->project_backup, enable);
  if (window_menu && window_menu->file_projects_merge)
    gtk_widget_set_sensitive(window_menu->file_projects_merge, enable);
  navigation.sensitive(enable);
}

/*
 |
 |
 |
 |
 |
 Menu window
 |
 |
 |
 |
 |
 */

void MainWindow::display_menu_window() {
  if (window_menu == NULL) {
    window_menu = new WindowMenu (accelerator_group, windows_startup_pointer != G_MAXINT);
    g_signal_connect ((gpointer) window_menu->delete_signal_button, "clicked", G_CALLBACK (on_window_menu_delete_button_clicked), gpointer(this));
    g_signal_connect ((gpointer) window_menu->focus_in_signal_button, "clicked", G_CALLBACK (on_window_focus_button_clicked), gpointer(this));
    // Menu callbacks.
    if (window_menu->new1)
      g_signal_connect ((gpointer) window_menu->new1, "activate", G_CALLBACK (on_new1_activate), gpointer(this));
    if (window_menu->open1)
      g_signal_connect ((gpointer) window_menu->open1, "activate", G_CALLBACK (on_open1_activate), gpointer(this));
    if (window_menu->delete1)
      g_signal_connect ((gpointer) window_menu->delete1, "activate", G_CALLBACK (on_delete1_activate), gpointer(this));
    if (window_menu->properties1)
      g_signal_connect ((gpointer) window_menu->properties1, "activate", G_CALLBACK (on_properties1_activate), gpointer(this));
    if (window_menu->import1)
      g_signal_connect ((gpointer) window_menu->import1, "activate", G_CALLBACK (on_import1_activate), gpointer(this));
    if (window_menu->export_usfm_files)
      g_signal_connect ((gpointer) window_menu->export_usfm_files, "activate", G_CALLBACK (on_export_usfm_files_activate), gpointer(this));
    if (window_menu->export_zipped_unified_standard_format_markers1)
      g_signal_connect ((gpointer) window_menu->export_zipped_unified_standard_format_markers1, "activate", G_CALLBACK (on_export_zipped_unified_standard_format_markers1_activate), gpointer(this));
    if (window_menu->to_bibleworks_version_database_compiler)
      g_signal_connect ((gpointer) window_menu->to_bibleworks_version_database_compiler, "activate", G_CALLBACK (on_to_bibleworks_version_compiler_activate), gpointer(this));
    if (window_menu->export_to_sword_module)
      g_signal_connect ((gpointer) window_menu->export_to_sword_module, "activate", G_CALLBACK (on_export_to_sword_module_activate), gpointer(this));
    if (window_menu->export_opendocument)
      g_signal_connect ((gpointer) window_menu->export_opendocument, "activate", G_CALLBACK (on_export_opendocument_activate), gpointer(this));
    if (window_menu->copy_project_to)
      g_signal_connect ((gpointer) window_menu->copy_project_to, "activate", G_CALLBACK (on_copy_project_to_activate), gpointer(this));
    if (window_menu->compare_with1)
      g_signal_connect ((gpointer) window_menu->compare_with1, "activate", G_CALLBACK (on_compare_with1_activate), gpointer(this));
    if (window_menu->project_backup_incremental)
      g_signal_connect ((gpointer) window_menu->project_backup_incremental, "activate", G_CALLBACK (on_project_backup_incremental_activate), gpointer(this));
    if (window_menu->project_backup_flexible)
      g_signal_connect ((gpointer) window_menu->project_backup_flexible, "activate", G_CALLBACK (on_project_backup_flexible_activate), gpointer(this));
    if (window_menu->project_changes)
      g_signal_connect ((gpointer) window_menu->project_changes, "activate", G_CALLBACK (on_project_changes_activate), gpointer(this));
    if (window_menu->file_projects_merge)
      g_signal_connect ((gpointer) window_menu->file_projects_merge, "activate", G_CALLBACK (on_file_projects_merge_activate), gpointer(this));
    if (window_menu->open_references1)
      g_signal_connect ((gpointer) window_menu->open_references1, "activate", G_CALLBACK (on_open_references1_activate), gpointer(this));
    if (window_menu->references_save_as)
      g_signal_connect ((gpointer) window_menu->references_save_as, "activate", G_CALLBACK (on_references_save_as_activate), gpointer(this));
    if (window_menu->close_references)
      g_signal_connect ((gpointer) window_menu->close_references, "activate", G_CALLBACK (on_close_references_activate), gpointer (this));
    if (window_menu->delete_references)
      g_signal_connect ((gpointer) window_menu->delete_references, "activate", G_CALLBACK (on_delete_references_activate), gpointer (this));
    if (window_menu->reference_hide)
      g_signal_connect ((gpointer) window_menu->reference_hide, "activate", G_CALLBACK (on_reference_hide_activate), gpointer (this));
    if (window_menu->new_note)
      g_signal_connect ((gpointer) window_menu->new_note, "activate", G_CALLBACK (on_new_note_activate), gpointer(this));
    if (window_menu->delete_note)
      g_signal_connect ((gpointer) window_menu->delete_note, "activate", G_CALLBACK (on_delete_note_activate), gpointer(this));
    if (window_menu->import_notes)
      g_signal_connect ((gpointer) window_menu->import_notes, "activate", G_CALLBACK (on_import_notes_activate), gpointer(this));
    if (window_menu->export_notes)
      g_signal_connect ((gpointer) window_menu->export_notes, "activate", G_CALLBACK (on_export_notes_activate), gpointer(this));
    if (window_menu->file_resources)
      g_signal_connect ((gpointer) window_menu->file_resources, "activate", G_CALLBACK (on_file_resources_activate), gpointer(this));
    if (window_menu->file_resources_open)
      g_signal_connect ((gpointer) window_menu->file_resources_open, "activate", G_CALLBACK (on_file_resources_open_activate), gpointer(this));
    if (window_menu->file_resources_close)
      g_signal_connect ((gpointer) window_menu->file_resources_close, "activate", G_CALLBACK (on_file_resources_close_activate), gpointer(this));
    if (window_menu->file_resources_new)
      g_signal_connect ((gpointer) window_menu->file_resources_new, "activate", G_CALLBACK (on_file_resources_new_activate), gpointer(this));
    if (window_menu->file_resources_edit)
      g_signal_connect ((gpointer) window_menu->file_resources_edit, "activate", G_CALLBACK (on_file_resources_edit_activate), gpointer(this));
    if (window_menu->file_resources_delete)
      g_signal_connect ((gpointer) window_menu->file_resources_delete, "activate", G_CALLBACK (on_file_resources_delete_activate), gpointer(this));
    if (window_menu->print)
      g_signal_connect ((gpointer) window_menu->print, "activate", G_CALLBACK (on_print_activate), gpointer(this));
    if (window_menu->quit1)
      g_signal_connect ((gpointer) window_menu->quit1, "activate", G_CALLBACK (on_quit1_activate), gpointer(this));
    if (window_menu->menuitem_edit)
      g_signal_connect ((gpointer) window_menu->menuitem_edit, "activate", G_CALLBACK (on_edit1_activate), gpointer(this));
    if (window_menu->cut1)
      g_signal_connect ((gpointer) window_menu->cut1, "activate", G_CALLBACK (on_cut1_activate), gpointer(this));
    if (window_menu->copy1)
      g_signal_connect ((gpointer) window_menu->copy1, "activate", G_CALLBACK (on_copy1_activate), gpointer(this));
    if (window_menu->copy_without_formatting)
      g_signal_connect ((gpointer) window_menu->copy_without_formatting, "activate", G_CALLBACK (on_copy_without_formatting_activate), gpointer(this));
    if (window_menu->paste1)
      g_signal_connect ((gpointer) window_menu->paste1, "activate", G_CALLBACK (on_paste1_activate), gpointer(this));
    if (window_menu->undo1)
      g_signal_connect ((gpointer) window_menu->undo1, "activate", G_CALLBACK (on_undo1_activate), gpointer(this));
    if (window_menu->redo1)
      g_signal_connect ((gpointer) window_menu->redo1, "activate", G_CALLBACK (on_redo1_activate), gpointer(this));
    if (window_menu->find1)
      g_signal_connect ((gpointer) window_menu->find1, "activate", G_CALLBACK (on_findspecial1_activate), gpointer(this));
    if (window_menu->find_and_replace1)
      g_signal_connect ((gpointer) window_menu->find_and_replace1, "activate", G_CALLBACK (on_find_and_replace1_activate), gpointer (this));
    if (window_menu->find_in_notes1)
      g_signal_connect ((gpointer) window_menu->find_in_notes1, "activate", G_CALLBACK (on_find_in_notes1_activate), gpointer(this));
    if (window_menu->get_references_from_note)
      g_signal_connect ((gpointer) window_menu->get_references_from_note, "activate", G_CALLBACK (on_get_references_from_note_activate), gpointer(this));
    if (window_menu->edit_revert)
      g_signal_connect ((gpointer) window_menu->edit_revert, "activate", G_CALLBACK (on_edit_revert_activate), gpointer(this));
    if (window_menu->edit_bible_note)
      g_signal_connect ((gpointer) window_menu->edit_bible_note, "activate", G_CALLBACK (on_edit_bible_note_activate), gpointer(this));
    if (window_menu->editstatus)
      g_signal_connect ((gpointer) window_menu->editstatus, "activate", G_CALLBACK (on_editstatus_activate), gpointer(this));
    if (window_menu->edit_planning)
      g_signal_connect ((gpointer) window_menu->edit_planning, "activate", G_CALLBACK (on_edit_planning_activate), gpointer(this));
    if (window_menu->menuitem_view)
      g_signal_connect ((gpointer) window_menu->menuitem_view, "activate", G_CALLBACK (on_menuitem_view_activate), gpointer(this));
    if (window_menu->view_text_font)
      g_signal_connect ((gpointer) window_menu->view_text_font, "activate", G_CALLBACK (on_view_text_font_activate), gpointer(this));
    if (window_menu->viewnotes)
      g_signal_connect ((gpointer) window_menu->viewnotes, "activate", G_CALLBACK (on_viewnotes_activate), gpointer(this));
    if (window_menu->view_git_tasks)
      g_signal_connect ((gpointer) window_menu->view_git_tasks, "activate", G_CALLBACK (on_view_git_tasks_activate), gpointer(this));
    if (window_menu->parallel_passages1)
      g_signal_connect ((gpointer) window_menu->parallel_passages1, "activate", G_CALLBACK (on_parallel_passages1_activate), gpointer(this));
    if (window_menu->view_usfm_code)
      g_signal_connect ((gpointer) window_menu->view_usfm_code, "activate", G_CALLBACK (on_view_usfm_code_activate), gpointer(this));
    if (window_menu->view_status)
      g_signal_connect ((gpointer) window_menu->view_status, "activate", G_CALLBACK (on_view_status_activate), gpointer(this));
    if (window_menu->view_planning)
      g_signal_connect ((gpointer) window_menu->view_planning, "activate", G_CALLBACK (on_view_planning_activate), gpointer(this));
    if (window_menu->view_screen_layout)
      g_signal_connect ((gpointer) window_menu->view_screen_layout, "activate", G_CALLBACK (on_view_screen_layout_activate), gpointer(this));
    if (window_menu->view_keyterms)
      g_signal_connect ((gpointer) window_menu->view_keyterms, "activate", G_CALLBACK (on_view_keyterms_activate), gpointer(this));
    if (window_menu->view_quick_references)
      g_signal_connect ((gpointer) window_menu->view_quick_references, "activate", G_CALLBACK (on_view_quick_references_activate), gpointer(this));
    if (window_menu->view_outline)
      g_signal_connect ((gpointer) window_menu->view_outline, "activate", G_CALLBACK (on_view_outline_activate), gpointer(this));
    if (window_menu->insert1)
      g_signal_connect ((gpointer) window_menu->insert1, "activate", G_CALLBACK (on_insert1_activate), gpointer(this));
    if (window_menu->standard_text_1)
      g_signal_connect ((gpointer) window_menu->standard_text_1, "activate", G_CALLBACK (on_standard_text_1_activate), gpointer (this));
    if (window_menu->standard_text_2)
      g_signal_connect ((gpointer) window_menu->standard_text_2, "activate", G_CALLBACK (on_standard_text_2_activate), gpointer (this));
    if (window_menu->standard_text_3)
      g_signal_connect ((gpointer) window_menu->standard_text_3, "activate", G_CALLBACK (on_standard_text_3_activate), gpointer (this));
    if (window_menu->standard_text_4)
      g_signal_connect ((gpointer) window_menu->standard_text_4, "activate", G_CALLBACK (on_standard_text_4_activate), gpointer (this));
    if (window_menu->current_reference1)
      g_signal_connect ((gpointer) window_menu->current_reference1, "activate", G_CALLBACK (on_current_reference1_activate), gpointer (this));
    if (window_menu->insert_special_character)
      g_signal_connect ((gpointer) window_menu->insert_special_character, "activate", G_CALLBACK (on_insert_special_character_activate), gpointer (this));
    if (window_menu->synchronize_other_programs2)
      g_signal_connect ((gpointer) window_menu->synchronize_other_programs2, "activate", G_CALLBACK (on_synchronize_other_programs2_activate), gpointer(this));
    if (window_menu->check1)
      g_signal_connect ((gpointer) window_menu->check1, "activate", G_CALLBACK (on_check1_activate), gpointer(this));
    if (window_menu->markers1)
      g_signal_connect ((gpointer) window_menu->markers1, "activate", G_CALLBACK (on_markers1_activate), gpointer(this));
    if (window_menu->validate_usfms1)
      g_signal_connect ((gpointer) window_menu->validate_usfms1, "activate", G_CALLBACK (on_validate_usfms1_activate), gpointer(this));
    if (window_menu->count_usfms1)
      g_signal_connect ((gpointer) window_menu->count_usfms1, "activate", G_CALLBACK (on_count_usfms1_activate), gpointer(this));
    if (window_menu->compare_usfm1)
      g_signal_connect ((gpointer) window_menu->compare_usfm1, "activate", G_CALLBACK (on_compare_usfm1_activate), gpointer(this));
    if (window_menu->check_markers_spacing)
      g_signal_connect ((gpointer) window_menu->check_markers_spacing, "activate", G_CALLBACK (on_check_markers_spacing_activate), gpointer(this));
    if (window_menu->chapters_and_verses1)
      g_signal_connect ((gpointer) window_menu->chapters_and_verses1, "activate", G_CALLBACK (on_chapters_and_verses1_activate), gpointer(this));
    if (window_menu->count_characters)
      g_signal_connect ((gpointer) window_menu->count_characters, "activate", G_CALLBACK (on_count_characters_activate), gpointer(this));
    if (window_menu->unwanted_patterns)
      g_signal_connect ((gpointer) window_menu->unwanted_patterns, "activate", G_CALLBACK (on_unwanted_patterns_activate), gpointer(this));
    if (window_menu->check_capitalization)
      g_signal_connect ((gpointer) window_menu->check_capitalization, "activate", G_CALLBACK (on_check_capitalization_activate), gpointer(this));
    if (window_menu->check_repetition)
      g_signal_connect ((gpointer) window_menu->check_repetition, "activate", G_CALLBACK (on_check_repetition_activate), gpointer(this));
    if (window_menu->unwanted_words)
      g_signal_connect ((gpointer) window_menu->unwanted_words, "activate", G_CALLBACK (on_unwanted_words_activate), gpointer(this));
    if (window_menu->check_matching_pairs)
      g_signal_connect ((gpointer) window_menu->check_matching_pairs, "activate", G_CALLBACK (on_check_matching_pairs_activate), gpointer(this));
    if (window_menu->check_sentence_structure)
      g_signal_connect ((gpointer) window_menu->check_sentence_structure, "activate", G_CALLBACK (on_check_sentence_structure_activate), gpointer(this));
    if (window_menu->word_count_inventory)
      g_signal_connect ((gpointer) window_menu->word_count_inventory, "activate", G_CALLBACK (on_word_count_inventory_activate), gpointer(this));
    if (window_menu->check_references_inventory)
      g_signal_connect ((gpointer) window_menu->check_references_inventory, "activate", G_CALLBACK (on_check_references_inventory_activate), gpointer(this));
    if (window_menu->check_references_validate)
      g_signal_connect ((gpointer) window_menu->check_references_validate, "activate", G_CALLBACK (on_check_references_validate_activate), gpointer(this));
    if (window_menu->check_nt_quotations_from_the_ot)
      g_signal_connect ((gpointer) window_menu->check_nt_quotations_from_the_ot, "activate", G_CALLBACK (on_check_nt_quotations_from_the_ot_activate), gpointer(this));
    if (window_menu->synoptic_parallel_passages_from_the_nt)
      g_signal_connect ((gpointer) window_menu->synoptic_parallel_passages_from_the_nt, "activate", G_CALLBACK (on_synoptic_parallel_passages_from_the_nt_activate), gpointer(this));
    if (window_menu->parallels_from_the_ot)
      g_signal_connect ((gpointer) window_menu->parallels_from_the_ot, "activate", G_CALLBACK (on_parallels_from_the_ot_activate), gpointer(this));
    if (window_menu->check_key_terms)
      g_signal_connect ((gpointer) window_menu->check_key_terms, "activate", G_CALLBACK (on_check_key_terms_activate), gpointer(this));
    if (window_menu->my_checks)
      g_signal_connect ((gpointer) window_menu->my_checks, "activate", G_CALLBACK (on_my_checks_activate), gpointer(this));
    if (window_menu->menutools)
      g_signal_connect ((gpointer) window_menu->menutools, "activate", G_CALLBACK (on_menutools_activate), gpointer(this));
    if (window_menu->line_cutter_for_hebrew_text1)
      g_signal_connect ((gpointer) window_menu->line_cutter_for_hebrew_text1, "activate", G_CALLBACK (on_line_cutter_for_hebrew_text1_activate), gpointer(this));
    if (window_menu->notes_transfer)
      g_signal_connect ((gpointer) window_menu->notes_transfer, "activate", G_CALLBACK (on_notes_transfer_activate), gpointer(this));
    if (window_menu->tool_origin_references_in_bible_notes)
      g_signal_connect ((gpointer) window_menu->tool_origin_references_in_bible_notes, "activate", G_CALLBACK (on_tool_origin_references_in_bible_notes_activate), gpointer(this));
    if (window_menu->tool_project_notes_mass_update1)
      g_signal_connect ((gpointer) window_menu->tool_project_notes_mass_update1, "activate", G_CALLBACK (on_tool_project_notes_mass_update1_activate), gpointer(this));
    if (window_menu->tool_generate_word_lists)
      g_signal_connect ((gpointer) window_menu->tool_generate_word_lists, "activate", G_CALLBACK (on_tool_generate_word_lists_activate), gpointer(this));
    if (window_menu->tool_simple_text_corrections)
      g_signal_connect ((gpointer) window_menu->tool_simple_text_corrections, "activate", G_CALLBACK (on_tool_simple_text_corrections_activate), gpointer(this));
    if (window_menu->tool_transfer_project_notes_to_text)
      g_signal_connect ((gpointer) window_menu->tool_transfer_project_notes_to_text, "activate", G_CALLBACK (on_tool_transfer_project_notes_to_text_activate), gpointer(this));
    if (window_menu->notes_preferences)
      g_signal_connect ((gpointer) window_menu->notes_preferences, "activate", G_CALLBACK (on_notes_preferences_activate), gpointer(this));
    if (window_menu->printingprefs)
      g_signal_connect ((gpointer) window_menu->printingprefs, "activate", G_CALLBACK (on_printingprefs_activate), gpointer(this));
    if (window_menu->reference_exchange1)
      g_signal_connect ((gpointer) window_menu->reference_exchange1, "activate", G_CALLBACK (on_reference_exchange1_activate), gpointer(this));
    if (window_menu->ignored_references1)
      g_signal_connect ((gpointer) window_menu->ignored_references1, "activate", G_CALLBACK (on_ignored_references1_activate), gpointer(this));
    if (window_menu->prefs_books)
      g_signal_connect ((gpointer) window_menu->prefs_books, "activate", G_CALLBACK (on_prefs_books_activate), gpointer(this));
    if (window_menu->preferences_windows_outpost)
      g_signal_connect ((gpointer) window_menu->preferences_windows_outpost, "activate", G_CALLBACK (on_preferences_windows_outpost_activate), gpointer(this));
    if (window_menu->preferences_tidy_text)
      g_signal_connect ((gpointer) window_menu->preferences_tidy_text, "activate", G_CALLBACK (on_preferences_tidy_text_activate), gpointer(this));
    if (window_menu->preferences_remote_git_repository)
      g_signal_connect ((gpointer) window_menu->preferences_remote_git_repository, "activate", G_CALLBACK (on_preferences_remote_git_repository_activate), gpointer(this));
    if (window_menu->preferences_features)
      g_signal_connect ((gpointer) window_menu->preferences_features, "activate", G_CALLBACK (on_preferences_features_activate), gpointer(this));
    if (window_menu->preferences_password)
      g_signal_connect ((gpointer) window_menu->preferences_password, "activate", G_CALLBACK (on_preferences_password_activate), gpointer(this));
    if (window_menu->preferences_text_replacement)
      g_signal_connect ((gpointer) window_menu->preferences_text_replacement, "activate", G_CALLBACK (on_preferences_text_replacement_activate), gpointer(this));
    if (window_menu->pdf_viewer1)
      g_signal_connect ((gpointer) window_menu->pdf_viewer1, "activate", G_CALLBACK (on_pdf_viewer1_activate), gpointer(this));
    if (window_menu->preferences_reporting)
      g_signal_connect ((gpointer) window_menu->preferences_reporting, "activate", G_CALLBACK (on_preferences_reporting_activate), gpointer(this));
    if (window_menu->preferences_planning)
      g_signal_connect ((gpointer) window_menu->preferences_planning, "activate", G_CALLBACK (on_preferences_planning_activate), gpointer(this));
    if (window_menu->preferences_graphical_interface)
      g_signal_connect ((gpointer) window_menu->preferences_graphical_interface, "activate", G_CALLBACK (on_preferences_graphical_interface_activate), gpointer(this));
    if (window_menu->preferences_filters)
      g_signal_connect ((gpointer) window_menu->preferences_filters, "activate", G_CALLBACK (on_preferences_filters_activate), gpointer(this));
    if (window_menu->help_context)
      g_signal_connect ((gpointer) window_menu->help_context, "activate", G_CALLBACK (on_help_context_activate), gpointer(this));
    if (window_menu->help_main)
      g_signal_connect ((gpointer) window_menu->help_main, "activate", G_CALLBACK (on_help_main_activate), gpointer(this));
    if (window_menu->system_log1)
      g_signal_connect ((gpointer) window_menu->system_log1, "activate", G_CALLBACK (on_system_log1_activate), gpointer(this));
    if (window_menu->about1)
      g_signal_connect ((gpointer) window_menu->about1, "activate", G_CALLBACK (on_about1_activate), gpointer(this));
    navigation.build(window_menu->toolbar1);
    g_signal_connect ((gpointer) navigation.reference_signal_delayed, "clicked", G_CALLBACK (on_navigation_new_reference_clicked), gpointer(this));
  }
  window_menu->present();
}

void MainWindow::on_window_menu_delete_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_menu_delete_button();
}

void MainWindow::on_window_menu_delete_button() {
  gtk_main_quit();
}

void MainWindow::on_open1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->open();
}

void MainWindow::open()
// Do the logic for opening a project.
{
  // Get new project, bail out if none.
  ustring newproject;
  if (!project_select(newproject))
    return;
  // Open editor.
  on_file_project_open(newproject);
}

void MainWindow::on_new1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->newproject();
}

void MainWindow::newproject() {
  git_command_pause(true);
  ProjectDialog projectdialog(true);
  if (projectdialog.run() == GTK_RESPONSE_OK) {
    on_file_project_open(projectdialog.newprojectname);
  }
  git_command_pause(false);
}

void MainWindow::on_properties1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->editproject();
}

void MainWindow::editproject() {
  git_command_pause(true);
  save_editors();
  // Show project dialog.
  ProjectDialog projectdialog(false);
  if (projectdialog.run() == GTK_RESPONSE_OK) {
    // Get focused project window.
    WindowEditor * editor_window = last_focused_editor_window();
    if (editor_window) {
      // Reload dictionaries.
      editor_window->editor->load_dictionaries();
    }
    // As anything could have been changed to the project, reopen it.
    reload_project();
  }
  git_command_pause(false);
}

void MainWindow::on_delete1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->deleteproject();
}

void MainWindow::deleteproject() {
  // Get all projects, leave the current one and the non-editable ones out.
  extern Settings * settings;
  vector <ustring> all_projects = projects_get_all();
  vector<ustring> projects;
  for (unsigned int i = 0; i < all_projects.size(); i++) {
    bool include = true;
    if (all_projects[i] == settings->genconfig.project_get())
      include = false;
    ProjectConfiguration * projectconfig = settings->projectconfig(all_projects[i]);
    if (!projectconfig->editable_get())
      include = false;
    if (include)
      projects.push_back(all_projects[i]);
  }
  // User interface.
  ListviewDialog dialog("Delete project", projects, "", true, NULL);
  if (dialog.run() == GTK_RESPONSE_OK) {
    int result;
    result = gtkw_dialog_question(mainwindow, "Are you sure you want to delete project " + dialog.focus + "?");
    if (result == GTK_RESPONSE_YES) {
      result = gtkw_dialog_question(mainwindow, "Are you really sure to delete project " + dialog.focus + ", something worth perhaps years of work?");
    }
    if (result == GTK_RESPONSE_YES) {
      project_delete(dialog.focus);
    }
  }
}

void MainWindow::on_quit1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  gtk_main_quit();
}

void MainWindow::on_system_log1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->viewlog();
}

void MainWindow::viewlog() {
  ShowScriptDialog showscript(0);
  showscript.run();
}

void MainWindow::on_help_context_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_help_context();
}

void MainWindow::on_help_context() {
  help_open(NULL, gpointer ("none"));
}

void MainWindow::on_help_main_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_help_main();
}

void MainWindow::on_help_main() {
  htmlbrowser("localhost:51516", true);
}

void MainWindow::on_about1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->showabout();
}

void MainWindow::showabout() {
  gtk_show_about_dialog(GTK_WINDOW (mainwindow),
  "version", PACKAGE_VERSION,
  "website", PACKAGE_BUGREPORT,
  "copyright", "Copyright (©) 2003-2008 Teus Benschop",
  "license", "This program is free software; you can redistribute it and/or modify\n"
  "it under the terms of the GNU General Public License as published by\n"
  "the Free Software Foundation; either version 3 of the License, or\n"
  "(at your option) any later version.\n"
  "\n"
  "This program is distributed in the hope that it will be useful,\n"
  "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
  "GNU General Public License for more details.\n"
  "\n"
  "You should have received a copy of the GNU General Public License\n"
  "along with this program; if not, write to the Free Software\n"
  "Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.\n",
  NULL);
}

void MainWindow::on_undo1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->menu_undo();
}

void MainWindow::menu_undo()
// Called for undo.
{
  /* 
   Editor * editor = editorsgui->focused_editor();
   if (editor) {
   if (editor->has_focus()) {
   editor->undo();
   }
   }
   */
  if (window_notes && (now_focused_signal_button == NULL) && (last_focused_signal_button == window_notes->focus_in_signal_button)) {
    //window_notes->undo();
  }
}

void MainWindow::on_redo1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->menu_redo();
}

void MainWindow::menu_redo()
// Called for redo.
{
  /*
   Editor * editor = editorsgui->focused_editor();
   if (editor) {
   if (editor->has_focus()) {
   editor->redo();
   }
   }
   */
  if (window_notes && (now_focused_signal_button == NULL) && (last_focused_signal_button == window_notes->focus_in_signal_button)) {
    //window_notes->redo();
  }
}

void MainWindow::on_edit1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->menu_edit();
}

void MainWindow::menu_edit() {
  // Set the sensitivity of some items in the Edit menu.
  /* 
   Editor * editor = editorsgui->focused_editor();
   if (editor) {
   gtk_widget_set_sensitive(copy_without_formatting, editor->has_focus());
   //gtk_widget_set_sensitive(undo1, editor->can_undo());
   //gtk_widget_set_sensitive(redo1, editor->can_redo());
   } else {
   gtk_widget_set_sensitive(undo1, true);
   gtk_widget_set_sensitive(redo1, true);
   }
   */

  // Sensitivity of the clipboard operations.
  // There is also the "owner-change" signal of the clipboard, but this is not
  // a reliable indicator for pastable content.
  bool cut = true;
  bool copy = true;
  bool paste = true;
  gtk_widget_set_sensitive(window_menu->cut1, cut);
  gtk_widget_set_sensitive(window_menu->copy1, copy);
  gtk_widget_set_sensitive(window_menu->paste1, paste);

  // Enable/disable based on whether we're editing a note.
  bool enable = (window_notes && window_notes->note_being_edited());
  // References can only be taken from a note when it is opened.
  gtk_widget_set_sensitive(window_menu->get_references_from_note, enable);

  // The Bible notes can only be edited when the cursor is in a note text.
  enable = false;
  WindowEditor * editor_window = last_focused_editor_window();
  if (editor_window)
    if (editor_window->editor->last_focused_type() == etvtNote)
      enable = true;
  gtk_widget_set_sensitive(window_menu->edit_bible_note, enable);
}

void MainWindow::on_find_and_replace1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->menu_replace();
}

void MainWindow::menu_replace() {
  // Before finding, save the current file.
  save_editors();
  // Display references.
  show_references_window();
  // Start find/replace dialog.
  vector <Reference> results;
  {
    ReplaceDialog replacedialog(0);
    if (replacedialog.run() == GTK_RESPONSE_OK) {
      results.assign(replacedialog.results.begin(), replacedialog.results.end());
      if (window_references) {
        window_references->display(replacedialog.results);
      }
    } else {
      return;
    }
  }
  // Replace text.
  if (results.size()) {
    ReplacingDialog replacedialog(results);
    replacedialog.run();
    reload_project();
  } else {
    gtkw_dialog_info(mainwindow, "There was nothing to replace");
  }
}

void MainWindow::on_findspecial1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->menu_findspecial();
}

void MainWindow::menu_findspecial() {
  // Before finding, save the current file.
  save_editors();
  // Display the references window.
  show_references_window();
  // Start dialog.
  {
    SearchSpecialDialog dialog(&bibletime);
    if (dialog.run() != GTK_RESPONSE_OK)
      return;
  }
  // Carry out the search. 
  search_string(window_references->liststore, window_references->treeview, window_references->treecolumn, &bibletime);
}

void MainWindow::on_import1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->menu_import();
}

void MainWindow::menu_import() {
  bool structured, raw;
  import_dialog_selector(structured, raw);
  if (structured) {
    ImportTextDialog dialog(0);
    if (dialog.run() != GTK_RESPONSE_OK)
      return;
  }
  if (raw) {
    ImportRawTextDialog dialog(0);
    if (dialog.run() != GTK_RESPONSE_OK)
      return;
  }
  reload_project();
}

gboolean MainWindow::on_mainwindow_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
  /*
   This solves a bug:
   When quitting the program though the menu, all went fine.
   But when quitting the program by clicking on the cross at the top of the program,
   a segmentation fault occurred.
   This segmentation occurred in the destructor of the object MainWindow.
   In the destructor properties of GTK were accessed, which obviously were 
   already in the process of being destroyed.
   This is the solution:
   When the window receives the delete_event (somebody clicks on the cross to
   close the program), the window is not closed, but instead a timeout is 
   installed that calls qtk_main_quit after a short while.
   The destructor of the object MainWindow can not access the properties of GTK
   without a segmentation fault.
   */
  g_timeout_add(10, GSourceFunc(gtk_main_quit), user_data);
  return true;
}

void MainWindow::on_insert1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_menu_insert();
}

void MainWindow::on_menu_insert()
// Sets the labels of the underlying menu items right.
{
  // Get the focused editor window.
  WindowEditor * editor_window = last_focused_editor_window();
  // Write the proper labels.
  extern Settings * settings;
  ustring std_txt = "Standard text ";
  ustring label;
  label = std_txt + "_1: " + settings->genconfig.edit_note_standard_text_one_get();
  if (window_menu->standard_text_1)
    gtk_label_set_text_with_mnemonic(GTK_LABEL (gtk_bin_get_child (GTK_BIN (window_menu->standard_text_1))), label.c_str());
  label = std_txt + "_2: " + settings->genconfig.edit_note_standard_text_two_get();
  if (window_menu->standard_text_2)
    gtk_label_set_text_with_mnemonic(GTK_LABEL (gtk_bin_get_child (GTK_BIN (window_menu->standard_text_2))), label.c_str());
  label = std_txt + "_3: " + settings->genconfig.edit_note_standard_text_three_get();
  if (window_menu->standard_text_3)
    gtk_label_set_text_with_mnemonic(GTK_LABEL (gtk_bin_get_child (GTK_BIN (window_menu->standard_text_3))), label.c_str());
  label = std_txt + "_4: " + settings->genconfig.edit_note_standard_text_four_get();
  if (window_menu->standard_text_4)
    gtk_label_set_text_with_mnemonic(GTK_LABEL (gtk_bin_get_child (GTK_BIN (window_menu->standard_text_4))), label.c_str());
  // Enable or disable depending on situation.
  bool enable = (window_notes && window_notes->note_being_edited());
  if (window_menu->standard_text_1)
    gtk_widget_set_sensitive(window_menu->standard_text_1, enable);
  if (window_menu->standard_text_2)
    gtk_widget_set_sensitive(window_menu->standard_text_2, enable);
  if (window_menu->standard_text_3)
    gtk_widget_set_sensitive(window_menu->standard_text_3, enable);
  if (window_menu->standard_text_4)
    gtk_widget_set_sensitive(window_menu->standard_text_4, enable);

  // Allow inserting reference when we edit a note and the reference is different 
  // from any of the references loaded already.
  enable = (window_notes && window_notes->note_being_edited());
  if (enable) {
    // Get all references from the note.
    vector<Reference> references;
    vector<ustring> messages;
    window_notes->get_references_from_note(references, messages);
    // See whether the current reference is already in it.
    bool already_in = false;
    for (unsigned int i = 0; i < references.size(); i++) {
      if (editor_window)
        if (references[i].equals(editor_window->editor->current_reference))
          already_in = true;
    }
    // If the reference is not yet in the note's references, enable menu, so user can add it.
    enable = !already_in;
  }
  // Update menu.
  ProjectConfiguration * projectconfig = settings->projectconfig(settings->genconfig.project_get());
  if (editor_window) {
    label = "_Add " + editor_window->editor->current_reference.human_readable(projectconfig->language_get()) + " to project note";
  } else {
    enable = false;
  }

  if (window_menu->current_reference1)
    gtk_label_set_text_with_mnemonic(GTK_LABEL (gtk_bin_get_child (GTK_BIN (window_menu->current_reference1))), label.c_str());
  if (window_menu->current_reference1)
    gtk_widget_set_sensitive(window_menu->current_reference1, enable);

  // Inserting special character.
  gtk_widget_set_sensitive(window_menu->insert_special_character, (editor_window && (now_focused_signal_button == editor_window->focus_in_signal_button)));
}

void MainWindow::on_menuitem_view_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_menuitem_view();
}

void MainWindow::on_menuitem_view() {
}

void MainWindow::on_notes_preferences_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_notes_preferences();
}

void MainWindow::on_notes_preferences() {
  NotesDialog dialog(0);
  dialog.run();
}

void MainWindow::on_copy_project_to_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_copy_project_to();
}

void MainWindow::on_copy_project_to()
// Copy project to another one.
{
  save_editors();
  extern Settings * settings;
  EntryDialog dialog("New project name", "Enter a name of a non-existent project\nwhere this project will be copied to.", settings->genconfig.project_get());
  if (dialog.run() == GTK_RESPONSE_OK) {
    // Does the project exist?
    if ((project_exists(dialog.entered_value)) || (dialog.entered_value == "data")) {
      // Yes, give message that project exists.
      ustring error = "Project ";
      error.append(dialog.entered_value);
      error.append(" already exists.");
      error.append("\nIf you still intend to copy the project,");
      error.append("\ndelete project ");
      error.append(dialog.entered_value);
      error.append(" first.");
      gtkw_dialog_error(mainwindow, error);
    } else {
      // Ok, go ahead with the copy.
      project_copy(settings->genconfig.project_get(), dialog.entered_value);
      // Give message when through.
      ustring message;
      message.append("The project has been copied to a new project\n");
      message.append("named ");
      message.append(dialog.entered_value);
      message.append(".");
      gtkw_dialog_info(mainwindow, message);
    }
  }
}

void MainWindow::on_compare_with1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_compare_with();
}

void MainWindow::on_compare_with()
// Compare the current project with another one.
{
  save_editors();
  show_references_window();
  git_command_pause(true);
  References references(window_references->liststore, window_references->treeview, window_references->treecolumn);
  CompareDialog dialog(&references);
  dialog.run();
  git_command_pause(false);
}

void MainWindow::on_printingprefs_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_printing_preferences();
}

void MainWindow::on_printing_preferences() {
  PrintPreferencesDialog dialog(0);
  dialog.run();
}

void MainWindow::on_prefs_books_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_prefs_books();
}

void MainWindow::on_prefs_books() {
  extern Settings * settings;
  BookDialog dialog(settings->genconfig.project_get());
  if (dialog.run() == GTK_RESPONSE_OK) {
    reload_project();
  }
}

void MainWindow::on_preferences_tidy_text_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_preferences_tidy_text();
}

void MainWindow::on_preferences_tidy_text() {
  TidyDialog dialog(0);
  dialog.run();
}

/*
 |
 |
 |
 |
 |
 Navigation
 |
 |
 |
 |
 |
 */

void MainWindow::on_navigation_new_reference_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_navigation_new_reference();
}

void MainWindow::on_navigation_new_reference() {
  extern Settings * settings;
  settings->genconfig.book_set(navigation.reference.book);
  settings->genconfig.chapter_set(convert_to_string(navigation.reference.chapter));
  settings->genconfig.verse_set(navigation.reference.verse);
  go_to_new_reference();
  // Optionally display the parallel passages in the reference area.
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (window_menu->parallel_passages1))) {
    show_references_window();
    parallel_passages_display(navigation.reference, window_references->liststore, window_references->treeview, window_references->treecolumn);
  }
  // Optional displaying keyterms in verse.
  if (window_show_keyterms) {
    window_show_keyterms->go_to(settings->genconfig.project_get(), navigation.reference);
  }
  // Optional resource windows.
  for (unsigned int i = 0; i < resource_windows.size(); i++) {
    resource_windows[i]->go_to(navigation.reference);
  }
  // Optional outline window.
  if (window_outline) {
    window_outline->go_to(settings->genconfig.project_get(), navigation.reference);
  }
}

void MainWindow::goto_next_verse() {
  navigation.nextverse();
}

void MainWindow::goto_previous_verse() {
  navigation.previousverse();
}

void MainWindow::goto_next_chapter() {
  navigation.nextchapter();
}

void MainWindow::goto_previous_chapter() {
  navigation.previouschapter();
}

void MainWindow::goto_next_book() {
  navigation.nextbook();
}

void MainWindow::goto_previous_book() {
  navigation.previousbook();
}

void MainWindow::goto_reference_interactive() {
  WindowEditor * editor_window = last_focused_editor_window();
  if (editor_window) {
    Editor * editor = editor_window->editor;
    GotoReferenceDialog dialog(editor->current_reference.book, editor->current_reference.chapter, editor->current_reference.verse);
    if (dialog.run() == GTK_RESPONSE_OK) {
      if (dialog.newreference) {
        // If the dialog closes, then another window will receive focus again.
        // This focusing causes the navigation to take the values as they are in the configuration.
        // This would frustrate the desire of the user to go somewhere else.
        // To fix the problem, the settings are updated here.
        extern Settings * settings;
        settings->genconfig.book_set(dialog.reference.book);
        settings->genconfig.chapter_set(convert_to_string(dialog.reference.chapter));
        settings->genconfig.verse_set(dialog.reference.verse);
        navigation.display(dialog.reference);
      }
    }
  }
}

void MainWindow::go_to_new_reference()
// This starts the procedure to carries out a requested change of reference.
{
  // Let the editor(s) show the right reference.
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    editor_windows[i]->go_to(navigation.reference);
  }

  // Create a reference for the external programs.
  // These do not take verses like 10a or 10-12, but only numbers like 10 or 12.
  Reference goto_reference(navigation.reference.book, navigation.reference.chapter, number_in_string(navigation.reference.verse));

  // Send it to the external programs.
  extern Settings * settings;
  if (settings->genconfig.reference_exchange_send_to_bibleworks_get())
    windowsoutpost->BibleWorksReferenceSet(goto_reference);
  if (settings->genconfig.reference_exchange_send_to_santafefocus_get())
    windowsoutpost->SantaFeFocusReferenceSet(goto_reference);

  // Send to resources.
  for (unsigned int i = 0; i < resource_windows.size(); i++) {
    resource_windows[i]->go_to(goto_reference);
  }
  // Update the notes view.
  notes_redisplay();
}

void MainWindow::on_new_verse_signalled(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_new_verse();
}

void MainWindow::on_new_verse()
/*
 When the cursor has moved, the navigation system needs to be updated
 so that it shows the right reference. If the user was, for example
 on Matthew 1:10, and the cursor moves, the move might have brought him
 to another reference, though this is not necessarily so. Therefore, as we 
 don't know where the user is now after the cursor moved, we need to find
 it out. The book is known, the chapter is known, because both stay the same.
 The only thing we don't know is the verse. 
 */
{
  WindowEditor * editor_window = last_focused_editor_window();
  if (editor_window) {
    Editor * editor = editor_window->editor;
    Reference reference(navigation.reference.book, navigation.reference.chapter, editor->current_verse_number);
    navigation.display(reference);
    if (window_outline)
      window_outline->go_to(editor->project, navigation.reference);
  }
}

void MainWindow::on_synchronize_other_programs2_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_synchronize_other_programs();
}

void MainWindow::on_synchronize_other_programs() {
  // Get the focused editor. If none, bail out.
  WindowEditor * editor_window = last_focused_editor_window();
  if (!editor_window)
    return;
  // Send the reference.
  extern Settings * settings;
  if (settings->genconfig.reference_exchange_send_to_bibleworks_get()) {
    windowsoutpost->BibleWorksReferenceSet(editor_window->editor->current_reference);
  }
  if (settings->genconfig.reference_exchange_send_to_santafefocus_get()) {
    windowsoutpost->SantaFeFocusReferenceSet(editor_window->editor->current_reference);
  }
  if (settings->genconfig.reference_exchange_send_to_bibletime_get()) {
    bibletime.sendreference(editor_window->editor->current_reference);
  }
  for (unsigned int i = 0; i < resource_windows.size(); i++) {
    resource_windows[i]->go_to(editor_window->editor->current_reference);
  }
}

void MainWindow::on_text_area_activate() {
  WindowEditor * editor_window = last_focused_editor_window();
  if (editor_window) {
    editor_window->present();
  }
}

void MainWindow::on_tools_area_activate() {
}

void MainWindow::on_notes_area_activate() {
  view_project_notes();
  notes_redisplay();
}

/*
 |
 |
 |
 |
 |
 Clipboard
 |
 |
 |
 |
 |
 */

void MainWindow::on_cut1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_cut();
}

void MainWindow::on_cut() {
  /* 
   GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
   if (editorsgui->has_focus()) {
   Editor * editor = editorsgui->focused_editor();
   if (editor) {
   gtk_clipboard_set_text(clipboard, editor->text_get_selection ().c_str(), -1);
   editor->text_erase_selection();
   }
   }
   if (window_notes && (now_focused_signal_button == NULL) && (last_focused_signal_button == window_notes->focus_in_signal_button)) {
   //window_notes->cut();
   }
   */
}

void MainWindow::on_copy1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_copy();
}

void MainWindow::on_copy() {
  /* 
   GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
   if (editorsgui->has_focus()) {
   Editor * editor = editorsgui->focused_editor();
   if (editor) {
   // In case of the text editor, the USFM code is copied, not the plain text. 
   gtk_clipboard_set_text(clipboard, editor->text_get_selection ().c_str(), -1);
   }
   }
   if (window_notes && (now_focused_signal_button == NULL) && (last_focused_signal_button == window_notes->focus_in_signal_button)) {
   //window_notes->copy();
   }
   if (window_check_keyterms) {
   window_check_keyterms->copy_clipboard();
   }
   */
}

void MainWindow::on_copy_without_formatting_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_copy_without_formatting();
}

void MainWindow::on_copy_without_formatting() {
  /* 
   GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
   if (editorsgui->has_focus()) {
   Editor * editor = editorsgui->focused_editor();
   if (editor) {
   gtk_text_buffer_copy_clipboard(editor->last_focused_textbuffer(), clipboard);
   }
   }
   */
}

void MainWindow::on_paste1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_paste();
}

void MainWindow::on_paste() {
  // Get the clipboard.
  GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  // Bail out if no text is available.
  if (!gtk_clipboard_wait_is_text_available(clipboard))
    return;
  // Paste text in the focused textview.  
  /* 
   if (editorsgui->has_focus()) {
   Editor * editor = editorsgui->focused_editor();
   if (editor) {
   gchar * text = gtk_clipboard_wait_for_text(clipboard);
   if (text) {
   editor->text_insert(text);
   g_free(text);
   }
   }
   }
   if (window_notes && (now_focused_signal_button == NULL) && (last_focused_signal_button == window_notes->focus_in_signal_button)) {
   //window_notes->paste();
   }
   */
}

/*
 |
 |
 |
 |
 |
 References
 |
 |
 |
 |
 |
 */

void MainWindow::show_references_window() {
  if (!window_references) {
    window_references = new WindowReferences (accelerator_group, windows_startup_pointer != G_MAXINT);
    g_signal_connect ((gpointer) window_references->delete_signal_button, "clicked", G_CALLBACK (on_window_references_delete_button_clicked), gpointer(this));
    g_signal_connect ((gpointer) window_references->focus_in_signal_button, "clicked", G_CALLBACK (on_window_focus_button_clicked), gpointer(this));
    g_signal_connect ((gpointer) window_references->general_signal_button, "clicked", G_CALLBACK (on_window_references_general_signal_button_clicked), gpointer(this));
  }
}

void MainWindow::on_window_references_delete_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_references_delete_button();
}

void MainWindow::on_window_references_delete_button() {
  if (window_references) {
    delete window_references;
    window_references = NULL;
  }
}

void MainWindow::on_window_references_general_signal_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_references_general_signal_button();
}

void MainWindow::on_window_references_general_signal_button()
// This routine is called when the reference window fires a signal that something has happened.
{
  switch (window_references->action)
  {
    case wratReferenceActivated:
    {
      on_list_goto();
      break;
    }
    case wratPopupMenu:
    {
      gtk_menu_popup(GTK_MENU (window_menu->file_references_menu), NULL, NULL, NULL, NULL, window_references->popup_button, window_references->popup_event_time);
      break;
    }
    case wratReferencesSelected:
    {
      treeview_references_display_quick_reference();
      break;
    }
  }
}

void MainWindow::on_list_goto()
// Handler for when the user pressed Enter in the list and wants to go to a reference.
{
  // Get the editor window. If none, bail out.
  WindowEditor * editor_window = last_focused_editor_window();
  if (!editor_window)
    return;

  // Bail out if there's no references window.
  if (!window_references)
    return;

  // Jump to the reference.
  navigation.display(window_references->reference);
  editor_window->editor->go_to_new_reference_highlight = true;
}

void MainWindow::on_open_references1_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_open_references();
}

void MainWindow::on_open_references() {
  show_references_window();
  window_references->open();
}

void MainWindow::on_references_save_as_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_save_references();
}

void MainWindow::on_save_references() {
  show_references_window();
  window_references->save();
}

void MainWindow::on_close_references_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_clear_references();
}

void MainWindow::on_clear_references() {
  show_references_window();
  window_references->clear();
}

void MainWindow::on_delete_references_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_delete_references();
}

void MainWindow::on_delete_references() {
  show_references_window();
  window_references->dismiss();
}

void MainWindow::on_next_reference()
// This goes to the next reference, if there is any.
// If no item has been selected it chooses the first, if it's there.
{
  // Display references.
  show_references_window();
  // Select next reference in the list.
  References references(window_references->liststore, window_references->treeview, window_references->treecolumn);
  references.goto_next();
  // Actually open the reference in the editor.
  window_references->activate();
}

void MainWindow::on_previous_reference()
// This goes to the previous reference, if there is any.
// If no item has been selected it chooses the first, if it's there.
{
  // Show references.
  show_references_window();
  // Select previous reference in the list.
  References references(window_references->liststore, window_references->treeview, window_references->treecolumn);
  references.goto_previous();
  // Actually open the reference in the editor.
  window_references->activate();
}

void MainWindow::on_ignored_references1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_ignored_references();
}

void MainWindow::on_ignored_references() {
  vector<ustring> hidden_references = references_hidden_ones_load();
  EditListDialog dialog( &hidden_references, "Hidden references", "of references and comments that will never be shown in the reference area.", true, false, true, false, false, false, false, NULL);
  if (dialog.run() == GTK_RESPONSE_OK)
    references_hidden_ones_save(hidden_references);
}

void MainWindow::on_reference_hide_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_reference_hide();
}

void MainWindow::on_reference_hide()
// Deals with hiding references.
{
  show_references_window();
  window_references->hide();
}

/*
 |
 |
 |
 |
 |
 Bibledit Windows Outpost
 BibleTime
 |
 |
 |
 |
 |
 */

bool MainWindow::mainwindow_on_external_programs_timeout(gpointer data) {
  return ((MainWindow *) data)->on_external_programs_timeout();
}

bool MainWindow::on_external_programs_timeout() {
  // Deal with exchanging references between Bibledit and BibleTime.
  // The trick of the trade here is that we look which of the two made a change.
  // If Bibledit made a change, then it sends its reference to BibleTime.
  // And vice versa.
  // This avoids race conditions, i.e. they keep sending
  // references to each other, and the reference keeps changing between the 
  // one Bibledit had and the one BibleTime had.

  extern Settings * settings;
  // A reference is taken from BibleTime only when it changed reference.
  if (settings->genconfig.reference_exchange_receive_from_bibletime_get()) {
    Reference btreference(0);
    if (bibletime.getreference(btreference)) {
      ustring new_reference = btreference.human_readable("");
      if (new_reference != bibletime_previous_reference) {
        bibletime_previous_reference = new_reference;
        // As BibleTime does not support chapter 0 or verse 0, let Bibledit not 
        // receive any reference from it if it is on such a chapter or verse.
        bool receive = true;
        if (navigation.reference.chapter == 0)
          receive = false;
        if (navigation.reference.verse == "0")
          receive = false;
        if (receive)
          navigation.display(btreference);
        got_new_bt_reference = 5;
      }
    }
  }

  // The reference is sent to BibleTime only after it changed.
  // But when we received a new reference, nothing will be sent, 
  // to avoid the race condition that both Bibledit and BibleTime keep 
  // alternating between two references on their own.
  if (got_new_bt_reference > 0)
    got_new_bt_reference--;
  if (got_new_bt_reference == 0) {
    WindowEditor * editor_window = last_focused_editor_window();
    if (editor_window) {
      if (settings->genconfig.reference_exchange_send_to_bibletime_get()) {
        ustring bibledit_bt_new_reference = convert_to_string(editor_window->editor->current_reference.book) + convert_to_string(editor_window->editor->current_reference.chapter)
            + editor_window->editor->current_reference.verse;
        if (bibledit_bt_new_reference != bibledit_bt_previous_reference) {
          bibledit_bt_previous_reference = bibledit_bt_new_reference;
          bibletime.sendreference(editor_window->editor->current_reference);
        }
      }
    }
  }

  return true;
}

void MainWindow::on_reference_exchange1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_reference_exchange();
}

void MainWindow::on_reference_exchange() {
  ReferenceExchangeDialog dialog(0);
  dialog.run();
}

void MainWindow::on_send_word_to_toolbox_signalled(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->send_word_to_toolbox();
}

void MainWindow::send_word_to_toolbox() {
  WindowEditor * editor_window = last_focused_editor_window();
  if (!editor_window)
    return;
  ustring word = editor_window->editor->word_double_clicked_text;
  if (word.empty())
    return;
  gw_message("Sending to Toolbox: " + word);
  windowsoutpost->SantaFeFocusWordSet(word);
}

void MainWindow::on_preferences_windows_outpost_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_preferences_windows_outpost();
}

void MainWindow::on_preferences_windows_outpost() {
  // Dialog for making settings.
  OutpostDialog dialog(0);
  dialog.run();
  if (dialog.changed) {
    // Changes were made: destroy and recreate the object.
    delete windowsoutpost;
    windowsoutpost = new WindowsOutpost (true);
    // If required, start the Outpost.
    extern Settings * settings;
    if (settings->genconfig.use_outpost_get()) {
      // Delay a bit so wine debugger doesn't start.
      g_usleep(1000000);
      windowsoutpost->Start();
    }
  }
}

/*
 |
 |
 |
 |
 |
 Title bar and status bar
 |
 |
 |
 |
 |
 */

bool MainWindow::on_gui_timeout(gpointer data) {
  ((MainWindow *) data)->on_gui();
  return true;
}

void MainWindow::on_gui()
// Tasks related to the GUI.
{
  // Display information about the number of Git tasks to be done if there are many of them.
  {
    ustring git;
    if (git_tasks_count() >= 100)
      git = "Git tasks pending: " + convert_to_string(git_tasks_count());
    ustring currenttext = gtk_label_get_text(GTK_LABEL (window_menu->label_git));
    if (git != currenttext) {
      gtk_label_set_text(GTK_LABEL (window_menu->label_git), git.c_str());
    }
  }

  // Check whether to reopen the project.
  on_git_reopen_project();

  // Care for possible restart.
  extern Settings * settings;
  if (settings->session.restart) {
    gtk_main_quit();
  }
}

/*
 |
 |
 |
 |
 |
 Project notes
 |
 |
 |
 |
 |
 */

void MainWindow::view_project_notes() {
  if (!project_notes_enabled)
    return;
  if (window_notes) {
    // If the window is there, present it to the user.
    window_notes->present();
  } else {
    window_notes = new WindowNotes (accelerator_group, windows_startup_pointer != G_MAXINT);
    g_signal_connect ((gpointer) window_notes->delete_signal_button, "clicked", G_CALLBACK (on_window_notes_delete_button_clicked), gpointer(this));
    g_signal_connect ((gpointer) window_notes->focus_in_signal_button, "clicked", G_CALLBACK (on_window_focus_button_clicked), gpointer(this));
    treeview_references_display_quick_reference();
  }
}

void MainWindow::on_window_notes_delete_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_notes_delete_button();
}

void MainWindow::on_window_notes_delete_button() {
  if (window_notes) {
    delete window_notes;
    window_notes = NULL;
  }
}

void MainWindow::on_new_note_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_new_note();
}

void MainWindow::on_new_note() {
  // Display notes.
  view_project_notes();
  // Create new note.
  if (window_notes)
    window_notes->new_note();
}

void MainWindow::on_delete_note_activate(GtkMenuItem *menuitem, gpointer user_data) {
  gtkw_dialog_info(((MainWindow *) user_data)->mainwindow, "A note can be deleted by clicking on the [delete] link in the notes view");
}

void MainWindow::on_viewnotes_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_view_notes();
}

void MainWindow::on_view_notes() {
  ShowNotesDialog dialog(0);
  if (dialog.run() == GTK_RESPONSE_OK) {
    view_project_notes();
    notes_redisplay();
  }
}

void MainWindow::notes_redisplay() {
  if (window_notes) {
    window_notes->redisplay();
  }
}

void MainWindow::on_find_in_notes1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->find_in_notes();
}

void MainWindow::find_in_notes() {
  FindNoteDialog findnotedialog(0);
  if (findnotedialog.run() == GTK_RESPONSE_OK) {
    view_project_notes();
    if (window_notes) {
      window_notes->display(findnotedialog.ids);
    }
  }
}

void MainWindow::on_import_notes_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_import_notes();
}

void MainWindow::on_import_notes() {
  ImportNotesDialog dialog(0);
  if (dialog.run() == GTK_RESPONSE_APPLY) {
    view_project_notes();
    notes_redisplay();
  }
}

void MainWindow::on_export_notes_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_export_notes();
}

void MainWindow::on_export_notes() {
  view_project_notes();
  int result;
  ustring filename;
  ExportNotesFormat format;
  bool save_all_notes;
  {
    ExportNotesDialog dialog(0);
    result = dialog.run();
    filename = dialog.filename;
    format = dialog.exportnotesformat;
    save_all_notes = dialog.save_all_notes;
  }
  if (result == GTK_RESPONSE_OK) {
    vector <unsigned int> ids_to_display;
    export_translation_notes(filename, format, ids_to_display, save_all_notes, mainwindow);
  }
}

void MainWindow::on_standard_text_1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_insert_standard_text(menuitem);
}

void MainWindow::on_standard_text_2_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_insert_standard_text(menuitem);
}

void MainWindow::on_standard_text_3_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_insert_standard_text(menuitem);
}

void MainWindow::on_standard_text_4_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_insert_standard_text(menuitem);
}

void MainWindow::on_insert_standard_text(GtkMenuItem *menuitem) {
  // Find out which standard text to insert, and where to insert it, and how.
  extern Settings * settings;
  ustring standardtext;
  unsigned int selector = 0;
  bool addspace = false;
  bool gtkhtml = false;
  if (menuitem == GTK_MENU_ITEM (window_menu->standard_text_1)) {
    standardtext = settings->genconfig.edit_note_standard_text_one_get();
    selector = 0;
    addspace = true;
    gtkhtml = true;
  } else if (menuitem == GTK_MENU_ITEM (window_menu->standard_text_2)) {
    standardtext = settings->genconfig.edit_note_standard_text_two_get();
    selector = 1;
    addspace = true;
    gtkhtml = true;
  } else if (menuitem == GTK_MENU_ITEM (window_menu->standard_text_3)) {
    standardtext = settings->genconfig.edit_note_standard_text_three_get();
    selector = 2;
    addspace = true;
    gtkhtml = true;
  } else if (menuitem == GTK_MENU_ITEM (window_menu->standard_text_4)) {
    standardtext = settings->genconfig.edit_note_standard_text_four_get();
    selector = 3;
    addspace = true;
    gtkhtml = true;
  } else if (menuitem == GTK_MENU_ITEM (window_menu->current_reference1)) {
    WindowEditor * editor_window = last_focused_editor_window();
    if (editor_window) {
      Editor * editor = editor_window->editor;
      standardtext = books_id_to_english(editor->current_reference.book) + " " + convert_to_string(editor->current_reference.chapter) + ":" + editor->current_reference.verse;
      selector = 4;
      addspace = false;
      gtkhtml = false;
    }
  }

  // Insert the text.
  if (window_notes) {
    window_notes->insert_standard_text(selector);
  }
}

void MainWindow::on_current_reference1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_insert_standard_text(menuitem);
}

void MainWindow::on_get_references_from_note_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_get_references_from_note();
}

void MainWindow::on_get_references_from_note() {
  // Store references.
  vector<Reference> references;
  // Store possible messages here, but they will be dumped.
  vector<ustring> messages;
  // Get all references from the editor.
  if (window_notes)
    window_notes->get_references_from_note(references, messages);
  // Sort the references so they appear nicely in the editor.
  sort_references(references);
  // Set none searchwords.
  extern Settings* settings;
  settings->session.highlights.clear();
  // Display the References
  show_references_window();
  // Set references.
  References references2(window_references->liststore, window_references->treeview, window_references->treecolumn);
  references2.set_references(references);
  ProjectConfiguration * projectconfig = settings->projectconfig(settings->genconfig.project_get());
  references2.fill_store(projectconfig->language_get());
}

void MainWindow::notes_get_references_from_id(gint id)
// Get the references from the note id
{
  // Store references we get.
  vector<Reference> references;

  // Fetch the references for the note from the database.
  sqlite3 *db;
  int rc;
  char *error= NULL;
  try
  {
    rc = sqlite3_open(notes_database_filename ().c_str (), &db);
    if (rc) throw runtime_error (sqlite3_errmsg(db));
    sqlite3_busy_timeout (db, 1000);
    SqliteReader sqlitereader (0);
    char * sql;
    sql = g_strdup_printf ("select ref_osis from %s where id = %d;", TABLE_NOTES, id);
    rc = sqlite3_exec(db, sql, sqlitereader.callback, &sqlitereader, &error);
    g_free (sql);
    if (rc != SQLITE_OK)
    throw runtime_error (error);
    if ((sqlitereader.ustring0.size()> 0))
    {
      ustring reference;
      reference = sqlitereader.ustring0[0];
      // Read the reference(s).
      Parse parse (reference, false);
      for (unsigned int i = 0; i < parse.words.size(); i++)
      {
        Reference ref (0);
        reference_discover (0, 0, "", parse.words[i], ref.book, ref.chapter, ref.verse);
        references.push_back (ref);
      }
    }
  }
  catch (exception & ex)
  {
    gw_critical (ex.what ());
  }
  // Close connection.  
  sqlite3_close(db);

  // Display the References
  show_references_window();
  // Set references.
  References references2(window_references->liststore, window_references->treeview, window_references->treecolumn);
  references2.set_references(references);
  extern Settings * settings;
  ProjectConfiguration * projectconfig = settings->projectconfig(settings->genconfig.project_get());
  references2.fill_store(projectconfig->language_get());
}

/*
 |
 |
 |
 |
 |
 Export
 |
 |
 |
 |
 |
 */

void MainWindow::on_export_usfm_files_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_export_usfm_files(false);
}

void MainWindow::on_export_zipped_unified_standard_format_markers1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_export_usfm_files(true);
}

void MainWindow::on_export_usfm_files(bool zipped) {
  save_editors();
  export_to_usfm(mainwindow, zipped);
}

void MainWindow::on_to_bibleworks_version_compiler_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_to_bibleworks_version_compiler();
}

void MainWindow::on_to_bibleworks_version_compiler() {
  save_editors();
  export_to_bibleworks(mainwindow);
}

void MainWindow::on_export_to_sword_module_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_export_to_sword_module();
}

void MainWindow::on_export_to_sword_module() {
  save_editors();
  export_to_sword_interactive();
  bibletime.reloadmodules();
}

void MainWindow::on_export_opendocument_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_export_opendocument();
}

void MainWindow::on_export_opendocument() {
  save_editors();
  export_to_opendocument(mainwindow);
}

/*
 |
 |
 |
 |
 |
 Checks
 |
 |
 |
 |
 |
 */

void MainWindow::on_check1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_menu_check();
}

void MainWindow::on_menu_check() {
  // Display the references.
  show_references_window();
}

void MainWindow::on_markers1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_menu_check_markers();
}

void MainWindow::on_menu_check_markers() {
}

void MainWindow::on_validate_usfms1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_menu_check_markers_validate();
}

void MainWindow::on_menu_check_markers_validate() {
  save_editors();
  show_references_window();
  scripture_checks_validate_usfms(window_references->liststore, window_references->treeview, window_references->treecolumn, NULL);
}

void MainWindow::on_count_usfms1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_menu_check_markers_count();
}

void MainWindow::on_menu_check_markers_count() {
  save_editors();
  scripture_checks_count_usfms(true);
}

void MainWindow::on_compare_usfm1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_menu_check_markers_compare();
}

void MainWindow::on_menu_check_markers_compare() {
  save_editors();
  show_references_window();
  scripture_checks_compare_usfms(window_references->liststore, window_references->treeview, window_references->treecolumn, NULL);
}

void MainWindow::on_chapters_and_verses1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_menu_check_chapters_and_verses();
}

void MainWindow::on_menu_check_chapters_and_verses() {
  save_editors();
  show_references_window();
  scripture_checks_chapters_verses(window_references->liststore, window_references->treeview, window_references->treecolumn, NULL);
}

void MainWindow::on_count_characters_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_count_characters();
}

void MainWindow::on_count_characters() {
  save_editors();
  scripture_checks_count_characters(true);
}

void MainWindow::on_unwanted_patterns_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_unwanted_patterns();
}

void MainWindow::on_unwanted_patterns() {
  save_editors();
  show_references_window();
  scripture_checks_unwanted_patterns(window_references->liststore, window_references->treeview, window_references->treecolumn, NULL);
}

void MainWindow::on_check_capitalization_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_check_capitalization();
}

void MainWindow::on_check_capitalization() {
  save_editors();
  show_references_window();
  scripture_checks_capitalization(window_references->liststore, window_references->treeview, window_references->treecolumn, NULL);
}

void MainWindow::on_check_repetition_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_check_repetition();
}

void MainWindow::on_check_repetition() {
  save_editors();
  show_references_window();
  scripture_checks_repetition(window_references->liststore, window_references->treeview, window_references->treecolumn, NULL);
}

void MainWindow::on_check_matching_pairs_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_check_matching_pairs();
}

void MainWindow::on_check_matching_pairs() {
  save_editors();
  show_references_window();
  scripture_checks_matching_pairs(window_references->liststore, window_references->treeview, window_references->treecolumn, NULL);
}

void MainWindow::on_unwanted_words_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_unwanted_words();
}

void MainWindow::on_unwanted_words() {
  save_editors();
  show_references_window();
  scripture_checks_unwanted_words(window_references->liststore, window_references->treeview, window_references->treecolumn, NULL);
}

void MainWindow::on_word_count_inventory_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_word_count_inventory();
}

void MainWindow::on_word_count_inventory() {
  save_editors();
  scripture_checks_word_inventory(true);
}

bool MainWindow::on_check_httpd_timeout(gpointer data) {
  ((MainWindow *) data)->on_check_httpd();
  return true;
}

void MainWindow::on_check_httpd() {
  // Does the httpd have a request for us?
  if (!httpd.search_whole_word.empty()) {
    // Bibledit presents itself and searches for the word.
    gtk_window_present(GTK_WINDOW (mainwindow));
    extern Settings * settings;
    settings->session.searchword = httpd.search_whole_word;
    httpd.search_whole_word.clear();
    settings->session.search_case_sensitive = true;
    settings->session.search_current_book = false;
    settings->session.search_current_chapter = false;
    settings->session.search_globbing = false;
    settings->session.search_start_word_match = true;
    settings->session.search_end_word_match = true;
    settings->session.search_page = 1;
    settings->session.searchresultstype = sstLoad;
    settings->session.highlights.clear();
    SessionHighlights
        sessionhighlights(settings->session.searchword, settings->session.search_case_sensitive, settings->session.search_globbing, settings->session.search_start_word_match, settings->session.search_end_word_match, atRaw, false, false, false, false, false, false, false, false);
    settings->session.highlights.push_back(sessionhighlights);
    show_references_window();
    search_string(window_references->liststore, window_references->treeview, window_references->treecolumn, &bibletime);
  }
  // Did the browser request a url too difficult for it to handle?
  if (!httpd.difficult_url.empty()) {
    htmlbrowser(httpd.difficult_url, true, true);
    httpd.difficult_url.clear();
  }
}

void MainWindow::on_my_checks_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_my_checks();
}

void MainWindow::on_my_checks() {
  save_editors();
  show_references_window();
  MyChecksDialog dialog(window_references->liststore, window_references->treeview, window_references->treecolumn);
  dialog.run();
}

void MainWindow::on_check_markers_spacing_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_check_markers_spacing();
}

void MainWindow::on_check_markers_spacing() {
  save_editors();
  show_references_window();
  scripture_checks_usfm_spacing(window_references->liststore, window_references->treeview, window_references->treecolumn, NULL);
}

void MainWindow::on_check_references_inventory_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_check_references_inventory();
}

void MainWindow::on_check_references_inventory() {
  save_editors();
  scripture_checks_references_inventory(true);
}

void MainWindow::on_check_references_validate_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_check_references_validate();
}

void MainWindow::on_check_references_validate() {
  save_editors();
  show_references_window();
  scripture_checks_validate_references(window_references->liststore, window_references->treeview, window_references->treecolumn, NULL);
}

void MainWindow::on_check_nt_quotations_from_the_ot_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_check_nt_quotations_from_the_ot();
}

void MainWindow::on_check_nt_quotations_from_the_ot() {
  save_editors();
  scripture_checks_nt_quotations_from_ot(true);
}

void MainWindow::on_synoptic_parallel_passages_from_the_nt_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_synoptic_parallel_passages_from_the_nt();
}

void MainWindow::on_synoptic_parallel_passages_from_the_nt() {
  save_editors();
  scripture_checks_synoptic_parallels_from_nt(true);
}

void MainWindow::on_parallels_from_the_ot_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_parallels_from_the_ot();
}

void MainWindow::on_parallels_from_the_ot() {
  save_editors();
  scripture_checks_parallels_from_ot(true);
}

void MainWindow::on_check_sentence_structure_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_check_sentence_structure();
}

void MainWindow::on_check_sentence_structure() {
  save_editors();
  show_references_window();
  scripture_checks_sentence_structure(window_references->liststore, window_references->treeview, window_references->treecolumn, NULL);
}

/*
 |
 |
 |
 |
 |
 Styles
 |
 |
 |
 |
 |
 */

void MainWindow::on_goto_styles_area() {
  // Create or active the styles window.
  display_window_styles();
  // Focus the window to enable the user to start inserting the style using the keyboard.
  if (window_styles) {
    window_styles->present();
  }
}

void MainWindow::display_window_styles() {
  // Display the styles if needed.
  if (!window_styles) {
    window_styles = new WindowStyles (accelerator_group, windows_startup_pointer != G_MAXINT,
        window_menu->style, window_menu->style_menu,
        window_menu->stylesheets_expand_all, window_menu->stylesheets_collapse_all,
        window_menu->style_insert, window_menu->stylesheet_edit_mode, window_menu->style_new,
        window_menu->style_properties, window_menu->style_delete,
        window_menu->stylesheet_switch, window_menu->stylesheets_new, window_menu->stylesheets_delete,
        window_menu->stylesheets_rename, window_menu->stylesheets_import, window_menu->stylesheets_export);
    g_signal_connect ((gpointer) window_styles->delete_signal_button, "clicked", G_CALLBACK (on_window_styles_delete_button_clicked), gpointer(this));
    g_signal_connect ((gpointer) window_styles->focus_in_signal_button, "clicked", G_CALLBACK (on_window_focus_button_clicked), gpointer(this));
    g_signal_connect ((gpointer) window_styles->apply_signal, "clicked", G_CALLBACK (on_style_button_apply_clicked), gpointer (this));
    g_signal_connect ((gpointer) window_styles->open_signal, "clicked", G_CALLBACK (on_style_button_open_clicked), gpointer (this));
    g_signal_connect ((gpointer) window_styles->edited_signal, "clicked", G_CALLBACK (on_style_edited), gpointer (this));
    extern Settings * settings;
    ustring stylesheet = settings->genconfig.stylesheet_get();
    stylesheet_open_named(stylesheet);
  }
}

void MainWindow::on_window_styles_delete_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_styles_delete_button();
}

void MainWindow::on_window_styles_delete_button() {
  if (window_styles) {
    delete window_styles;
    window_styles = NULL;
  }
}

void MainWindow::stylesheet_open_named(const ustring& stylesheet) {
  if (window_styles) {
    window_styles->load(stylesheet);
  }
}

void MainWindow::on_style_button_open_clicked(GtkButton *button, gpointer user_data)
// This is activated by the GuiStyles object if another stylesheet should be opened.
{
  ((MainWindow *) user_data)->on_style_button_open();
}

void MainWindow::on_style_button_open() {
  if (window_styles) {
    // Save the name of the stylesheet in two locations.
    extern Settings * settings;
    settings->genconfig.stylesheet_set(window_styles->get_sheet());
    if (!settings->genconfig.project_get().empty()) {
      ProjectConfiguration * projectconfig = settings->projectconfig(settings->genconfig.project_get());
      projectconfig->stylesheet_set(window_styles->get_sheet());
    }
    // Actually open it.  
    stylesheet_open_named(window_styles->get_sheet());
  }
}

void MainWindow::on_style_button_apply_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_style_apply();
}

void MainWindow::on_style_apply() {
  // Get the focused Editor. If none, bail out.
  WindowEditor * editor_window = last_focused_editor_window();
  if (!editor_window)
    return;

  // Bail out if there's no styles window.
  if (!window_styles)
    return;

  // Get the focused style(s).
  ustring selected_style = window_styles->get_focus();

  // Only proceed when a style has been selected.
  if (selected_style.empty())
    return;

  // Get the Style object.
  extern Settings * settings;
  Style style(settings->genconfig.stylesheet_get(), selected_style, false);

  // Whether and how the style is used.
  bool style_was_used = true;
  bool style_was_treated_specially = false;

  // Special treatment for the chapter style.
  if (style.type == stChapterNumber) {
    // Ask whether the user wishes to insert a new chapter.
    if (gtkw_dialog_question(mainwindow, "Would you like to insert a new chapter?", GTK_RESPONSE_YES) == GTK_RESPONSE_YES) {
      // Insert a new chapter.
      save_editors();
      ChapterNumberDialog dialog(true);
      if (dialog.run() == GTK_RESPONSE_OK) {
        reload_project();
      } else {
        style_was_used = false;
      }
      style_was_treated_specially = true;
    }
  }

  // Inserting footnote or endnote or crossreference.
  {
    Editor * editor = editor_window->editor;
    if (editor->last_focused_type() == etvtBody) {
      if (style.type == stFootEndNote) {
        if (style.subtype == fentFootnote) {
          InsertNoteDialog dialog(indtFootnote);
          if (dialog.run() == GTK_RESPONSE_OK) {
            editor->insert_note(style.marker, dialog.rawtext, 
            NULL, true);
          } else {
            style_was_used = false;
          }
          style_was_treated_specially = true;
        }
        if (style.subtype == fentEndnote) {
          InsertNoteDialog dialog(indtEndnote);
          if (dialog.run() == GTK_RESPONSE_OK) {
            editor->insert_note(style.marker, dialog.rawtext, 
            NULL, true);
          } else {
            style_was_used = false;
          }
          style_was_treated_specially = true;
        }
      }
      if (style.type == stCrossreference) {
        InsertNoteDialog dialog(indtCrossreference);
        if (dialog.run() == GTK_RESPONSE_OK) {
          editor->insert_note(style.marker, dialog.rawtext, NULL, true);
        } else {
          style_was_used = false;
        }
        style_was_treated_specially = true;
        // If the gui has been set so, display the references in the tools area.
        if (settings->genconfig.inserting_xref_shows_references_get()) {
          show_references_window();
          gtk_widget_grab_focus(editor->last_focused_textview());
        }
      }
    }
  }

  // Special treatment for a table style.
  {
    Editor * editor = editor_window->editor;
    if (editor->last_focused_type() == etvtBody) {
      if (style.type == stTableElement) {
        InsertTableDialog dialog(editor->project);
        if (dialog.run() == GTK_RESPONSE_OK) {
          editor->insert_table(dialog.rawtext, NULL);
        } else {
          style_was_used = false;
        }
        style_was_treated_specially = true;
      }
    }
  }

  // Normal treatment of the style if it was not handled specially.
  if (!style_was_treated_specially) {
    // Normal treatment of the marker: apply it.
    editor_window->editor->apply_style(selected_style);
  }

  // Take some actions if the style was used.
  if (style_was_used) {
    window_styles->use(selected_style);
  }
}

void MainWindow::on_editor_style_changed(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->editor_style_changed();
}

void MainWindow::editor_style_changed() {
  WindowEditor * editor_window = last_focused_editor_window();
  if (!editor_window)
    return;
  Editor * editor = editor_window->editor;
  set <ustring> styles = editor->get_styles_at_cursor();
  vector <ustring> styles2(styles.begin(), styles.end());
  ustring text = "Style ";
  for (unsigned int i = 0; i < styles2.size(); i++) {
    if (i)
      text.append(", ");
    text.append(styles2[i]);
  }
  gtk_label_set_text(GTK_LABEL (window_menu->statuslabel_style), text.c_str ());
}

void MainWindow::on_style_edited(GtkButton *button, gpointer user_data)
// This function is called when the properties of a style have been edited.
{
  ((MainWindow *) user_data)->reload_styles();
}

void MainWindow::reload_styles() {
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    editor_windows[i]->editor->create_or_update_formatting_data();
  }
}

/*
 |
 |
 |
 |
 |
 Footnotes, endnotes, crossreferences
 |
 |
 |
 |
 |
 */

void MainWindow::on_edit_bible_note_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_edit_bible_note();
}

void MainWindow::on_edit_bible_note() {
  WindowEditor * editor_window = last_focused_editor_window();
  if (editor_window) {
    EditNoteDialog dialog(editor_window->editor);
    dialog.run();
  }
}

/*
 |
 |
 |
 |
 |
 Formatted view
 |
 |
 |
 |
 |
 */

/*
 |
 |
 |
 |
 |
 Tools
 |
 |
 |
 |
 |
 */

void MainWindow::on_menutools_activate(GtkMenuItem *menuitem, gpointer user_data) {
}

void MainWindow::on_line_cutter_for_hebrew_text1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_line_cutter_for_hebrew_text();
}

void MainWindow::on_line_cutter_for_hebrew_text() {
  LineCutterDialog dialog(0);
  if (dialog.run() == GTK_RESPONSE_OK) {
    reload_project();
  }
}

void MainWindow::on_notes_transfer_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_notes_transfer();
}

void MainWindow::on_notes_transfer() {
  save_editors();
  NotesTransferDialog dialog(0);
  if (dialog.run() == GTK_RESPONSE_OK)
    notes_redisplay();
}

void MainWindow::on_tool_origin_references_in_bible_notes_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_tool_origin_references_in_bible_notes();
}

void MainWindow::on_tool_origin_references_in_bible_notes() {
  save_editors();
  OriginReferencesDialog dialog(0);
  if (dialog.run() == GTK_RESPONSE_OK)
    reload_project();
}

void MainWindow::on_tool_project_notes_mass_update1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_tool_project_notes_mass_update();
}

void MainWindow::on_tool_project_notes_mass_update() {
  NotesUpdateDialog dialog(0);
  if (dialog.run() == GTK_RESPONSE_OK) {
    notes_redisplay();
  }
}

void MainWindow::on_tool_generate_word_lists_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_tool_generate_word_lists();
}

void MainWindow::on_tool_generate_word_lists() {
  save_editors();
  WordlistDialog dialog(0);
  if (dialog.run() == GTK_RESPONSE_OK)
    reload_project();
}

void MainWindow::on_tool_transfer_project_notes_to_text_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_tool_transfer_project_notes_to_text();
}

void MainWindow::on_tool_transfer_project_notes_to_text()
// This transfers the currently visible project notes to the currently active project, 
// and does that for each verse.
{
  save_editors();
  XferNotes2TextDialog dialog(0);
  if (dialog.run() == GTK_RESPONSE_OK) {
    reload_project();
  }
}

void MainWindow::on_preferences_features_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_preferences_features();
}

void MainWindow::on_preferences_features() {
  if (password_pass(mainwindow)) {
    FeaturesDialog dialog(0);
    dialog.run();
  }
}

void MainWindow::on_preferences_password_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_preferences_password();
}

void MainWindow::on_preferences_password() {
  password_edit(mainwindow);
}

void MainWindow::on_tool_simple_text_corrections_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_tool_simple_text_corrections();
}

void MainWindow::on_tool_simple_text_corrections() {
  save_editors();
  FixMarkersDialog dialog(0);
  if (dialog.run() == GTK_RESPONSE_OK)
    reload_project();
}

void MainWindow::on_preferences_text_replacement_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_preferences_text_replacement();
}

void MainWindow::on_preferences_text_replacement() {
  TextReplacementDialog dialog(0);
  dialog.run();
}

void MainWindow::on_parallel_passages1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_parallel_passages1();
}

void MainWindow::on_parallel_passages1() {
  // Act depending on whether to view the parallel passages.
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (window_menu->parallel_passages1))) {
    // View them: show references tab, view passages.
    show_references_window();
    parallel_passages_display(navigation.reference, window_references->liststore, window_references->treeview, window_references->treecolumn);
  } else {
    // Don't view them: clear store.
    on_clear_references();
  }
}

void MainWindow::on_pdf_viewer1_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_pdf_viewer();
}

void MainWindow::on_pdf_viewer() {
  PDFViewerDialog dialog(0);
  dialog.run();
}

void MainWindow::on_view_usfm_code_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_view_usfm_code();
}

void MainWindow::on_view_usfm_code() {
  WindowEditor * editor_window = last_focused_editor_window();
  if (!editor_window)
    return;
  Editor * editor = editor_window->editor;
  save_editors();
  ustring filename = project_data_filename_chapter(editor->project, editor->book, editor->chapter, false);
  ViewUSFMDialog dialog(filename);
  dialog.run();
  if (dialog.changed) {
    reload_project();
  }
}

void MainWindow::on_insert_special_character_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_insert_special_character();
}

void MainWindow::on_insert_special_character() {
  WindowEditor * editor_window = last_focused_editor_window();
  if (!editor_window)
    return;
  Editor * editor = editor_window->editor;
  extern Settings * settings;
  vector <ustring> characters;
  vector <ustring> descriptions;
  characters.push_back("­");
  descriptions.push_back("Soft hyphen");
  characters.push_back(" ");
  descriptions.push_back("No-break space");
  characters.push_back("“");
  descriptions.push_back("Left double quotation mark");
  characters.push_back("”");
  descriptions.push_back("Right double quotation mark");
  characters.push_back("‘");
  descriptions.push_back("Left single quotation mark");
  characters.push_back("’");
  descriptions.push_back("Right single quotation mark");
  characters.push_back("«");
  descriptions.push_back("Left-pointing double angle quotation mark");
  characters.push_back("»");
  descriptions.push_back("Right-pointing double angle quotation mark");
  RadiobuttonDialog dialog("Insert character", "Insert special character", descriptions, settings->session.special_character_selection);
  if (dialog.run() != GTK_RESPONSE_OK)
    return;
  settings->session.special_character_selection = dialog.selection;
  editor->text_insert(characters[dialog.selection]);
}

void MainWindow::on_preferences_graphical_interface_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_preferences_graphical_interface();
}

void MainWindow::on_preferences_graphical_interface() {
  InterfacePreferencesDialog dialog(0);
  dialog.run();
}

/*
 |
 |
 |
 |
 |
 Keyterms
 |
 |
 |
 |
 |
 */

void MainWindow::on_check_key_terms_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_check_key_terms();
}

void MainWindow::on_check_key_terms() {
  on_window_check_keyterms_delete_button();
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (window_menu->check_key_terms))) {
    window_check_keyterms = new WindowCheckKeyterms (accelerator_group, windows_startup_pointer != G_MAXINT);
    g_signal_connect ((gpointer) window_check_keyterms->delete_signal_button, "clicked", G_CALLBACK (on_window_check_keyterms_delete_button_clicked), gpointer(this));
    g_signal_connect ((gpointer) window_check_keyterms->focus_in_signal_button, "clicked", G_CALLBACK (on_window_focus_button_clicked), gpointer(this));
    g_signal_connect ((gpointer) window_check_keyterms->signal, "clicked", G_CALLBACK (on_keyterms_new_reference), gpointer(this));
  }
}

void MainWindow::on_window_check_keyterms_delete_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_check_keyterms_delete_button();
}

void MainWindow::on_window_check_keyterms_delete_button() {
  if (window_check_keyterms) {
    delete window_check_keyterms;
    window_check_keyterms = NULL;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (window_menu->check_key_terms), false);
  }
}

void MainWindow::on_keyterms_new_reference(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->check_move_new_reference();
}

void MainWindow::check_move_new_reference() {
  Reference reference(window_check_keyterms->new_reference_showing->book, window_check_keyterms->new_reference_showing->chapter, window_check_keyterms->new_reference_showing->verse);
  navigation.display(reference);
}

void MainWindow::on_view_keyterms_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_view_keyterms();

}

void MainWindow::on_view_keyterms() {
  on_window_show_keyterms_delete_button();
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (window_menu->view_keyterms))) {
    window_show_keyterms = new WindowShowKeyterms (accelerator_group, windows_startup_pointer != G_MAXINT);
    g_signal_connect ((gpointer) window_show_keyterms->delete_signal_button, "clicked", G_CALLBACK (on_window_show_keyterms_delete_button_clicked), gpointer(this));
    g_signal_connect ((gpointer) window_show_keyterms->focus_in_signal_button, "clicked", G_CALLBACK (on_window_focus_button_clicked), gpointer(this));
    extern Settings * settings;
    window_show_keyterms->go_to(settings->genconfig.project_get(), navigation.reference);
  }
}

void MainWindow::on_window_show_keyterms_delete_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_show_keyterms_delete_button();
}

void MainWindow::on_window_show_keyterms_delete_button() {
  if (window_show_keyterms) {
    delete window_show_keyterms;
    window_show_keyterms = NULL;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (window_menu->view_keyterms), false);
  }
}

/*
 |
 |
 |
 |
 |
 Backup
 |
 |
 |
 |
 |
 */

void MainWindow::on_project_backup_incremental_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_project_backup_incremental();
}

void MainWindow::on_project_backup_incremental() {
  save_editors();
  git_command_pause(true);
  backup_make_incremental();
  git_command_pause(false);
}

void MainWindow::on_project_backup_flexible_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_project_backup_flexible();
}

void MainWindow::on_project_backup_flexible() {
  save_editors();
  git_command_pause(true);
  BackupDialog dialog(0);
  if (dialog.run() == GTK_RESPONSE_OK) {
    backup_make_flexible();
  }
  git_command_pause(false);
}

/*
 |
 |
 |
 |
 |
 Git
 |
 |
 |
 |
 |
 */

void MainWindow::on_view_git_tasks_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_view_git_tasks();
}

void MainWindow::on_view_git_tasks() {
  ViewGitDialog dialog(0);
  dialog.run();
}

void MainWindow::on_preferences_remote_git_repository_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_preferences_remote_git_repository();
}

void MainWindow::on_preferences_remote_git_repository() {
  save_editors();
  GitSetupDialog dialog(0);
  if (dialog.run() == GTK_RESPONSE_OK)
    reload_project();
}

void MainWindow::on_git_reopen_project() {
  if (git_reopen_project) {
    git_reopen_project = false; // Close flag before dialog shows, else the dialogs keep coming.
    reload_project();
  }
}

void MainWindow::on_project_changes_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_project_changes();
}

void MainWindow::on_project_changes() {
  // Save even the very latest changes.
  save_editors();
  // The changes checker will generate git tasks. Pause git.
  git_command_pause(true);
  // Do the actual changes dialog. 
  // It will delete the unwanted git tasks.
  show_references_window();
  References references(window_references->liststore, window_references->treeview, window_references->treecolumn);
  ChangesDialog dialog(&references);
  dialog.run();
  // Resume git operations.
  git_command_pause(false);
}

void MainWindow::on_edit_revert_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_edit_revert();
}

void MainWindow::on_edit_revert() {
  save_editors();
  RevertDialog dialog(&navigation.reference);
  if (dialog.run() == GTK_RESPONSE_OK) {
    reload_project();
  }
}

void MainWindow::git_update_intervals_initialize() {
  // Make containers with all projects to be updated, and their intervals.
  extern Settings * settings;
  vector <ustring> projects = projects_get_all();
  for (unsigned int i = 0; i < projects.size(); i++) {
    ProjectConfiguration * projectconfig = settings->projectconfig(projects[i]);
    if (projectconfig->git_use_remote_repository_get()) {
      git_update_intervals[projects[i]] = 0;
    }
  }
  // Start the timer.
  git_update_interval_event_id = g_timeout_add_full(G_PRIORITY_DEFAULT, 1000, GSourceFunc (on_git_update_timeout), gpointer(this), NULL);
}

bool MainWindow::on_git_update_timeout(gpointer user_data) {
  ((MainWindow *) user_data)->git_update_timeout();
  return true;
}

void MainWindow::git_update_timeout()
// Schedule project update tasks.
{
  // Bail out if git tasks are paused.
  extern Settings * settings;
  if (settings->session.git_pause)
    return;

  // Schedule a task for each relevant project.
  vector <ustring> projects = projects_get_all();
  for (unsigned int i = 0; i < projects.size(); i++) {
    ProjectConfiguration * projectconfig = settings->projectconfig(projects[i]);
    if (projectconfig->git_use_remote_repository_get()) {
      int interval = git_update_intervals[projects[i]];
      interval++;
      if (interval >= projectconfig->git_remote_update_interval_get()) {
        git_schedule(gttUpdateProject, projects[i], 0, 0, projectconfig->git_remote_repository_url_get());
        interval = 0;
      }
      git_update_intervals[projects[i]] = interval;
    }
  }
}

/*
 |
 |
 |
 |
 |
 Fonts
 |
 |
 |
 |
 |
 */

void MainWindow::on_view_text_font_activate(GtkMenuItem * menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_text_font();
}

void MainWindow::on_text_font() {
  // Get the font and colour settings, either from the project, if it is opened, 
  // or else from genconfig.
  extern Settings * settings;
  bool defaultfont = settings->genconfig.text_editor_font_default_get();
  ustring fontname = settings->genconfig.text_editor_font_name_get();
  unsigned int linespacing = 100;
  bool defaultcolour = settings->genconfig.text_editor_default_color_get();
  unsigned int normaltextcolour = settings->genconfig.text_editor_normal_text_color_get();
  unsigned int backgroundcolour = settings->genconfig.text_editor_background_color_get();
  unsigned int selectedtextcolour = settings->genconfig.text_editor_selected_text_color_get();
  unsigned int selectioncolour = settings->genconfig.text_editor_selection_color_get();
  WindowEditor * editor_window = last_focused_editor_window();
  if (editor_window) {
    Editor * editor = editor_window->editor;
    ProjectConfiguration * projectconfig = settings->projectconfig(editor->project);
    defaultfont = projectconfig->editor_font_default_get();
    fontname = projectconfig->editor_font_name_get();
    linespacing = projectconfig->text_line_height_get();
    defaultcolour = settings->genconfig.text_editor_default_color_get();
    normaltextcolour = projectconfig->editor_normal_text_color_get();
    backgroundcolour = projectconfig->editor_background_color_get();
    selectedtextcolour = projectconfig->editor_selected_text_color_get();
    selectioncolour = projectconfig->editor_selection_color_get();
  }

  // Display font selection dialog. 
  FontColorDialog dialog(defaultfont, fontname, linespacing, defaultcolour, normaltextcolour, backgroundcolour, selectedtextcolour, selectioncolour);
  if (dialog.run() != GTK_RESPONSE_OK)
    return;

  // Save font, and set it.
  settings->genconfig.text_editor_font_default_set(dialog.new_use_default_font);
  settings->genconfig.text_editor_font_name_set(dialog.new_font);
  settings->genconfig.text_editor_default_color_set(dialog.new_use_default_color);
  settings->genconfig.text_editor_normal_text_color_set(dialog.new_normal_text_color);
  settings->genconfig.text_editor_background_color_set(dialog.new_background_color);
  settings->genconfig.text_editor_selected_text_color_set(dialog.new_selected_text_color);
  settings->genconfig.text_editor_selection_color_set(dialog.new_selection_color);
  if (editor_window) {
    Editor * editor = editor_window->editor;
    ProjectConfiguration * projectconfig = settings->projectconfig(editor->project);
    projectconfig->editor_font_default_set(dialog.new_use_default_font);
    projectconfig->editor_font_name_set(dialog.new_font);
    projectconfig->text_line_height_set(dialog.new_line_spacing);
    projectconfig->editor_default_color_set(dialog.new_use_default_color);
    projectconfig->editor_normal_text_color_set(dialog.new_normal_text_color);
    projectconfig->editor_background_color_set(dialog.new_background_color);
    projectconfig->editor_selected_text_color_set(dialog.new_selected_text_color);
    projectconfig->editor_selection_color_set(dialog.new_selection_color);
  }
  set_fonts();
}

void MainWindow::set_fonts() {
  // Set font in the text editors. Set text direction too.
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    editor_windows[i]->editor->set_font();
    editor_windows[i]->editor->create_or_update_formatting_data();
  }
}

/*
 |
 |
 |
 |
 |
 Outline
 |
 |
 |
 |
 |
 */

void MainWindow::on_view_outline_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_view_outline();
}

void MainWindow::on_view_outline() {
  on_window_outline_delete_button();
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (window_menu->view_outline))) {
    window_outline = new WindowOutline (accelerator_group, windows_startup_pointer != G_MAXINT);
    g_signal_connect ((gpointer) window_outline->delete_signal_button, "clicked", G_CALLBACK (on_window_outline_delete_button_clicked), gpointer(this));
    g_signal_connect ((gpointer) window_outline->focus_in_signal_button, "clicked", G_CALLBACK (on_window_focus_button_clicked), gpointer(this));
    g_signal_connect ((gpointer) window_outline->outline->reference_changed_signal, "clicked", G_CALLBACK (on_button_outline_clicked), gpointer(this));
    extern Settings * settings;
    window_outline->go_to(settings->genconfig.project_get(), navigation.reference);
  }
}

void MainWindow::on_window_outline_delete_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_outline_delete_button();
}

void MainWindow::on_window_outline_delete_button() {
  if (window_outline) {
    delete window_outline;
    window_outline = NULL;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (window_menu->view_outline), false);
  }
}

void MainWindow::on_button_outline_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_button_outline();
}

void MainWindow::on_button_outline() {
  if (window_outline) {
    Reference reference(navigation.reference);
    reference.chapter = window_outline->outline->newchapter;
    reference.verse = convert_to_string(window_outline->outline->newverse);
    navigation.display(reference);
  }
}

/*
 |
 |
 |
 |
 |
 Interprocess communications
 |
 |
 |
 |
 |
 */

void MainWindow::on_ipc_method_called(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_ipc_method();
}

void MainWindow::on_ipc_method() {
  // Interprocess Communication pointer.
  extern InterprocessCommunication * ipc;

  // Settings pointer.
  extern Settings * settings;

  // Handle call for a new git job.
  if (ipc->method_called_type == ipcctGitJobDescription) {
    if (!settings->session.git_pause) {
      vector <ustring> task = git_get_next_task();
      if (!task.empty()) {
        ipc->send(ipcstBibleditGit, ipcctGitJobDescription, task);
        // The chapter state looks whether something changed in the chapter 
        // now opened while we pull changes from the remote repository.
        gitchapterstate = NULL;
        if (convert_to_int(task[0]) == gttUpdateProject) {
          if (task[1] == settings->genconfig.project_get()) {
            gitchapterstate = new GitChapterState (task[1], navigation.reference.book, navigation.reference.chapter);
          }
        }
      }
    }
  }

  // Handle a job done.
  else if (ipc->method_called_type == ipcctGitTaskDone) {
    // Get the errors given by this task.
    vector <ustring> error = ipc->get_payload(ipcctGitTaskDone);
    if (error.empty()) {
      // No errors: erase the task.
      git_erase_task_done();
    } else {
      // Errors: Resolve any conflicts in the project the latest task refers to.  
      vector <ustring> task = git_get_next_task();
      if (!task.empty()) {
        git_resolve_conflicts(task[1], error);
      }
      // Task failed: re-queue it.
      git_fail_task_done();
    }
    // Set a flag if the state of the currently opened chapter changed.
    // (This must be done after conflicting merges have been resolved, as that
    //  could affect the chapter now opened).
    if (gitchapterstate) {
      if (gitchapterstate->changed()) {
        git_reopen_project = true;
      }
      delete gitchapterstate;
      gitchapterstate = NULL;
    }
  }
}

/*
 |
 |
 |
 |
 |
 Reporting and Planning
 |
 |
 |
 |
 |
 */

void MainWindow::on_preferences_reporting_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_preferences_reporting();
}

void MainWindow::on_preferences_reporting() {
  ReportingSetupDialog dialog(0);
  dialog.run();
}

void MainWindow::on_editstatus_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_editstatus();
}

void MainWindow::on_editstatus()
// Edits the project's status.
{
  extern Settings * settings;
  EditStatusDialog dialog(settings->genconfig.project_get(), navigation.reference.book, navigation.reference.chapter);
  dialog.run();
}

void MainWindow::on_view_status_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_view_status();
}

void MainWindow::on_view_status() {
  ViewStatusDialog dialog(0);
  dialog.run();
}

void MainWindow::on_edit_planning_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_edit_planning();
}

void MainWindow::on_edit_planning() {
  extern Settings * settings;
  planning_edit(settings->genconfig.project_get());
}

void MainWindow::on_view_planning_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_view_planning();
}

void MainWindow::on_view_planning() {
  extern Settings * settings;
  planning_produce_report(settings->genconfig.project_get());
}

void MainWindow::on_preferences_planning_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_preferences_planning();
}

void MainWindow::on_preferences_planning() {
  PlanningSetupDialog dialog(0);
  dialog.run();
}

/*
 |
 |
 |
 |
 |
 Resources
 |
 |
 |
 |
 |
 */

void MainWindow::on_window_resource_delete_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_resource_delete_button(button);
}

void MainWindow::on_window_resource_delete_button(GtkButton *button) {
  GtkWidget * widget= GTK_WIDGET (button);
  vector <WindowResource *>::iterator iterator = resource_windows.begin();
  for (unsigned int i = 0; i < resource_windows.size(); i++) {
    if (widget == resource_windows[i]->delete_signal_button) {
      delete resource_windows[i];
      resource_windows.erase(iterator);
      break;
    }
    iterator++;
  }
}

void MainWindow::on_file_resources_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_file_resources();
}

WindowResource * MainWindow::last_focused_resource_window()
// Get the focused resource window, or NULL if there's none.
{
  WindowResource * resource_window= NULL;
  time_t most_recent_time = 0;
  for (unsigned int i = 0; i < resource_windows.size(); i++) {
    if (resource_windows[i]->focused_time > most_recent_time) {
      most_recent_time = resource_windows[i]->focused_time;
      resource_window = resource_windows[i];
    }
  }
  return resource_window;
}

void MainWindow::on_file_resources()
// Set some menu options sensitive if a resource is open.
{
  gtk_widget_set_sensitive(window_menu->file_resources_close, !resource_windows.empty());
  gtk_widget_set_sensitive(window_menu->file_resources_edit, !resource_windows.empty());
}

void MainWindow::on_file_resources_open_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_file_resources_open("");
}

void MainWindow::on_file_resources_open(ustring resource)
// Opens a resource.
{
  // Find data about the resource, and whether it exists.
  vector <ustring> filenames;
  vector <ustring> resources = resource_get_resources(filenames, false);
  quick_sort(resources, filenames, 0, resources.size());
  if (resource.empty()) {
    ListviewDialog dialog("Open resource", resources, "", false, NULL);
    if (dialog.run() == GTK_RESPONSE_OK) {
      resource = dialog.focus;
    }
  }
  if (resource.empty())
    return;
  ustring filename;
  for (unsigned int i = 0; i < resources.size(); i++) {
    if (resource == resources[i]) {
      filename = filenames[i];
    }
  }
  if (filename.empty())
    return;

  // If the resource already displays, bail out.
  for (unsigned int i = 0; i < resource_windows.size(); i++) {
    if (resource == resource_windows[i]->name) {
      return;
    }
  }

  // Display a new resource.
  WindowResource * resource_window = new WindowResource (resource, accelerator_group, false);
  g_signal_connect ((gpointer) resource_window->delete_signal_button, "clicked", G_CALLBACK (on_window_resource_delete_button_clicked), gpointer(this));
  g_signal_connect ((gpointer) resource_window->focus_in_signal_button, "clicked", G_CALLBACK (on_window_focus_button_clicked), gpointer(this));
  resource_window->go_to(navigation.reference);
  resource_windows.push_back(resource_window);
}

void MainWindow::on_file_resources_close_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_file_resources_close();
}

void MainWindow::on_file_resources_close()
// Closes the focused resource.
{
  WindowResource * focused_window = last_focused_resource_window();
  if (focused_window) {
    on_window_resource_delete_button(GTK_BUTTON (focused_window->delete_signal_button));
  }
}

void MainWindow::on_file_resources_new_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_file_resources_new();
}

void MainWindow::on_file_resources_new() {
  NewResourceDialog dialog("");
  dialog.run();
}

void MainWindow::on_file_resources_edit_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_file_resources_edit();
}

void MainWindow::on_file_resources_edit() {
  WindowResource * focused_resource_window = last_focused_resource_window();
  if (focused_resource_window) {
    ustring templatefile = focused_resource_window->resource->template_get();
    NewResourceDialog dialog(templatefile);
    if (dialog.run() == GTK_RESPONSE_OK) {
      focused_resource_window->resource->open(dialog.edited_template_file);
    }
  }
}

void MainWindow::on_file_resources_delete_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_file_resources_delete();
}

void MainWindow::on_file_resources_delete()
// Delete a resource.
{
  vector <ustring> filenames;
  vector <ustring> resources = resource_get_resources(filenames, false);
  ListviewDialog dialog("Delete resource", resources, "", false, NULL);
  if (dialog.run() == GTK_RESPONSE_OK) {
    int result = gtkw_dialog_question(NULL, "Are you sure you want to delete resource " + dialog.focus + "?");
    if (result == GTK_RESPONSE_YES) {
      ustring filename;
      for (unsigned int i = 0; i < resources.size(); i++) {
        if (dialog.focus == resources[i]) {
          filename = filenames[i];
        }
      }
      // There are two methods of deleting resources:
      if (resource_add_name_to_deleted_ones_if_standard_template(filename)) {
        // 1. A template that comes with bibledit: We can't delete this as we don't 
        // have write access to the folder where it is stored. Therefore 
        // we "delete" it by placing it in a file that lists deleted
        // resources.
      } else {
        // 2. A user-generated resource: Just delete it.
        ustring directory = gw_path_get_dirname(filename);
        unix_rmdir(directory);
      }
    }
  }
}

/*
 |
 |
 |
 |
 |
 Text Editors
 |
 |
 |
 |
 |
 */

void MainWindow::on_window_editor_delete_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_editor_delete_button(button);
}

void MainWindow::on_window_editor_delete_button(GtkButton *button) {
  GtkWidget * widget= GTK_WIDGET (button);
  vector <WindowEditor *>::iterator iterator = editor_windows.begin();
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    if (widget == editor_windows[i]->delete_signal_button) {
      delete editor_windows[i];
      editor_windows.erase(iterator);
      break;
    }
    iterator++;
  }
  handle_editor_focus();
}

WindowEditor * MainWindow::last_focused_editor_window()
// Get the focused editor window, or NULL if there's none.
{
  WindowEditor * editor_window= NULL;
  time_t most_recent_time = 0;
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    if (editor_windows[i]->focused_time > most_recent_time) {
      most_recent_time = editor_windows[i]->focused_time;
      editor_window = editor_windows[i];
    }
  }
  return editor_window;
}

void MainWindow::on_file_project_open(const ustring& project)
// Opens an editor.
{
  // If the editor already displays, present it and bail out.
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    if (project == editor_windows[i]->window_data) {
      editor_windows[i]->present();
      return;
    }
  }

  // Display a new editor.
  WindowEditor * editor_window = new WindowEditor (project, accelerator_group, false);
  g_signal_connect ((gpointer) editor_window->delete_signal_button, "clicked", G_CALLBACK (on_window_editor_delete_button_clicked), gpointer(this));
  g_signal_connect ((gpointer) editor_window->focus_in_signal_button, "clicked", G_CALLBACK (on_window_focus_button_clicked), gpointer(this));
  g_signal_connect ((gpointer) editor_window->editor->new_verse_signal, "clicked", G_CALLBACK (on_new_verse_signalled), gpointer(this));
  g_signal_connect ((gpointer) editor_window->editor->new_styles_signal, "clicked", G_CALLBACK (on_editor_style_changed), gpointer(this));
  g_signal_connect ((gpointer) editor_window->editor->quick_references_button, "clicked", G_CALLBACK (on_show_quick_references_signal_button_clicked), gpointer(this));
  g_signal_connect ((gpointer) editor_window->editor->word_double_clicked_signal, "clicked", G_CALLBACK (on_send_word_to_toolbox_signalled), gpointer(this));
  g_signal_connect ((gpointer) editor_window->editor->reload_signal, "clicked", G_CALLBACK (on_editor_reload_clicked), gpointer(this));
  g_signal_connect ((gpointer) editor_window->editor->changed_signal, "clicked", G_CALLBACK (on_editorsgui_changed_clicked), gpointer(this));
  editor_windows.push_back(editor_window);

  // After creation the window will generate a focus signal, 
  // and this signal in turn will cause further processing of the editor.
}

void MainWindow::on_editor_reload_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_editor_reload();
}

void MainWindow::on_editor_reload() {
  // Get the focused editor, if none, bail out.
  WindowEditor * editor_window = last_focused_editor_window();
  if (!editor_window)
    return;
  Editor * editor = editor_window->editor;
  // Create the reference where to go to after the project has been reopened.
  // The reference should be obtained before closing the project,
  // so that the chapter number to go to is accessible.
  Reference reference(navigation.reference);
  reference.chapter = editor->reload_chapter_number;
  if (editor->reload_chapter_number == 0)
    reference.verse = "0";
  // Reopen.
  reload_project();
  // Go to the right reference.
  navigation.display(reference);
}

void MainWindow::handle_editor_focus() {
  // Get the focused editor and the project.
  WindowEditor * editor_window = last_focused_editor_window();
  ustring project;
  if (editor_window)
    project = editor_window->editor->project;

  // Set the focused project in the configuration.
  extern Settings * settings;
  settings->genconfig.project_set(project);

  // Enable or disable widgets depending on whether an editor window is focused.
  enable_or_disable_widgets(editor_window);

  // Inform the merge window, if it is there, about the editors.
  if (window_merge) {
    window_merge->set_focused_editor(editor_window->editor);
    vector <Editor *> visible_editors;
    for (unsigned int i = 0; i < editor_windows.size(); i++) {
      visible_editors.push_back(editor_windows[i]->editor);
    }
    window_merge->set_visible_editors(visible_editors);
  }

  // If we've no project bail out.
  if (project.empty())
    return;

  // Project configuration.
  ProjectConfiguration * projectconfig = settings->projectconfig(project);

  // The font and colour are tied to the project, 
  // but also stored in the general configuration.
  settings->genconfig.text_editor_font_default_set(projectconfig->editor_font_default_get());
  settings->genconfig.text_editor_font_name_set(projectconfig->editor_font_name_get());
  settings->genconfig.text_editor_default_color_set(projectconfig->editor_default_color_get());
  settings->genconfig.text_editor_normal_text_color_set(projectconfig->editor_normal_text_color_get());
  settings->genconfig.text_editor_background_color_set(projectconfig->editor_background_color_get());
  settings->genconfig.text_editor_selected_text_color_set(projectconfig->editor_selected_text_color_get());
  settings->genconfig.text_editor_selection_color_set(projectconfig->editor_selection_color_get());

  // Re-initialize Navigation.
  navigation.set_project(project, false);
  Reference reference(settings->genconfig.book_get(), convert_to_int(settings->genconfig.chapter_get()), settings->genconfig.verse_get());
  navigation.display(reference);
  // Some rumbling internal logic causes the verse to jump to 0 in several cases.
  // To resolve this in a rough manner, after a delay, that is, after the 
  // rumbling is over, cause the navigator to go to the desired verse.
  navigation.display_delayed(reference.verse);

  // Set the available books for search/replace functions.
  vector <unsigned int> books = project_get_books(project);
  set<unsigned int> selection(books.begin(), books.end());
  settings->session.selected_books = selection;

  // Redisplay the project notes.
  notes_redisplay();

  // Open the right stylesheet.
  settings->genconfig.stylesheet_set(projectconfig->stylesheet_get());
  stylesheet_open_named(settings->genconfig.stylesheet_get());
}

void MainWindow::save_editors()
// Save all and any editors.
{
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    editor_windows[i]->editor->chapter_save();
  }
}

void MainWindow::goto_next_previous_project(bool next) {
  // Bail out if there are not enough windows to switch.
  if (editor_windows.size() < 2)
    return;

  // Get the focused project window and its offset.
  WindowEditor * present_window = last_focused_editor_window();
  int offset = 0;
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    if (present_window == editor_windows[i]) {
      offset = i;
    }
  }

  // Move offset to next (or previous) window.
  if (next) {
    offset++;
    if ((unsigned int)(offset) >= editor_windows.size())
      offset = 0;
  } else {
    offset--;
    if (offset < 0)
      offset = editor_windows.size() - 1;
  }

  // Present the new window.
  editor_windows[offset]->present();
}

void MainWindow::on_editorsgui_changed_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_editorsgui_changed();
}

void MainWindow::on_editorsgui_changed() {
  if (window_merge) {
    window_merge->editors_changed();
  }
}

void MainWindow::reload_project()
// This function reloads the projects.
{
  // Project.
  extern Settings * settings;
  ustring project = settings->genconfig.project_get();
  if (project.empty())
    return;

  // Store the reference where to go to after the project has been reloaded.
  Reference reference(navigation.reference);

  // As anything could have happened to the data in the project, force a reload of the navigator.
  navigation.set_project(project, true);

  // Navigate to the old place.
  navigation.display(reference);

  // Reload all editors.
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    editor_windows[i]->editor->chapter_load(reference.chapter);
  }
}

/*
 |
 |
 |
 |
 |
 Merge
 |
 |
 |
 |
 |
 */

void MainWindow::on_file_projects_merge_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_file_projects_merge();
}

void MainWindow::on_file_projects_merge() {
  on_window_merge_delete_button();
  if (window_menu) {
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (window_menu->file_projects_merge))) {
      window_merge = new WindowMerge (accelerator_group, windows_startup_pointer != G_MAXINT);
      g_signal_connect ((gpointer) window_merge->delete_signal_button, "clicked", G_CALLBACK (on_window_merge_delete_button_clicked), gpointer(this));
      g_signal_connect ((gpointer) window_merge->focus_in_signal_button, "clicked", G_CALLBACK (on_window_focus_button_clicked), gpointer(this));
      g_signal_connect ((gpointer) window_merge->editors_get_text_button, "clicked", G_CALLBACK (on_merge_window_get_text_button_clicked), gpointer(this));
      g_signal_connect ((gpointer) window_merge->new_reference_button, "clicked", G_CALLBACK (on_merge_window_new_reference_button_clicked), gpointer(this));
      g_signal_connect ((gpointer) window_merge->save_editors_button, "clicked", G_CALLBACK (on_merge_window_save_editors_button_clicked), gpointer(this));
      g_signal_connect ((gpointer) window_merge->reload_editors_button, "clicked", G_CALLBACK (on_editor_reload_clicked), gpointer(this));
    }
  }
}

void MainWindow::on_window_merge_delete_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_merge_delete_button();
}

void MainWindow::on_window_merge_delete_button() {
  if (window_merge) {
    delete window_merge;
    window_merge = NULL;
    if (window_menu) {
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (window_menu->file_projects_merge), false);
    }
  }
}

void MainWindow::on_merge_window_get_text_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_merge_window_get_text_button();
}

void MainWindow::on_merge_window_get_text_button() {
  if (window_merge) {
    for (unsigned int i = 0; i < editor_windows.size(); i++) {
      if (editor_windows[i]->window_data == window_merge->current_master_project) {
        window_merge->main_project_data = editor_windows[i]->editor->get_chapter();
      }
      if (editor_windows[i]->window_data == window_merge->current_edited_project) {
        window_merge->edited_project_data = editor_windows[i]->editor->get_chapter();
      }
    }
    window_merge->book = navigation.reference.book;
    window_merge->chapter = navigation.reference.chapter;
  }
}

void MainWindow::on_merge_window_new_reference_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_merge_window_new_reference_button();
}

void MainWindow::on_merge_window_new_reference_button() {
  if (window_merge) {
    Reference reference(window_merge->book, window_merge->chapter, "0");
    navigation.display(reference);
  }
}

void MainWindow::on_merge_window_save_editors_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_merge_window_save_editors_button();
}

void MainWindow::on_merge_window_save_editors_button() {
  save_editors();
}

/*
 |
 |
 |
 |
 |
 Diglot
 |
 |
 |
 |
 |
 */

void MainWindow::on_preferences_filters_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_preferences_filters();
}

void MainWindow::on_preferences_filters() {
  FiltersDialog dialog(0);
  dialog.run();
}

/*
 |
 |
 |
 |
 |
 Print
 |
 |
 |
 |
 |
 */

void MainWindow::on_print_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_print();
}

void MainWindow::on_print() {
  // Create the selection dialog using the saved selection.
  unsigned int selection;
  {
    vector <ustring> labels;
    labels.push_back("Project");
    labels.push_back("Parallel Bible");
    labels.push_back("References");
    //labels.push_back("Test usfm2pdf");
    extern Settings * settings;
    RadiobuttonDialog dialog("Print", "Select what to print", labels, settings->genconfig.print_job_get());
    if (dialog.run() != GTK_RESPONSE_OK)
      return;
    selection = dialog.selection;
    settings->genconfig.print_job_set(selection);
  }
  // Save the editors.
  save_editors();

  switch (selection)
  {
    case 0: // Project.
    {
      {
        PrintProjectDialog dialog(0);
        if (dialog.run() != GTK_RESPONSE_OK)
          return;
      }
      extern Settings * settings;
      ProjectMemory projectmemory(settings->genconfig.project_get(), true);
      PrintProject printproject(&projectmemory);
      printproject.print();
      break;
    }
    case 1: // Parallel Bible.
    {
      {
        ParallelBibleDialog dialog(0);
        if (dialog.run() != GTK_RESPONSE_OK)
          return;
      }
      view_parallel_bible_pdf();
      break;
    }
    case 2: // References.
    {
      // Activate references.
      show_references_window();
      // Show dialog.
      {
        PrintReferencesDialog dialog(0);
        if (dialog.run() != GTK_RESPONSE_OK)
          return;
      }
      // Load refs from the editor.
      show_references_window();
      References references(window_references->liststore, window_references->treeview, window_references->treecolumn);
      references.get_loaded();
      vector<Reference> refs;
      references.get_references(refs);
      if (refs.empty()) {
        gtkw_dialog_info(mainwindow, "There are no references to print");
      } else {
        // Run the function for printing the references.
        extern Settings * settings;
        vector <ustring> extra_projects = settings->genconfig.print_references_projects_get();
        ProjectMemory projectmemory(settings->genconfig.project_get(), true);
        view_parallel_references_pdf(projectmemory, &extra_projects, refs, true, NULL, true);
      }
      break;
    }
    case 3: // Tes
    {
      extern Settings * settings;
      Text2Pdf text2pdf(gw_build_filename(directories_get_temp(), "pdf.pdf"), settings->genconfig.print_engine_use_intermediate_text_get());

      text2pdf.page_size_set(21, 10);
      text2pdf.page_margins_set(2, 1, 2, 2);
      //text2pdf.print_date_in_header();
      //text2pdf.set_right_to_left(0);
      //text2pdf.suppress_header_this_page();
      text2pdf.set_running_header_left_page("Izihlabelelo");
      text2pdf.set_running_header_right_page("Izihlabelelo");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(16);
      text2pdf.paragraph_set_italic(1);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(2);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_column_count(1);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("UGWALO ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(20);
      text2pdf.paragraph_set_bold(1);
      text2pdf.paragraph_set_space_before(8);
      text2pdf.paragraph_set_space_after(4);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_column_count(1);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("LWEZIHLABELELO");
      text2pdf.set_running_chapter_number(1, 1);
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(15);
      text2pdf.paragraph_set_bold(1);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(1);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Isihlabelelo");
      text2pdf.add_text(" ");
      text2pdf.add_text("1");
      text2pdf.close_paragraph();
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.inline_set_font_size_percentage(100);
      text2pdf.inline_set_italic(0);
      text2pdf.inline_set_bold(1);
      text2pdf.inline_set_underline(0);
      text2pdf.inline_set_small_caps(0);
      text2pdf.add_text("1");
      text2pdf.inline_clear_font_size_percentage();
      text2pdf.inline_clear_italic();
      text2pdf.inline_clear_bold();
      text2pdf.inline_clear_underline();
      text2pdf.inline_clear_small_caps();
      text2pdf.inline_clear_superscript();
      text2pdf.inline_clear_colour();
      text2pdf.add_text(" Uyabusiswa umuntu ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Ongahambi ngalo ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Icebo labo ababi ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(2);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.add_text("Lenhlanganwen' yabo. ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Ongem' endleleni yabo ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Abayizw' izoni, ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Engahlal' esihlalweni ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(2);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.add_text("Esabagconayo. ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.inline_set_font_size_percentage(100);
      text2pdf.inline_set_italic(0);
      text2pdf.inline_set_bold(1);
      text2pdf.inline_set_underline(0);
      text2pdf.inline_set_small_caps(0);
      text2pdf.add_text("2");
      text2pdf.inline_clear_font_size_percentage();
      text2pdf.inline_clear_italic();
      text2pdf.inline_clear_bold();
      text2pdf.inline_clear_underline();
      text2pdf.inline_clear_small_caps();
      text2pdf.inline_clear_superscript();
      text2pdf.inline_clear_colour();
      text2pdf.add_text(" Kodw' ukujabula kwakhe ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Kukuwo umthetho ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("WeNkosi ewukhumbula ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(2);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.add_text("Emini leb'suku. ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.inline_set_font_size_percentage(100);
      text2pdf.inline_set_italic(0);
      text2pdf.inline_set_bold(1);
      text2pdf.inline_set_underline(0);
      text2pdf.inline_set_small_caps(0);
      text2pdf.add_text("3");
      text2pdf.inline_clear_font_size_percentage();
      text2pdf.inline_clear_italic();
      text2pdf.inline_clear_bold();
      text2pdf.inline_clear_underline();
      text2pdf.inline_clear_small_caps();
      text2pdf.inline_clear_superscript();
      text2pdf.inline_clear_colour();
      text2pdf.add_text(" Uzaba njengesihlahla ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("'Simil' emfuleni ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("'Sithel' izithelo zaso ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(2);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.add_text("Ngesikhathi saso. ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Amahlamvu aso wona ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Awayikubuna, ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Konke akwenzayo yena ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(2);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.add_text("Kuzaphumelela. ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.inline_set_font_size_percentage(100);
      text2pdf.inline_set_italic(0);
      text2pdf.inline_set_bold(1);
      text2pdf.inline_set_underline(0);
      text2pdf.inline_set_small_caps(0);
      text2pdf.add_text("4");
      text2pdf.inline_clear_font_size_percentage();
      text2pdf.inline_clear_italic();
      text2pdf.inline_clear_bold();
      text2pdf.inline_clear_underline();
      text2pdf.inline_clear_small_caps();
      text2pdf.inline_clear_superscript();
      text2pdf.inline_clear_colour();
      text2pdf.add_text(" AbangelaNkulunkulu ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Kabanjalo bona, ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Kodwa banjengamakhoba ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(2);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.add_text("Amuka lomoya. ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.inline_set_font_size_percentage(100);
      text2pdf.inline_set_italic(0);
      text2pdf.inline_set_bold(1);
      text2pdf.inline_set_underline(0);
      text2pdf.inline_set_small_caps(0);
      text2pdf.add_text("5");
      text2pdf.inline_clear_font_size_percentage();
      text2pdf.inline_clear_italic();
      text2pdf.inline_clear_bold();
      text2pdf.inline_clear_underline();
      text2pdf.inline_clear_small_caps();
      text2pdf.inline_clear_superscript();
      text2pdf.inline_clear_colour();
      text2pdf.add_text(" Kant' ababi kabasoze ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Bem' ekwahluleni, ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Lezoni emhlanganweni ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(2);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.add_text("Wabalungileyo. ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.inline_set_font_size_percentage(100);
      text2pdf.inline_set_italic(0);
      text2pdf.inline_set_bold(1);
      text2pdf.inline_set_underline(0);
      text2pdf.inline_set_small_caps(0);
      text2pdf.add_text("6");
      text2pdf.inline_clear_font_size_percentage();
      text2pdf.inline_clear_italic();
      text2pdf.inline_clear_bold();
      text2pdf.inline_clear_underline();
      text2pdf.inline_clear_small_caps();
      text2pdf.inline_clear_superscript();
      text2pdf.inline_clear_colour();
      text2pdf.add_text(" Ngob' iNkosi iyayazi ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Indlela yabo nje ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(0);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.paragraph_set_keep_with_next();
      text2pdf.add_text("Abalungileyo kodwa ");
      text2pdf.close_paragraph();
      text2pdf.open_paragraph();
      text2pdf.paragraph_set_font_size(12);
      text2pdf.paragraph_set_space_before(0);
      text2pdf.paragraph_set_space_after(2);
      text2pdf.paragraph_set_left_margin(0);
      text2pdf.paragraph_set_right_margin(0);
      text2pdf.paragraph_set_first_line_indent(0);
      text2pdf.add_text("Eyababi yofa.");
      text2pdf.close_paragraph();

      text2pdf.run();
      text2pdf.view();
      break;
    }
  }
}

/*
 |
 |
 |
 |
 |
 Windowing system
 |
 |
 |
 |
 |
 */

void MainWindow::on_view_screen_layout_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_view_screen_layout();
}

void MainWindow::on_view_screen_layout() {
  on_window_screen_layout_button();
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (window_menu->view_screen_layout))) {
    window_screen_layout = new WindowLayout (0);
    g_signal_connect ((gpointer) window_screen_layout->signal_button, "clicked", G_CALLBACK (on_window_screen_layout_button_clicked), gpointer(this));
  }
}

void MainWindow::on_window_screen_layout_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_screen_layout_button();
}

void MainWindow::on_window_screen_layout_button() {
  if (window_screen_layout) {
    delete window_screen_layout;
    window_screen_layout = NULL;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (window_menu->view_screen_layout), false);
  }
}

bool MainWindow::on_windows_startup_timeout(gpointer data) {
  return ((MainWindow *) data)->on_windows_startup();
}

bool MainWindow::on_windows_startup() {
  // Get all window data.
  WindowData window_data(false);

  bool window_started = false;
  while ((windows_startup_pointer < window_data.ids.size()) && !window_started) {
    if (window_data.shows[windows_startup_pointer]) {
      WindowID id = WindowID(window_data.ids[windows_startup_pointer]);
      ustring data = window_data.datas[windows_startup_pointer];
      switch (id)
      {
        case widShowKeyterms:
        {
          gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (window_menu->view_keyterms), true);
          break;
        }
        case widShowQuickReferences:
        {
          gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (window_menu->view_quick_references), true);
          break;
        }
        case widMerge:
        {
          gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (window_menu->file_projects_merge), true);
          break;
        }
        case widResource:
        {
          on_file_resources_open(data);
          break;
        }
        case widOutline:
        {
          gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (window_menu->view_outline), true);
          break;
        }
        case widCheckKeyterms:
        {
          gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (window_menu->check_key_terms), true);
          break;
        }
        case widStyles:
        {
          on_goto_styles_area();
          break;
        }
        case widNotes:
        {
          view_project_notes();
          break;
        }
        case widReferences:
        {
          show_references_window();
          break;
        }
        case widEditor:
        {
          on_file_project_open(data);
          break;
        }
        case widMenu:
        {
          break;
        }
      }
      window_started = true;
    }
    windows_startup_pointer++;
  }
  if (windows_startup_pointer < window_data.ids.size()) {
    return true;
  } else {
    windows_startup_pointer = G_MAXINT;
  }

  // At the end of all focus the right editor, the one that had focus last time on shutdown.
  if (focused_project_last_session.empty()) {
    for (unsigned int i = 0; i < editor_windows.size(); i++) {
      if (focused_project_last_session == editor_windows[i]->window_data) {
        editor_windows[i]->present();
      }
    }
    focused_project_last_session.clear();
  }

  // We're through.
  return false;
}

void MainWindow::shutdown_windows()
// Shut any open windows down.
{
  // Keyterms in verse. 
  if (window_show_keyterms) {
    window_show_keyterms->shutdown();
    delete window_show_keyterms;
    window_show_keyterms = NULL;
  }

  // Quick references.
  if (window_show_quick_references) {
    window_show_quick_references->shutdown();
    delete window_show_quick_references;
    window_show_quick_references = NULL;
  }

  // Merge
  if (window_merge) {
    window_merge->shutdown();
    delete window_merge;
    window_merge = NULL;
  }

  // Resources.
  while (!resource_windows.empty()) {
    WindowResource * resource_window = resource_windows[0];
    resource_window->shutdown();
    delete resource_window;
    resource_windows.erase(resource_windows.begin());
  }

  // Outline.
  if (window_outline) {
    window_outline->shutdown();
    delete window_outline;
    window_outline = NULL;
  }

  // Check keyterms.
  if (window_check_keyterms) {
    window_check_keyterms->shutdown();
    delete window_check_keyterms;
    window_check_keyterms = NULL;
  }

  // Styles.
  if (window_styles) {
    window_styles->shutdown();
    delete window_styles;
    window_styles = NULL;
  }

  // Notes.
  if (window_notes) {
    window_notes->shutdown();
    delete window_notes;
    window_notes = NULL;
  }

  // References.
  if (window_references) {
    window_references->shutdown();
    delete window_references;
    window_references = NULL;
  }

  // Editors.
  while (!editor_windows.empty()) {
    WindowEditor * editor_window = editor_windows[0];
    editor_window->shutdown();
    delete editor_window;
    editor_windows.erase(editor_windows.begin());
  }

  // Menu.
  if (window_menu) {
    on_window_menu_delete_button();
  }

}

void MainWindow::on_window_focus_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_focus_button(button);
}

void MainWindow::on_window_focus_button(GtkButton *button)
// Called when a subwindow gets focused.
{
  // Store the focus if it is different from the currently stored values.
  GtkWidget * widget= GTK_WIDGET (button);
  if (widget != now_focused_signal_button) {
    last_focused_signal_button = now_focused_signal_button;
    now_focused_signal_button = widget;
  }

  // Present the windows.
  present_windows(widget);

  // Handle cases that an editor receives focus.
  handle_editor_focus();
}

void MainWindow::present_windows(GtkWidget * widget)
// If one window is focused, present them all.
{
  // Present all windows.
  if (window_show_quick_references)
    window_show_quick_references->present();
  if (window_show_keyterms)
    window_show_keyterms->present();
  if (window_merge)
    window_merge->present();
  for (unsigned int i = 0; i < resource_windows.size(); i++) {
    resource_windows[i]->present();
  }
  if (window_outline)
    window_outline->present();
  if (window_check_keyterms)
    window_check_keyterms->present();
  if (window_styles)
    window_styles->present();
  if (window_notes)
    window_notes->present();
  if (window_references)
    window_references->present();
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    editor_windows[i]->present();
  }
  if (window_menu)
    window_menu->present();

  // Present the calling window again so that it keeps the focus.
  if (window_show_quick_references) {
    if (widget == window_show_quick_references->focus_in_signal_button)
      window_show_quick_references->present();
  }
  if (window_show_keyterms) {
    if (widget == window_show_keyterms->focus_in_signal_button)
      window_show_keyterms->present();
  }
  if (window_merge) {
    if (widget == window_merge->focus_in_signal_button)
      window_merge->present();
  }
  for (unsigned int i = 0; i < resource_windows.size(); i++) {
    if (widget == resource_windows[i]->focus_in_signal_button)
      resource_windows[i]->present();
  }
  if (window_outline) {
    if (widget == window_outline->focus_in_signal_button)
      window_outline->present();
  }
  if (window_check_keyterms) {
    if (widget == window_check_keyterms->focus_in_signal_button)
      window_check_keyterms->present();
  }
  if (window_styles) {
    if (widget == window_styles->focus_in_signal_button)
      window_styles->present();
  }
  if (window_notes) {
    if (widget == window_notes->focus_in_signal_button)
      window_notes->present();
  }
  if (window_references) {
    if (widget == window_references->focus_in_signal_button)
      window_references->present();
  }
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    if (widget == editor_windows[i]->focus_in_signal_button)
      editor_windows[i]->present();
  }
  if (window_menu) {
    if (widget == window_menu->focus_in_signal_button)
      window_menu->present();
  }
}

gboolean MainWindow::on_mainwindow_focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer user_data) {
  ((MainWindow *) user_data)->mainwindow_focus_in_event(event);
  return FALSE;
}

void MainWindow::mainwindow_focus_in_event(GdkEventFocus *event) {
  present_windows(NULL);
}

gboolean MainWindow::on_mainwindow_window_state_event(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data) {
  ((MainWindow *) user_data)->mainwindow_window_state_event(event);
  return FALSE;
}

void MainWindow::mainwindow_window_state_event(GdkEventWindowState *event) {
  // Todo this works terribly gtk_window_iconify(GTK_WINDOW(mainwindow));
}

/*
 |
 |
 |
 |
 |
 Quick references
 |
 |
 |
 |
 |
 */

void MainWindow::on_view_quick_references_activate(GtkMenuItem *menuitem, gpointer user_data) {
  ((MainWindow *) user_data)->on_view_quick_references();
}

void MainWindow::on_view_quick_references() {
  on_window_show_quick_references_delete_button();
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (window_menu->view_quick_references))) {
    window_show_quick_references = new WindowShowQuickReferences (accelerator_group, windows_startup_pointer != G_MAXINT);
    g_signal_connect ((gpointer) window_show_quick_references->delete_signal_button, "clicked", G_CALLBACK (on_window_show_quick_references_delete_button_clicked), gpointer(this));
    g_signal_connect ((gpointer) window_show_quick_references->focus_in_signal_button, "clicked", G_CALLBACK (on_window_focus_button_clicked), gpointer(this));
    treeview_references_display_quick_reference();
  }
}

void MainWindow::on_window_show_quick_references_delete_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_window_show_quick_references_delete_button();
}

void MainWindow::on_window_show_quick_references_delete_button() {
  if (window_show_quick_references) {
    delete window_show_quick_references;
    window_show_quick_references = NULL;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (window_menu->view_quick_references), false);
  }
}

void MainWindow::on_show_quick_references_signal_button_clicked(GtkButton *button, gpointer user_data) {
  ((MainWindow *) user_data)->on_show_quick_references_signal_button(button);
}

void MainWindow::on_show_quick_references_signal_button(GtkButton *button) {
  if (!window_show_quick_references)
    return;
  WindowEditor * editor_window = last_focused_editor_window();
  if (!editor_window)
    return;
  extern Settings * settings;
  ustring project = settings->genconfig.project_get();
  window_show_quick_references->go_to(project, editor_window->editor->quick_references);
}

void MainWindow::treeview_references_display_quick_reference()
// Display the quick references.
{
  // Bail out if there's no quick references window or references window..
  if (window_show_quick_references == NULL)
    return;
  if (window_references == NULL)
    return;

  // Display the verses.
  extern Settings * settings;
  ustring project = settings->genconfig.project_get();
  window_show_quick_references->go_to(project, window_references->references);
}

/*
 |
 |
 |
 |
 |
 Accelerators
 |
 |
 |
 |
 |
 */

void MainWindow::accelerator_undo_callback(gpointer user_data) {
  ((MainWindow *) user_data)->accelerator_undo();
}

void MainWindow::accelerator_undo() {
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    if (now_focused_signal_button == editor_windows[i]->focus_in_signal_button) {
      editor_windows[i]->editor->undo();
    }
  }
  if (window_notes) {
    if (now_focused_signal_button == window_notes->focus_in_signal_button) {
      window_notes->undo();
    }
  }
}

void MainWindow::accelerator_redo_callback(gpointer user_data) {
  ((MainWindow *) user_data)->accelerator_redo();
}

void MainWindow::accelerator_redo() {
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    if (now_focused_signal_button == editor_windows[i]->focus_in_signal_button) {
      editor_windows[i]->editor->redo();
    }
  }
  if (window_notes) {
    if (now_focused_signal_button == window_notes->focus_in_signal_button) {
      window_notes->redo();
    }
  }
}

void MainWindow::accelerator_cut_callback(gpointer user_data) {
  ((MainWindow *) user_data)->accelerator_cut();
}

void MainWindow::accelerator_cut() {
  GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    if (now_focused_signal_button == editor_windows[i]->focus_in_signal_button) {
      Editor * editor = editor_windows[i]->editor;
      gtk_clipboard_set_text(clipboard, editor->text_get_selection ().c_str(), -1);
      editor->text_erase_selection();
    }
  }

  if (window_notes) {
    if (now_focused_signal_button == window_notes->focus_in_signal_button) {
      window_notes->cut();
    }
  }
}

void MainWindow::accelerator_copy_callback(gpointer user_data) {
  ((MainWindow *) user_data)->accelerator_copy();
}

void MainWindow::accelerator_copy() {
  GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    if (now_focused_signal_button == editor_windows[i]->focus_in_signal_button) {
      // In case of the text editor, the USFM code is copied, not the plain text. 
      Editor * editor = editor_windows[i]->editor;
      gtk_clipboard_set_text(clipboard, editor->text_get_selection ().c_str(), -1);
    }
  }

  if (window_check_keyterms) {
    if (now_focused_signal_button == window_check_keyterms->focus_in_signal_button) {
      window_check_keyterms->copy_clipboard();
    }
  }

  if (window_notes) {
    if (now_focused_signal_button == window_notes->focus_in_signal_button) {
      window_notes->copy();
    }
  }
}

void MainWindow::accelerator_paste_callback(gpointer user_data) {
  ((MainWindow *) user_data)->accelerator_paste();
}

void MainWindow::accelerator_paste() {
  GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  if (!gtk_clipboard_wait_is_text_available(clipboard))
    return;

  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    if (now_focused_signal_button == editor_windows[i]->focus_in_signal_button) {
      Editor * editor = editor_windows[i]->editor;
      gchar * text = gtk_clipboard_wait_for_text(clipboard);
      if (text) {
        editor->text_insert(text);
        g_free(text);
      }
    }
  }

  if (window_notes) {
    if (now_focused_signal_button == window_notes->focus_in_signal_button) {
      window_notes->paste();
    }
  }
}

void MainWindow::accelerator_standard_text_1_callback(gpointer user_data) {
  ((MainWindow *) user_data)->accelerator_standard_text_n(0);
}

void MainWindow::accelerator_standard_text_2_callback(gpointer user_data) {
  ((MainWindow *) user_data)->accelerator_standard_text_n(1);
}

void MainWindow::accelerator_standard_text_3_callback(gpointer user_data) {
  ((MainWindow *) user_data)->accelerator_standard_text_n(2);
}

void MainWindow::accelerator_standard_text_4_callback(gpointer user_data) {
  ((MainWindow *) user_data)->accelerator_standard_text_n(3);
}

void MainWindow::accelerator_standard_text_n(unsigned int selector) {
  if (window_notes) {
    // Insert the text if the notes window has focus.
    if (now_focused_signal_button == window_notes->focus_in_signal_button) {
      window_notes->insert_standard_text(selector);
    }
  }
}

void MainWindow::accelerator_new_project_note_callback(gpointer user_data) {
  ((MainWindow *) user_data)->on_new_note();
}

void MainWindow::accelerator_next_verse_callback(gpointer user_data) {
  ((MainWindow *) user_data)->goto_next_verse();

}

void MainWindow::accelerator_previous_verse_callback(gpointer user_data) {
  ((MainWindow *) user_data)->goto_previous_verse();
}

void MainWindow::accelerator_next_chapter_callback(gpointer user_data) {
  ((MainWindow *) user_data)->goto_next_chapter();
}

void MainWindow::accelerator_previous_chapter_callback(gpointer user_data) {
  ((MainWindow *) user_data)->goto_previous_chapter();
}

void MainWindow::accelerator_next_book_callback(gpointer user_data) {
  ((MainWindow *) user_data)->goto_next_book();
}

void MainWindow::accelerator_previous_book_callback(gpointer user_data) {
  ((MainWindow *) user_data)->goto_previous_book();
}

void MainWindow::accelerator_next_reference_in_history_callback(gpointer user_data) {
  ((MainWindow *) user_data)->accelerator_next_reference_in_history();
}

void MainWindow::accelerator_next_reference_in_history() {
  navigation.on_forward();
}

void MainWindow::accelerator_previous_reference_in_history_callback(gpointer user_data) {
  ((MainWindow *) user_data)->accelerator_previous_reference_in_history();
}

void MainWindow::accelerator_previous_reference_in_history() {
  navigation.on_back();
}

void MainWindow::accelerator_go_to_reference_callback(gpointer user_data) {
  ((MainWindow *) user_data)->goto_reference_interactive();
}

void MainWindow::accelerator_close_window_callback(gpointer user_data) {
  ((MainWindow *) user_data)->accelerator_close_window();
}

void MainWindow::accelerator_close_window()
// Closes the focused window.
{
  // Keyterms in verse. 
  if (window_show_keyterms) {
    if (now_focused_signal_button == window_show_keyterms->focus_in_signal_button) {
      on_window_show_keyterms_delete_button();
    }
  }

  // Quick references.
  if (window_show_quick_references) {
    if (now_focused_signal_button == window_show_quick_references->focus_in_signal_button) {
      on_window_show_quick_references_delete_button();
    }
  }

  // Merge
  if (window_merge) {
    if (now_focused_signal_button == window_merge->focus_in_signal_button) {
      on_window_merge_delete_button();
    }
  }

  // Resources.
  for (unsigned int i = 0; i < resource_windows.size(); i++) {
    WindowResource * resource_window = resource_windows[i];
    if (now_focused_signal_button == resource_window->focus_in_signal_button) {
      on_window_resource_delete_button(GTK_BUTTON(resource_window->delete_signal_button));
      break;
    }
  }

  // Outline.
  if (window_outline) {
    if (now_focused_signal_button == window_outline->focus_in_signal_button) {
      on_window_outline_delete_button();
    }
  }

  // Check keyterms.
  if (window_check_keyterms) {
    if (now_focused_signal_button == window_check_keyterms->focus_in_signal_button) {
      on_window_check_keyterms_delete_button();
    }
  }

  // Styles.
  if (window_styles) {
    if (now_focused_signal_button == window_styles->focus_in_signal_button) {
      on_window_styles_delete_button();
    }
  }

  // Notes.
  if (window_notes) {
    if (now_focused_signal_button == window_notes->focus_in_signal_button) {
      on_window_notes_delete_button();
    }
  }

  // References.
  if (window_references) {
    if (now_focused_signal_button == window_references->focus_in_signal_button) {
      on_window_references_delete_button();
    }
  }

  // Editors.
  for (unsigned int i = 0; i < editor_windows.size(); i++) {
    WindowEditor * editor_window = editor_windows[i];
    if (now_focused_signal_button == editor_window->focus_in_signal_button) {
      on_window_editor_delete_button(GTK_BUTTON(editor_window->delete_signal_button));
      break;
    }
  }
  handle_editor_focus();

  // Menu.
  if (window_menu) {
    if (now_focused_signal_button == window_menu->focus_in_signal_button) {
      gtk_main_quit();
    }
  }

}

void MainWindow::accelerator_goto_styles_area_callback(gpointer user_data) {
  ((MainWindow *) user_data)->on_goto_styles_area();
}

void MainWindow::accelerator_quit_program_callback(gpointer user_data) {
  gtk_main_quit();
}

void MainWindow::accelerator_activate_text_area_callback(gpointer user_data) {
  ((MainWindow *) user_data)->on_text_area_activate();
}

void MainWindow::accelerator_activate_tools_area_callback(gpointer user_data) {
  ((MainWindow *) user_data)->on_tools_area_activate();
}

void MainWindow::accelerator_activate_notes_area_callback(gpointer user_data) {
  ((MainWindow *) user_data)->on_notes_area_activate();
}

void MainWindow::accelerator_next_reference_in_reference_area_callback(gpointer user_data) {
  ((MainWindow *) user_data)->on_next_reference();
}
void MainWindow::accelerator_previous_reference_in_reference_area_callback(gpointer user_data) {
  ((MainWindow *) user_data)->on_previous_reference();
}
void MainWindow::accelerator_next_project_callback(gpointer user_data) {
  ((MainWindow *) user_data)->goto_next_previous_project(true);
}

void MainWindow::accelerator_previous_project_callback(gpointer user_data) {
  ((MainWindow *) user_data)->goto_next_previous_project(false);
}

void MainWindow::accelerator_open_project_callback(gpointer user_data) {
  ((MainWindow *) user_data)->open();
}

void MainWindow::accelerator_print_callback(gpointer user_data) {
  ((MainWindow *) user_data)->on_print();
}

void MainWindow::accelerator_copy_without_formatting_callback(gpointer user_data) {
  ((MainWindow *) user_data)->on_copy_without_formatting();
}

void MainWindow::accelerator_find_callback(gpointer user_data) {
  ((MainWindow *) user_data)->menu_findspecial();
}

void MainWindow::accelerator_replace_callback(gpointer user_data) {
  ((MainWindow *) user_data)->menu_replace();
}

void MainWindow::accelerator_main_help_callback(gpointer user_data) {
  ((MainWindow *) user_data)->on_help_main();
}

void MainWindow::accelerator_context_help_callback(gpointer user_data) {
  ((MainWindow *) user_data)->on_help_context();
}

void MainWindow::accelerator_menu_callback(gpointer user_data) {
  ((MainWindow *) user_data)->display_menu_window();
}

/*

 Todo Improve the window layout system.

 Urgent: To revert to mainwindow being the menu one again, as the current system is bad.
 further to track the state of a window in the signal callback, whether hidden or not.
 If not hidden, not to present it. If hidden to present it.

 The styles menu should normally be disabled when there's no stylesheet opened.
 When "Open" is chosen, then it points to the stylesheet that currently belongs to the project,
 and suggests to open it.
 Otherwise all other options are disabled.
 When the styles window closes again, the menu is disabled again.
 Under normal condition the menu is operated by the styles window object.

 We need to look at the "todo" entries in windownotes.h/cpp.

 Adding text to notes by accelerators, and by the menu.
 Adding the current reference to the note.
 If the notes window shows up on startup, it does now not display the relevant notes.
 
 There are still a lot of accelerators that should be added to the system.

 Add all the accelerators in a helpfile, build the file while adding the accelerator.
 
 To make a View / Tile menu. And a stack one. 
 When a new window is opened, for example, a new editor is opened, it then automatically finds a place
 within the existing editors, and tiles (or stacks) the windows.

 If the positions are reset, then all standard positions are put to zero, so that next time they show up,
 they will be allocated into the new position.
 
 Eventually the menu will also become an independent window, and can be clicked away to disappear from the screen.
 But ideally we would like to stop the program if the menu is clicked away.
 We can resolved it this way.
 - If the menu is clicked away, then it asks whether to stop the program. If so, yes, if not, the menu goes away.
 - If the last window is clicked away, then the program stops too.
 - If Ctrl-Q is typed, it stops too, and this applies in any window.
 - If Ctrl-W is clicked in any window, it goes away, and if it is the last one, the program quits.

 When all the windows are done, then we need to check whether all menu entries work in each window,
 and whether all shortcuts in each relevant window.
 When all windows have been detached, we need to verify copy and paste through the menu and through shortcuts.
 
 All those places where it has "References references", it is to be looked into whether that cannot be moved
 into the references window itself, rather than cluttering the code in MainWindow.

 If Ctrl-N is pressed in an already existing notes window, the Ctrl-1/4 shortcuts don't work unless the window
 is clicked so as to focus it. No, even after clicking in the window, the shortcuts don't work.

 We have to put the footnotes in separate GtkTextViews. 
 So we can then use as many GtkTextViews as there are footnotes. 
 And then the automatic width routines go away.
 This is done in the same scrolled window, just by adding the required number of GtkTextViews to the vertical box.
 Each note consists of a horizontal box, with the caller in it as a label, and then a GtkTextView for the note.
 The up / down arrows keep working, so one can move from one GtkTextView to another, just as it is now.
 
 Clicking on a project notes [references] should bring up the references in the window. It does not do that now. 
 
 When selecting the note category there's a lot of redundant focusing going around, and same applies to deleting a note.
 
 The following routines need attention to their code that has been commented out:
 void MainWindow::menu_redo()
 void MainWindow::menu_edit()
 void MainWindow::menu_undo() 
 void MainWindow::on_tools_area_activate() - This needs a routine that tracks the last focused tool, and this is
 the tool to be focused when the area is activated.
 void MainWindow::on_cut() 
 void MainWindow::on_copy()
 void MainWindow::on_copy_without_formatting()
 void MainWindow::on_paste() 
 
 For the various functions that can be called by either the menu or by a shortcut, 
 the called functions should be integrated into one, and should further know
 whether it was called from the menu or from a shortcut.
 
 Hi Teus, Downloaded this version, BE opens in 5 windows Text, Quickreference, Styles, Merge, Resources, 
 where not project is loaded. As soon as I try to load a project (click file/open) BE crashes. Wolfgang
 
 If there's no project, disable the Ctrl-P accelerator.

 As BibleWorks runs under Linux without the extra translations, what we need to do is to move those extra translations
 to BibleTime, and then run BibleWorks under Linux. Else they could be moved to a Resource, but that would require
 search functionality to be added to the Resource, which is a thing we are not now looking for. It could be put in as 
 a feature request though.
 
 
 */

