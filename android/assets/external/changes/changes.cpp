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


#include <changes/changes.h>
#include <assets/header.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <config/logic.h>
#include <database/modifications.h>
#include <database/notes.h>
#include <trash/handler.h>
#include <ipc/focus.h>
#include <navigation/passage.h>
#include <changes/logic.h>


string changes_changes_url ()
{
  return "changes/changes";
}


bool changes_changes_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string changes_changes (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Modifications database_modifications = Database_Modifications ();
  Database_Notes database_notes = Database_Notes (request);
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Changes"), request);
  header.setStylesheet ();
  page += header.run ();
  Assets_View view = Assets_View ();
  
  
  string username = request->session_logic()->currentUser ();
  
  
  // Handle AJAX call to remove a change notification.
  if (request->post.count ("remove")) {
    int remove = convert_to_int (request->post["remove"]);
    trash_change_notification (request, remove);
    database_modifications.deleteNotification (remove);
    if (config_logic_client_prepared ()) {
      request->database_config_user ()->addRemovedChange (remove);
    }
    request->database_config_user ()->setChangeNotificationsChecksum ("");
    return "";
  }
  
  
  // Handle AJAX call to navigate to the passage belonging to the change notification.
  if (request->post.count ("navigate")) {
    string navigate = request->post["navigate"];
    int id = convert_to_int (navigate);
    Passage passage = database_modifications.getNotificationPassage (id);
    if (passage.book) {
      Ipc_Focus::set (request, passage.book, passage.chapter, convert_to_int (passage.verse));
      Navigation_Passage::recordHistory (request, passage.book, passage.chapter, convert_to_int (passage.verse));
    }
    // Set the correct default Bible for the user.
    string bible = database_modifications.getNotificationBible (id);
    if (!bible.empty ()) request->database_config_user()->setBible (bible);
    return "";
  }
  
  
  // Remove personal changes notifications and their matching change notifications in the Bible.
  if (request->query.count ("match")) {
    vector <int> ids = database_modifications.clearNotificationMatches (username, changes_personal_category (), changes_bible_category ());
    // Client records deletions for sending to the Cloud.
    if (config_logic_client_prepared ()) {
      for (auto & id : ids) {
        request->database_config_user ()->addRemovedChange (id);
      }
    }
    // Clear checksum cache.
    request->database_config_user ()->setChangeNotificationsChecksum ("");
  }
  
  
  // Remove all the personal change notifications.
  if (request->query.count ("personal")) {
    vector <int> ids = database_modifications.getNotificationTeamIdentifiers (username, changes_personal_category (), true);
    for (auto id : ids) {
      trash_change_notification (request, id);
      database_modifications.deleteNotification (id);
      if (config_logic_client_prepared ()) {
        request->database_config_user ()->addRemovedChange (id);
      }
      request->database_config_user ()->setChangeNotificationsChecksum ("");
    }
  }
  
  
  // Remove all the Bible change notifications.
  if (request->query.count ("bible")) {
    vector <int> ids = database_modifications.getNotificationTeamIdentifiers (username, changes_bible_category (), true);
    for (auto id : ids) {
      trash_change_notification (request, id);
      database_modifications.deleteNotification (id);
      if (config_logic_client_prepared ()) {
        request->database_config_user ()->addRemovedChange (id);
      }
      request->database_config_user ()->setChangeNotificationsChecksum ("");
    }
  }
  
  
  // Remove all the change notifications made by a certain user.
  if (request->query.count ("dismiss")) {
    string user = request->query ["dismiss"];
    vector <int> ids = database_modifications.getNotificationTeamIdentifiers (username, user, true);
    for (auto id : ids) {
      trash_change_notification (request, id);
      database_modifications.deleteNotification (id);
      if (config_logic_client_prepared ()) {
        request->database_config_user ()->addRemovedChange (id);
      }
      request->database_config_user ()->setChangeNotificationsChecksum ("");
    }
  }
  
  
  // Read the identifiers.
  // Limit the number of results to keep the page reasonabley fast even if there are many notifications.
  vector <int> personal_ids = database_modifications.getNotificationPersonalIdentifiers (username, changes_personal_category (), true);
  vector <int> bible_ids = database_modifications.getNotificationTeamIdentifiers (username, changes_bible_category (), true);
  vector <int> ids = database_modifications.getNotificationIdentifiers (username, true);

  
  string textblock;
  for (auto id : ids) {
    Passage passage = database_modifications.getNotificationPassage (id);
    string link = filter_passage_link_for_opening_editor_at (passage.book, passage.chapter, passage.verse);
    string category = database_modifications.getNotificationCategory (id);
    if (category == changes_personal_category ()) category = "😊";
    if (category == changes_bible_category ()) category = "📖";
    string modification = database_modifications.getNotificationModification (id);
    textblock.append ("<div id=\"entry" + convert_to_string (id) + "\">\n");
    textblock.append ("<a href=\"expand\" id=\"expand" + convert_to_string (id) + "\"> ⊞ </a>\n");
    textblock.append ("<a href=\"remove\" id=\"remove" + convert_to_string (id) + "\"> ✗ </a>\n");
    textblock.append (link + "\n");
    textblock.append (category + "\n");
    textblock.append (modification + "\n");
    textblock.append ("</div>\n");
  }
  view.set_variable ("textblock", textblock);
  
  
  string loading = "\"" + translate("Loading ...") + "\"";
  string script = "var loading = " + loading + ";";
  view.set_variable ("script", script);

  
  // Enable links to dismiss categories of notifications depending on whether there's anything to dismiss.
  if (!personal_ids.empty () && !bible_ids.empty ()) view.enable_zone ("matching");
  if (!personal_ids.empty ()) view.enable_zone ("personal");
  if (!bible_ids.empty ()) view.enable_zone ("bible");
  
  
  // Add links to clear the notifications from the individual contributors.
  string dismissblock;
  vector <string> users = request->database_users ()->getUsers ();
  for (auto & user : users) {
    vector <int> ids = database_modifications.getNotificationTeamIdentifiers (username, user, true);
    if (!ids.empty ()) {
      dismissblock.append ("<p>* <a href=\"?dismiss=" + user + "\">" + user + " " + translate("(all of them)")+ "</a></p>\n");
    }
  }
  view.set_variable ("dismissblock", dismissblock);
  
  
  page += view.render ("changes", "changes");
  
  
  page += Assets_Page::footer ();
  return page;
}
