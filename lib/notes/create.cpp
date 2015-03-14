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


#include <notes/create.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <notes/logic.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <notes/index.h>


string notes_create_url ()
{
  return "notes/create";
}


bool notes_create_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string notes_create (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Notes database_notes = Database_Notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Create note"), request);
  page += header.run();
  
  Assets_View view = Assets_View ();

  
  // Is is possible to pass a Bible to this script.
  // The note will then be created for this Bible.
  // If no Bible is passed, it takes the user's active Bible.
  string bible = request->post ["bible"];
  if (bible.empty ()) {
    bible = access_bible_clamp (webserver_request, request->database_config_user()->getBible ());
  }
  
  
  int book;
  if (request->post.count ("book")) book = convert_to_int (request->post ["book"]);
  else book = Ipc_Focus::getBook (webserver_request);
  int chapter;
  if (request->post.count ("chapter")) chapter = convert_to_int (request->post ["chapter"]);
  else chapter = Ipc_Focus::getChapter (webserver_request);
  int verse;
  if (request->post.count ("verse")) verse = convert_to_int (request->post ["verse"]);
  else verse = Ipc_Focus::getVerse (webserver_request);

  
  if (request->post.count ("submit")) {
    string summary = filter_string_trim (request->post["summary"]);
    string contents = filter_string_trim (request->post["contents"]);
    notes_logic.createNote (bible, book, chapter, verse, summary, contents, false);
    redirect_browser (request, notes_index_url ());
    return "";
  }

  
  if (request->post.count ("cancel")) {
    redirect_browser (request, notes_index_url ());
    return "";
  }
  
  
 
  
  /* Todo

  
  // This script can be called from a change notification.
  // It will then create a note based on that change notification.
  @$fromchange = request->query ["fromchange"];
  if (isset ($fromchange)) {
    $database_modifications = Database_Modifications::getInstance ();
    $database_bibles = Database_Bibles::getInstance ();
    $bible = $database_modifications->getNotificationBible ($fromchange);
    $summary = translate("Query about a change in the text");
    $contents = "<p>" . translate("Old text:") . "</p>";
    $contents += $database_modifications->getNotificationOldText ($fromchange);
    $contents += "<p>" .  translate("Change:") . "</p>";
    $contents += "<p>" . $database_modifications->getNotificationModification ($fromchange) . "</p>";
    $contents += "<p>" . translate("New text:") . "</p>";
    $contents += $database_modifications->getNotificationNewText ($fromchange);
    $view.set_variable ("summary = $summary;
    $view.set_variable ("contents = $contents;
  }
                                                                                                                                                */
  view.set_variable ("bible", bible);
  view.set_variable ("book", to_string (book));
  view.set_variable ("chapter", to_string (chapter));
  view.set_variable ("verse", to_string (verse));
  string passage = filter_passage_display (book, chapter, to_string (verse));
  view.set_variable ("passage", passage);
                                                                                                      
  
  page += view.render ("notes", "create");
  
  page += Assets_Page::footer ();
  
  return page;
}