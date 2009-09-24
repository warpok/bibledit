/*
** Copyright (©) 2003-2009 Teus Benschop.
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


#ifndef INCLUDED_D_BUS_H
#define INCLUDED_D_BUS_H


#include "libraries.h"
#include <dbus/dbus.h>
#include <gtk/gtkbutton.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>


enum DBusNameType {dbntNone, dbntOrgBibleditMain};

enum DBusMethodType {dbmtHello, dbmtEnd};


class DBus
{
public:
  DBus (DBusNameType name);
  ~DBus ();




  // Old.
  void send (DBusNameType destination, DBusMethodType, const vector<ustring>& payload);
  vector<ustring> get_payload (DBusMethodType method);
  void erase_payload (DBusMethodType method);
  void methodcall_add_signal (DBusMethodType method);
  GtkWidget * method_called_signal;
  void methodcall_remove_all_signals ();



private:
  DBusConnection *con;
	DBusGConnection *sigcon;
  void check_names_on_bus ();












  // Old.
  const gchar * dbusname (DBusNameType dbname);
  const gchar * dbuspath ();
  const gchar * dbusinterface ();
  const gchar * dbusmethod (DBusMethodType dbmethod);
  DBusMethodType dbusmethod (const char * dbmethod);
  void retrieve_message (DBusMessage *message);
  void retrieve_iter (DBusMessageIter *iter);
  int message_type;
  vector<ustring> string_reply;
  static void listener_start (gpointer data);
  void listener_main ();
  bool listener_run;
  bool listener_running;
  void log (const ustring& message, bool critical);
  void respond (DBusMessage* msg, const ustring& response);
  map<DBusMethodType, vector<ustring> > methodcalls;
  set<DBusMethodType> signalling_methods;
};


#endif
