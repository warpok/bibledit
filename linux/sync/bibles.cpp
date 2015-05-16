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


#include <sync/bibles.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/merge.h>
#include <tasks/logic.h>
#include <config/logic.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/mail.h>
#include <database/modifications.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <checksum/logic.h>
#include <access/bible.h>
#include <bible/logic.h>


string sync_bibles_url ()
{
  return "sync/bibles";
}


bool sync_bibles_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string sync_bibles_receive_chapter (Webserver_Request * request, string & bible, int book, int chapter)
{
  string oldusfm = request->post ["o"];
  string newusfm = request->post ["n"];
  string checksum = request->post ["s"];

  
  string username = request->session_logic ()->currentUser ();
  string bookname = Database_Books::getEnglishFromId (book);
  
  
  Database_Logs::log ("Client sent Bible data: " + bible + " " + bookname + " " + convert_to_string (chapter), Filter_Roles::manager ());
  
  
  // Check whether the user has write-access to the Bible.
  if (!access_bible_write (request, bible, username)) {
    string message = "User " + username + " does not have write access to Bible " + bible;
    Database_Logs::log (message, Filter_Roles::manager ());
    return message;
  }
  
  
  // Check checksum.
  if (checksum != Checksum_Logic::get (oldusfm + newusfm)) {
    string message = "The received data is corrupted";
    Database_Logs::log (message, Filter_Roles::manager ());
    return message;
  }
  
  
  string serverusfm = request->database_bibles()->getChapter (bible, book, chapter);
  
  
  // Gather data for recording the changes made by the user, for the change notifications.
  int old_id = request->database_bibles()->getChapterId (bible, book, chapter);
  string old_text = serverusfm;
  string new_text = newusfm;
  
  
  if (serverusfm == "") {
    // If the chapter on the server is still empty, then just store the client's version on the server, and that's it.
    Bible_Logic::storeChapter (bible, book, chapter, newusfm);
  } else if (newusfm != serverusfm) {
    // Do a merge in case the client sends USFM that differs from what's on the server.
    string mergedusfm = filter_merge_run (oldusfm, newusfm, serverusfm);
    if (mergedusfm == serverusfm) {
      // When the merged USFM is the same as what's already on the server, then it means there was a merge conflict.
      // Email the user with the details, so the user can resolve the conflicts.
      string subject = "Problem sending chapter to server";
      string body = "<p>While sending " + bible + " " + bookname + " " + convert_to_string (chapter) + " to the server, the server didn't manage to merge it.</p>";
      body.append ("<p>Please re-enter your changes as you see fit.</p>");
      body.append ("<p>Here is the chapter you sent to the server:</p>");
      body.append ("<pre>" + newusfm + "</pre>");
      Database_Mail database_mail = Database_Mail (request);
      database_mail.send (username, subject, body);
    } else {
      // Update the server with the new chapter data.
      Bible_Logic::storeChapter (bible, book, chapter, mergedusfm);
    }
  }
  

  // If text was saved, record it as a change entered by the user.
  int new_id = request->database_bibles()->getChapterId (bible, book, chapter);
  if (new_id != old_id) {
    Database_Modifications database_modifications = Database_Modifications ();
    database_modifications.recordUserSave (username, bible, book, chapter, old_id, old_text, new_id, new_text);
  }
  
  
  // Send the updated chapter back to the client.
  // Do this only in case the updated chapter USFM is different from the new USFM the client sent.
  // This means that in most cases, nothing will be sent back.
  // That saves bandwidth.
  // And it allows the user on the client to continue editing
  // without the returned chapter overwriting the changes the user made.
  serverusfm = request->database_bibles()->getChapter (bible, book, chapter);
  if (serverusfm != newusfm) {
    string checksum = Checksum_Logic::get (serverusfm);
    return checksum + "\n" + serverusfm;
  }

  
  return "";
}


string sync_bibles (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Sync_Logic sync_logic = Sync_Logic (webserver_request);
  
  // Check on the credentials.
  if (!sync_logic.credentials_okay ()) return "";
  
  // Get the relevant parameters the client may have POSTed to us, the server.
  int action = convert_to_int (request->post ["a"]);
  string bible = request->post ["b"];
  int book = convert_to_int (request->post ["bk"]);
  int chapter = convert_to_int (request->post ["c"]);
  
  switch (action) {
    case Sync_Logic::bibles_get_total_checksum:
    {
      // The server reads the credentials from the client's user,
      // checks which Bibles this user has access to,
      // calculate the checksum of all chapters in those Bibles,
      // and returns this checksum to the client.
      string username = request->session_logic ()->currentUser ();
      vector <string> bibles = access_bible_bibles (request, username);
      string server_checksum = Checksum_Logic::getBibles (request, bibles);
      return server_checksum;
    }
    case Sync_Logic::bibles_get_bibles:
    {
      // The server reads the credentials from the client's user,
      // and responds with a list of Bibles this user has access to.
      string username = request->session_logic ()->currentUser ();
      vector <string> bibles = access_bible_bibles (request, username);
      string checksum = Checksum_Logic::get (bibles);
      string s_bibles = filter_string_implode (bibles, "\n");
      return checksum + "\n" + s_bibles;
    }
    case Sync_Logic::bibles_get_bible_checksum:
    {
      // The server responds with the checksum for the whole Bible.
      return Checksum_Logic::getBible (request, bible);
    }
    case Sync_Logic::bibles_get_books:
    {
      // The server responds with a checksum and then the list of books in the Bible.
      vector <int> server_books = request->database_bibles()->getBooks (bible);
      vector <string> v_server_books;
      for (auto book : server_books) v_server_books.push_back (convert_to_string (book));
      string s_server_books = filter_string_implode (v_server_books, "\n");
      string checksum = Checksum_Logic::get (v_server_books);
      return checksum + "\n" + s_server_books;
    }
    case Sync_Logic::bibles_get_book_checksum:
    {
      // The server responds with the checksum of the whole book.
      return Checksum_Logic::getBook (request, bible, book);
    }
    case Sync_Logic::bibles_get_chapters:
    {
      // The server responds with the list of books in the Bible book.
      vector <int> server_chapters = request->database_bibles()->getChapters (bible, book);
      vector <string> v_server_chapters;
      for (auto & chapter : server_chapters) v_server_chapters.push_back (convert_to_string (chapter));
      string s_server_chapters = filter_string_implode (v_server_chapters, "\n");
      string checksum = Checksum_Logic::get (v_server_chapters);
      return checksum + "\n" + s_server_chapters;
    }
    case Sync_Logic::bibles_get_chapter_checksum:
    {
      // The server responds with the checksum of the whole chapter.
      return Checksum_Logic::getChapter (request, bible, book, chapter);
    }
    case Sync_Logic::bibles_send_chapter:
    {
      return sync_bibles_receive_chapter (request, bible, book, chapter);
    }
    case Sync_Logic::bibles_get_chapter:
    {
      // The server responds with the USFM of the chapter, prefixed by a checksum.
      string usfm = filter_string_trim (request->database_bibles()->getChapter (bible, book, chapter));
      string checksum = Checksum_Logic::get (usfm);
      return checksum + "\n" + usfm;
    }
  }
  
  // Bad request.
  // Delay a while to obstruct a flood of bad requests.
  this_thread::sleep_for (chrono::seconds (1));
  request->response_code = 400;
  return "";
}