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


#include <edit/focus.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <ipc/focus.h>
#include <editor/usfm2html.h>


string edit_focus_url ()
{
  return "edit/focus";
}


bool edit_focus_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string edit_focus (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
 
  
  // Get Bible: If an empty Bible is given, bail out.
  string bible = request->query ["bible"];
  if (bible.empty ()) return "";
  // Get book: If no book is given: Bail out.
  int book = convert_to_int (request->query ["book"]);
  if (!book) return "";
  // Get chapter.
  int chapter = convert_to_int (request->query ["chapter"]);
  
  
  string stylesheet = request->database_config_user()->getStylesheet ();
  string usfm = request->database_bibles()->getChapter (bible, book, chapter);
  int verse = Ipc_Focus::getVerse (request);


  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (stylesheet);
  editor_usfm2html.run ();
  
  int startingOffset = 0;
  int endingOffset = 0;
  // To deal with a combined verse, go through the offsets, and pick the correct one.
  for (auto element : editor_usfm2html.verseStartOffsets) {
    int vs = element.first;
    int offset = element.second;
    if (vs <= verse) startingOffset = offset;
    if (endingOffset == 0) {
      if (vs > verse) {
        endingOffset = offset;
      }
    }
  }
  if (verse) {
    startingOffset += convert_to_string (verse).length () + 1;
  }
  if (endingOffset) {
    endingOffset--;
  } else {
    endingOffset = editor_usfm2html.textLength;
  }
  
  string data = convert_to_string (startingOffset);
  data.append ("\n");
  data.append (convert_to_string (endingOffset));
  
  return data;
}
