/*
Copyright (©) 2003-2014 Teus Benschop.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include <webserver/request.h>


using namespace std;


Webserver_Request::Webserver_Request ()
{
  response_code = 200;
}


Webserver_Request::~Webserver_Request ()
{
  if (session_logic_instance) delete session_logic_instance;
  if (database_config_user_instance) delete database_config_user_instance;
  if (database_users_instance) delete database_users_instance;
  if (database_styles_instance) delete database_styles_instance;
  if (database_bibles_instance) delete database_bibles_instance;
}


// Returns a pointer to a live Session_Logic object.
Session_Logic * Webserver_Request::session_logic ()
{
  // Single live object during the entire web request.
  if (!session_logic_instance) session_logic_instance = new Session_Logic (this);
  return session_logic_instance;
}


// Returns a pointer to a live Database_Config_User object.
Database_Config_User * Webserver_Request::database_config_user ()
{
  // Single live object during the entire web request.
  if (!database_config_user_instance) database_config_user_instance = new Database_Config_User (this);
  return database_config_user_instance;
}


// Returns a pointer to a live Database_Users object.
Database_Users * Webserver_Request::database_users ()
{
  // Single live object during the entire web request.
  if (!database_users_instance) database_users_instance = new Database_Users ();
  return database_users_instance;
}



Database_Styles * Webserver_Request::database_styles ()
{
  if (!database_styles_instance) database_styles_instance = new Database_Styles ();
  return database_styles_instance;
}


Database_Bibles * Webserver_Request::database_bibles ()
{
  if (!database_bibles_instance) database_bibles_instance = new Database_Bibles ();
  return database_bibles_instance;
}
