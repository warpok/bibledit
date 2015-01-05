/*
 ** Copyright (©) 2003-2013 Teus Benschop.
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
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **  
 */


#include "libraries.h"
#include "directories.h"
#include <glib.h>
#include <config.h>
#include "constants.h"
#include "utilities.h"
#include "gwrappers.h"
#include "shell.h"
#include "unixwrappers.h"
#include "tiny_utilities.h"
#include "restore.h"

// Constructor
directories::directories(char *argv0)
{
  char *dirname = (char *)g_path_get_dirname (argv0);
  rundir = dirname;
  free(dirname);
  char *basename = (char *)g_path_get_basename (argv0);
  exename = basename;
  free(basename);

  // Instead of re-computing the directory every time
  // get_package_data() is called, we compute it once and store it,
  // making repeated calls more efficient by simply returning the
  // answer we have pre-computed.
#ifdef WIN32
    // A clever way to take a path like C:\Program
    // Files\Bibledit-Gtk\editor\bin\ and chop the
    // last component to get back to <something>\Bibledit-Gtk\editor...
    package_data = gw_path_get_dirname(dirname);
    // ... Then add two more dirs back on, resulting in
    // <something>Bibledit-Gtk\editor\share\bibledit
    package_data = gw_build_filename(package_data, "share", "bibledit");
  }
#else
  // For Linux, this is hard-coded to match the variable set in config.h
  package_data = PACKAGE_DATA_DIR;
#endif
}

directories::~directories()
{
}

void directories::check_structure()
{
  restore_all_stage_two ();
  gw_mkdir_with_parents(get_root());
  gw_mkdir_with_parents(get_projects());
  gw_mkdir_with_parents(get_notes());
  gw_mkdir_with_parents(get_stylesheets());
  gw_mkdir_with_parents(get_configuration());
  gw_mkdir_with_parents(get_pictures());
  gw_mkdir_with_parents(get_resources());
  gw_mkdir_with_parents(get_scripts());
  gw_mkdir_with_parents(get_temp());
  gw_mkdir_with_parents(get_templates());
  gw_mkdir_with_parents(get_templates_user());
}


ustring directories::get_root()
// Returns the root directory of all data.
{
  return tiny_directories_get_root();
}


ustring directories::get_projects()
{
  // This returns the directory with all the projects.
  return tiny_directories_get_projects();
}


ustring directories::get_notes()
{
  // This returns the directory with the notes
  return gw_build_filename(get_root(), "notes");
}


ustring directories::get_stylesheets()
{
  // This returns the directory with the stylesheets
  return gw_build_filename(get_root(), "stylesheets");
}


ustring directories::get_configuration()
{
  // This returns the directory with the configuration
  return gw_build_filename(get_root(), "configuration");
}


ustring directories::get_pictures()
{
  // This returns the directory with the pictures
  return gw_build_filename(get_root(), "pictures");
}


ustring directories::get_resources()
{
  // This returns the directory with the resources.
  return gw_build_filename(get_root(), "resources");
}


ustring directories::get_scripts()
{
  // This returns the directory with the scripts.
  return gw_build_filename(get_root(), "scripts");
}


ustring directories::get_temp()
{
  // Returns the temporal directory bibledit uses.
  return gw_build_filename(g_get_tmp_dir(), "bibledit");
}


ustring directories::get_templates()
{
  // This returns the directory with the templates
  return gw_build_filename(get_temp(), "templates");
}


ustring directories::get_templates_user()
{
  // This returns the directory with the User's custom raw templates
  return gw_build_filename(get_root(), "templates");
}


ustring directories::get_package_data()
// Gives the package data directory, cross platform.
{
  return package_data;
}

ustring directories::get_restore ()
// The directory, if found, to restore from.
{
  ustring directory;
  directory = get_root() + ".restored";
  return directory;
}

