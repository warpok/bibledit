/*
 Copyright (©) 2003-2015 Teus Benschop.
 
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


#ifndef INCLUDED_RESOURCE_SWORD_H
#define INCLUDED_RESOURCE_SWORD_H


#include <config/libraries.h>


string resource_sword_url ();
bool resource_sword_acl (void * webserver_request);
string resource_sword (void * webserver_request);

string resource_sword_get_path ();
void resource_sword_refresh_module_list ();
string resource_sword_module_list_path ();
string resource_sword_get_source (string line);
string resource_sword_get_remote_module (string line);
string resource_sword_get_installed_module (string line);
string resource_sword_get_version (string line);
string resource_sword_get_name (string line);
void resource_sword_install_module (string source, string module);
void resource_sword_update_module (string source, string module);
void resource_sword_uninstall_module (string module);
vector <string> resource_sword_get_available ();
vector <string> resource_sword_get_installed ();
string resource_sword_get_text (string source, string module, int book, int chapter, int verse);


#endif