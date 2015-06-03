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


#include <editverse/save.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <checksum/logic.h>
#include <database/modifications.h>
#include <database/logs.h>
#include <locale/translate.h>


string editverse_save_url ()
{
  return "editverse/save";
}


bool editverse_save_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string editverse_save (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  Database_Modifications database_modifications = Database_Modifications ();

  
  // Check on information about where to save the verse.
  bool save = (request->post.count ("bible") && request->post.count ("book") && request->post.count ("chapter") && request->post.count ("verse") && request->post.count ("usfm"));
  if (!save) {
    return translate("Don't know where to save");
  }

  
  string bible = request->post["bible"];
  int book = convert_to_int (request->post["book"]);
  int chapter = convert_to_int (request->post["chapter"]);
  int verse = convert_to_int (request->post["verse"]);
  string usfm = request->post["usfm"];
  string checksum = request->post["checksum"];

  
  // Checksum.
  if (Checksum_Logic::get (usfm) != checksum) {
    request->response_code = 409;
    return translate ("Checksum error");
  }

  
  // Check there's anything to save at all.
  usfm = filter_string_trim (usfm);
  if (usfm == "") {
    return translate("Nothing to save");
  }
  
  
  // Check on valid UTF-8.
  if (!unicode_string_is_valid (usfm)) {
    return translate("Cannot save: Needs Unicode");
  }
  
  
  // Collect some data about the changes for this user.
  string username = request->session_logic()->currentUser ();
  int oldID = request->database_bibles()->getChapterId (bible, book, chapter);
  string oldText = request->database_bibles()->getChapter (bible, book, chapter);
  // Safely store the verse.
  bool saved = usfm_safely_store_verse (request, bible, book, chapter, verse, usfm);
  if (saved) {
    // Store details for the user's changes.
    int newID = request->database_bibles()->getChapterId (bible, book, chapter);
    string newText = request->database_bibles()->getChapter (bible, book, chapter);
    database_modifications.recordUserSave (username, bible, book, chapter, oldID, oldText, newID, newText);
    return translate("Saved");
  } else {
    return translate("Not saved because of too many changes");
  }

  
  return "";
}