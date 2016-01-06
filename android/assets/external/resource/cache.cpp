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


#include <resource/cache.h>
#include <resource/external.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <tasks/logic.h>
#include <journal/index.h>
#include <sword/logic.h>
#include <config/logic.h>
#include <menu/logic.h>
#include <database/config/general.h>
#include <sendreceive/resources.h>


string resource_cache_url ()
{
  return "resource/cache";
}


bool resource_cache_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string resource_cache (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string resource = request->query ["resource"];
  
  
  string page;
  Assets_Header header = Assets_Header (menu_logic_resource_cache_text (), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view;

  
  if (request->query.count ("clear")) {
    sendreceive_resources_clear_all ();
  }
  vector <string> resources = Database_Config_General::getResourcesToCache ();
  if (!resources.empty ()) {
    view.enable_zone ("scheduled");
    view.set_variable ("scheduled", filter_string_implode (resources, " | "));
  }
  
  
  string block;
  resources = resource_external_names ();
  for (auto & resource : resources) {
    block.append ("<p>");
    block.append ("<a href=\"download?name=" + resource + "\">" + resource + "</a>");
    block.append ("</p>\n");
  }
  resources = sword_logic_get_available ();
  for (auto & resource : resources) {
    string source = sword_logic_get_source (resource);
    string module = sword_logic_get_remote_module (resource);
    string name = "[" + source + "][" + module + "]";
    block.append ("<p>");
    block.append ("<a href=\"download?name=" + name + "\">" + resource + "</a>");
    block.append ("</p>\n");
  }
  view.set_variable ("block", block);

  
  page += view.render ("resource", "cache");
  page += Assets_Page::footer ();
  return page;
}