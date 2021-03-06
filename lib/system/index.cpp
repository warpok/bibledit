/*
Copyright (©) 2003-2016 Teus Benschop.

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


#include <system/index.h>
#include <system/timezone.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <dialog/list.h>
#include <database/config/general.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <config/globals.h>


string system_index_url ()
{
  return "system/index";
}


bool system_index_acl (void * webserver_request)
{
#ifdef HAVE_CLIENT
  (void) webserver_request;
  // Client: Anyone has make system settings, basically.
  return true;
#else
  // Cloud: Manager can make system settings.
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
#endif
}


string system_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string page;
  string success;
  string error;

  
  // The available localizations.
  map <string, string> localizations = locale_logic_localizations ();

  
  // User can set the system language.
  // This is to be done before displaying the header.
  if (request->query.count ("language")) {
    string language = request->query ["language"];
    if (language == "select") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Set the language for Bibledit"), "", "");
      for (auto element : localizations) {
        dialog_list.add_row (element.second, "language", element.first);
      }
      page = Assets_Page::header ("", webserver_request);
      page += dialog_list.run ();
      return page;
    } else {
      Database_Config_General::setSiteLanguage (locale_logic_filter_default_language (language));
    }
  }

  
  // The header: The language has been set already.
  Assets_Header header = Assets_Header (translate("System"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();

  
  Assets_View view;

  
  // Set the language on the page.
  string language = locale_logic_filter_default_language (Database_Config_General::getSiteLanguage ());
  language = localizations [language];
  view.set_variable ("language", language);

  
  // Entry of time zone offset in hours.
  if (request->post.count ("savezone")) {
    string input = request->post ["timezone"];
    input = filter_string_str_replace ("UTC", "", input);
    int timezone = convert_to_int (input);
    bool clipped = false;
    if (timezone < MINIMUM_TIMEZONE) {
      timezone = MINIMUM_TIMEZONE;
      clipped = true;
    }
    if (timezone > MAXIMUM_TIMEZONE) {
      timezone = MAXIMUM_TIMEZONE;
      clipped = true;
    }
    if (clipped) error = translate ("The timezone was clipped");
    Database_Config_General::setTimezone (timezone);
    success = translate ("The timezone was saved");
  }
  // Set the time zone offset in the GUI.
  int timezone = Database_Config_General::getTimezone();
  view.set_variable ("timezone", convert_to_string (timezone));
  // Display the section to set the site's timezone only
  // in case the calling program has not yet set this zone in the library.
  // So for example the app for iOS can set the timezone from the device,
  // and in case this has been done, the user no longer can set it through Bibledit.
  if ((config_globals_timezone_offset_utc < MINIMUM_TIMEZONE)
      || (config_globals_timezone_offset_utc > MAXIMUM_TIMEZONE)) {
    view.enable_zone ("timezone");
  }


  // Set some feedback, if any.
  view.set_variable ("success", success);
  view.set_variable ("error", error);

  
  page += view.render ("system", "index");
  page += Assets_Page::footer ();
  return page;
}
