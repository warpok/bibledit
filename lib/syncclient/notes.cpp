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


#include <syncclient/notes.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/merge.h>
#include <filter/date.h>
#include <tasks/logic.h>
#include <config/logic.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/notes.h>
#include <database/noteactions.h>
#include <database/logs.h>
#include <database/state.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <sync/notes.h>
#include <checksum/logic.h>
#include <bible/logic.h>
#include <notes/logic.h>


int syncclient_notes_watchdog = 0;


string syncclient_notes_text ()
{
  return translate("Notes") + ": ";
}


string syncclient_notes_sendreceive_text ()
{
  return syncclient_notes_text () + translate ("Send/receive");
}


string syncclient_notes_up_to_date_text ()
{
  return syncclient_notes_text () + translate ("Up to date");
}


void syncclient_notes ()
{
  if (syncclient_notes_watchdog) {
    int time = filter_date_seconds_since_epoch ();
    if (time < (syncclient_notes_watchdog + 900)) {
      Database_Logs::log (syncclient_notes_text () + translate("Still busy"), Filter_Roles::translator ());
      return;
    }
    Database_Logs::log (syncclient_notes_text () + translate("Watchdog timeout"), Filter_Roles::translator ());
  }

  syncclient_notes_kick_watchdog ();

  bool success = syncclient_notes_upload ();
  

  // After all note actions have been sent to the server, and the notes updated on the client,
  // the client will now sync its notes with the server's notes.
  if (success) {
    success = syncclient_notes_download (Notes_Logic::lowNoteIdentifier, Notes_Logic::highNoteIdentifier);
  }

  if (success) Database_Logs::log (syncclient_notes_up_to_date_text (), Filter_Roles::translator ());

  syncclient_notes_watchdog = 0;
}


// Upload all note actions to the server.
bool syncclient_notes_upload ()
{
  Webserver_Request request;
  Sync_Logic sync_logic = Sync_Logic (&request);
  Database_Notes database_notes = Database_Notes (&request);
  Database_NoteActions database_noteactions = Database_NoteActions ();
  Notes_Logic notes_logic = Notes_Logic (&request);
  
  
  Database_Logs::log (syncclient_notes_sendreceive_text (), Filter_Roles::translator ());
  
  
  string response = client_logic_connection_setup ();
  int iresponse = convert_to_int (response);
  if (iresponse < Filter_Roles::guest () || iresponse > Filter_Roles::admin ()) {
    Database_Logs::log (syncclient_notes_text () + translate("Failure to initiate connection"), Filter_Roles::translator ());
    return false;
  }
  
  
  // Set the correct user in the session: The sole user on the Client.
  vector <string> users = request.database_users ()->getUsers ();
  if (users.empty ()) {
    Database_Logs::log (syncclient_notes_text () + translate("No local user found"), Filter_Roles::translator ());
    return false;
  }
  string user = users [0];
  request.session_logic ()->setUsername (user);
  
  
  // The basic request to be POSTed to the server.
  // It contains the user's credentials.
  map <string, string> post;
  post ["u"] = bin2hex (user);
  post ["p"] = request.database_users ()->getmd5 (user);
  post ["l"] = convert_to_string (request.database_users ()->getUserLevel (user));
  
  
  // Error variable.
  string error;
  
  
  // Server URL to call.
  string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  string url = client_logic_url (address, port, sync_notes_url ());
  
  
  // Check on communication errors to be careful that there will be no loss of notes data on the client.
  // While sending updates for a certai note identifier,
  // make sure to take the most recent update if there's more than one in the database for that identifier.
  
  
  // Go through all notes which have actions recorded for them.
  // While sending note actions, the database method to retrieve them
  // keeps the sequence of the actions as they occurred,
  // as later updates can undo or affect earlier updates.
  vector <int> notes = database_noteactions.getNotes ();
  for (auto identifier : notes) {
    

    string summary = database_notes.getSummary (identifier);
    if (summary.empty ()) summary = "<deleted>";
    Database_Logs::log (syncclient_notes_text () + translate("Sending note to server") + ": " + summary, Filter_Roles::translator ());
    
    
    // Get all the actions for the current note.
    vector <Database_Note_Action> note_actions = database_noteactions.getNoteData (identifier);
    
    
    // Due to some updates sent out here, record final actions to get the updated note from the server.
    map <int, bool> final_get_actions;
    
    
    // Deal with the note actions for this note.
    for (auto note_action : note_actions) {
      
      int rowid = note_action.rowid;
      string username = note_action.username;
      //int timestamp = note_action.timestamp;
      int action = note_action.action;
      string content = note_action.content;
      
      
      // Generate a POST request.
      post ["i"] = convert_to_string (identifier);
      post ["a"] = convert_to_string (action);
      switch (action) {
        case Sync_Logic::notes_put_create_initiate: break;
        case Sync_Logic::notes_put_create_complete: break;
        case Sync_Logic::notes_put_summary:
        {
          content = database_notes.getSummary (identifier);
          break;
        }
        case Sync_Logic::notes_put_contents: break;
        case Sync_Logic::notes_put_comment: break;
        case Sync_Logic::notes_put_subscribe: break;
        case Sync_Logic::notes_put_unsubscribe: break;
        case Sync_Logic::notes_put_assign: break;
        case Sync_Logic::notes_put_unassign: break;
        case Sync_Logic::notes_put_status:
        {
          content = database_notes.getRawStatus (identifier);
          break;
        }
        case Sync_Logic::notes_put_passages:
        {
          content = database_notes.getRawPassage (identifier);
          break;
        }
        case Sync_Logic::notes_put_severity:
        {
          content = convert_to_string (database_notes.getRawSeverity (identifier));
          break;
        }
        case Sync_Logic::notes_put_bible:
        {
          content = database_notes.getBible (identifier);
          break;
        }
        case Sync_Logic::notes_put_mark_delete: break;
        case Sync_Logic::notes_put_unmark_delete: break;
        case Sync_Logic::notes_put_delete: break;
      }
      post ["c"] = content;
      
      
      // Send the request off and receive the response.
      syncclient_notes_kick_watchdog ();
      response = sync_logic.post (post, url, error);
      if (!error.empty ()) {
        Database_Logs::log (syncclient_notes_text () + "Failure sending note: " + error, Filter_Roles::translator ());
        return false;
      }
      
      
      // Delete this note action because it has been dealt with.
      database_noteactions.erase (rowid);
      
      
      // Add final note actions depending on what was updated.
      switch (action) {
        case Sync_Logic::notes_put_create_initiate: break;
        case Sync_Logic::notes_put_create_complete:
        {
          int i = Sync_Logic::notes_get_contents;
          final_get_actions [i] = true;
          i = Sync_Logic::notes_get_subscribers;
          final_get_actions [i] = true;
          i = Sync_Logic::notes_get_assignees;
          final_get_actions [i] = true;
          i = Sync_Logic::notes_get_modified;
          final_get_actions [i] = true;
          break;
        }
        case Sync_Logic::notes_put_summary: break;
        case Sync_Logic::notes_put_contents: break;
        case Sync_Logic::notes_put_comment:
        {
          int i = Sync_Logic::notes_get_contents;
          final_get_actions [i] = true;
          i = Sync_Logic::notes_get_subscribers;
          final_get_actions [i] = true;
          i = Sync_Logic::notes_get_assignees;
          final_get_actions [i] = true;
          i = Sync_Logic::notes_get_modified;
          final_get_actions [i] = true;
          break;
        }
        case Sync_Logic::notes_put_subscribe: break;
        case Sync_Logic::notes_put_unsubscribe: break;
        case Sync_Logic::notes_put_assign: break;
        case Sync_Logic::notes_put_unassign: break;
        case Sync_Logic::notes_put_status: break;
        case Sync_Logic::notes_put_passages: break;
        case Sync_Logic::notes_put_severity: break;
        case Sync_Logic::notes_put_bible: break;
        case Sync_Logic::notes_put_mark_delete: break;
        case Sync_Logic::notes_put_unmark_delete: break;
        case Sync_Logic::notes_put_delete: break;
      }
    }
    
    
    // Deal with the extra, added, note actions.
    for (int action = Sync_Logic::notes_get_total; action <= Sync_Logic::notes_get_modified; action++) {
      map <string, string> post;
      post ["u"] = bin2hex (user);
      post ["i"] = convert_to_string (identifier);
      post ["a"] = convert_to_string (action);
      syncclient_notes_kick_watchdog ();
      response = sync_logic.post (post, url, error);
      if (error.empty ()) {
        if (action == Sync_Logic::notes_get_contents) {
          if (response != database_notes.getContents (identifier)) {
            database_notes.setContents (identifier, response);
          }
        }
        if (action == Sync_Logic::notes_get_subscribers) {
          vector <string> subscribers = filter_string_explode (response, '\n');
          database_notes.setSubscribers (identifier, subscribers);
        }
        if (action == Sync_Logic::notes_get_assignees) {
          vector <string> assignees = filter_string_explode (response, '\n');
          database_notes.setAssignees (identifier, assignees);
        }
        if (action == Sync_Logic::notes_get_modified) {
          database_notes.setModified (identifier, convert_to_int (response));
        }
      }
    }
  }
  
  return true;
}


// The function is called with the first and last note identifier to deal with.
bool syncclient_notes_download (int lowId, int highId)
{
  Webserver_Request request;
  Database_Notes database_notes = Database_Notes (&request);
  Database_NoteActions database_noteactions = Database_NoteActions ();
  Sync_Logic sync_logic = Sync_Logic (&request);
  Notes_Logic notes_logic = Notes_Logic (&request);
  
  
  string response = client_logic_connection_setup ();
  int iresponse = convert_to_int (response);
  if (iresponse < Filter_Roles::guest () || iresponse > Filter_Roles::admin ()) {
    Database_Logs::log (syncclient_notes_text () + translate("Failure to initiate connection"), Filter_Roles::translator ());
    return false;
  }
  
  
  // The client is supposed to have one user.
  // Get this username, plus the md5 hash of the password,
  // in preparation for sending it to the server in in the script below.
  // The server will use this user to find out the Bibles this user has access to,
  // so the server can select the correct notes for this user.
  // The client selects all available notes on the system.
  vector <string> users = request.database_users ()->getUsers ();
  if (users.empty ()) {
    Database_Logs::log (syncclient_notes_text () + translate("No local user found"), Filter_Roles::translator ());
    return false;
  }
  string user = users [0];
  request.session_logic ()->setUsername (user);
  string password = request.database_users ()->getmd5 (user);

  
  // Check for the health of the notes databases and take action if needed.
  bool healthy = true;
  if (!database_notes.healthy ()) healthy = false;
  if (!database_notes.checksums_healthy ()) healthy = false;
  if (!healthy) {
    Database_Logs::log (syncclient_notes_text () + "Abort receive just now because of database problems", Filter_Roles::translator ());
    return false;
  }
  
  
  // Server URL to call.
  string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  string url = client_logic_url (address, port, sync_notes_url ());
  
  
  // The basic request to be POSTed to the server.
  map <string, string> post;
  post ["u"] = bin2hex (user);
  post ["l"] = convert_to_string (lowId);
  post ["h"] = convert_to_string (highId);

  
  // Error variable.
  string error;
  
  
  // The script requests the total note count from the server, and their total checksum.
  // It compares this with the local notes.
  // If it matches, that means that the local notes match the notes on the server.
  // The script is then ready.
  post ["a"] = convert_to_string (Sync_Logic::notes_get_total);
  // Response comes after a relatively long delay: Enable burst mode timing.
  syncclient_notes_kick_watchdog ();
  response = sync_logic.post (post, url, error, true);
  if (!error.empty ()) {
    Database_Logs::log (syncclient_notes_text () + "Failure requesting totals: " + error, Filter_Roles::translator ());
    return false;
  }
  vector <string> vresponse = filter_string_explode (response, '\n');
  int server_total = 0;
  if (vresponse.size () >= 1) server_total = convert_to_int (vresponse [0]);
  string server_checksum;
  if (vresponse.size () >= 2) server_checksum = vresponse [1];
  vector <int> identifiers = database_notes.getNotesInRangeForBibles (lowId, highId, {}, true);
  int client_total = identifiers.size ();
  // Checksum cache to speed things up in case of thousands of notes.
  string client_checksum = Database_State::getNotesChecksum (lowId, highId);
  if (client_checksum.empty ()) {
    client_checksum = database_notes.getMultipleChecksum (identifiers);
    Database_State::putNotesChecksum (lowId, highId, client_checksum);
  }
  if (server_total == client_total) {
    if (server_checksum == client_checksum) {
      return true;
    }
  }
  
  
  // At this stage the total note count and/or their checksum on the client differs
  // from the total note count and/or their checksum on the server.
  // This applies to a certain range of notes for certain Bibles.


  // We know the total number of notes.
  // If the total note count is too high, divide the range of notes into smaller ranges,
  // and then deal with each range.
  if (server_total > 20) {
     vector <Sync_Logic_Range> ranges = sync_logic.create_range (lowId, highId);
    for (auto range : ranges) {
      syncclient_notes_download (range.low, range.high);
    }
    return true;
  }
  
  
  // At this stage the total note count is small enough that the function can
  // start to deal with the individual notes.
  // It requests the note identifiers, their checksums,
  // and compares them with the local notes.
  // If needed, it downloads the notes from the server.
  // If needed, the client removes local notes no longer available on the server.
  
  
  // Get note identifiers and checksums as on the server.
  post ["a"] = convert_to_string (Sync_Logic::notes_get_identifiers);
  syncclient_notes_kick_watchdog ();
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log (syncclient_notes_text () + "Failure requesting identifiers: " + error, Filter_Roles::translator ());
    return false;
  }
  vector <int> server_identifiers;
  vector <string> server_checksums;
  vresponse = filter_string_explode (response, '\n');
  for (size_t i = 0; i < vresponse.size (); i++) {
    if (i % 2 == 0) {
      int identifier = convert_to_int (vresponse [i]);
      if (identifier > 0) server_identifiers.push_back (identifier);
    }
    else server_checksums.push_back (vresponse [i]);
  }
  

  // Get note identifiers as locally on the client.
  vector <int> client_identifiers = database_notes.getNotesInRangeForBibles (lowId, highId, {}, true);
  
  
  // The client deletes notes no longer on the server.
  // But it skips the notes that have actions recorded for them,
  // as these notes are scheduled to be sent to the server first.
  identifiers = filter_string_array_diff (client_identifiers, server_identifiers);
  for (auto identifier : identifiers) {
    if (database_noteactions.exists (identifier)) continue;
    string summary = database_notes.getSummary (identifier);
    database_notes.erase (identifier);
    Database_Logs::log (syncclient_notes_text () + "Deleting because it is not on the server: " + summary, Filter_Roles::translator ());
  }
  

  // Check whether the local notes on the client match the ones on the server.
  // If needed download the note from the server.
  for (size_t i = 0; i < server_identifiers.size (); i++) {

    if (i >= server_checksums.size ()) continue;
    
    int identifier = server_identifiers [i];
    
    // Skip note if it is still to be sent off to the server.
    if (database_noteactions.exists (identifier)) continue;
    
    string server_checksum = server_checksums [i];
    string client_checksum = database_notes.getChecksum (identifier);
    if (client_checksum == server_checksum) continue;
    
    // If the note does not yet exist locally, create a dummy note and set the correct identifier.
    if (!database_notes.identifierExists (identifier)) {
      int id = database_notes.storeNewNote ("", 0, 0, 0, "dummy", "dummy", true);
      database_notes.setIdentifier (id, identifier);
    }

    // Fetch the note from the server.
    map <string, string> post;
    post ["i"] = convert_to_string (identifier);

    post ["a"] = convert_to_string (Sync_Logic::notes_get_summary);
    syncclient_notes_kick_watchdog ();
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (syncclient_notes_text () + "Failure requesting summary: " + error, Filter_Roles::translator ());
      return false;
    }
    string summary = response;
    if (summary != database_notes.getSummary (identifier)) {
      database_notes.setSummary (identifier, summary);
    }
    
    post ["a"] = convert_to_string (Sync_Logic::notes_get_contents);
    syncclient_notes_kick_watchdog ();
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (syncclient_notes_text () + "Failure requesting contents: " + error, Filter_Roles::translator ());
      return false;
    }
    if (response != database_notes.getContents (identifier)) {
      database_notes.setContents (identifier, response);
    }
    
    post ["a"] = convert_to_string (Sync_Logic::notes_get_subscribers);
    syncclient_notes_kick_watchdog ();
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (syncclient_notes_text () + "Failure requesting subscribers: " + error, Filter_Roles::translator ());
      return false;
    }
    vector <string> subscribers = filter_string_explode (response, '\n');
    database_notes.setSubscribers (identifier, subscribers);
    
    post ["a"] = convert_to_string (Sync_Logic::notes_get_assignees);
    syncclient_notes_kick_watchdog ();
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (syncclient_notes_text () + "Failure requesting assignees: " + error, Filter_Roles::translator ());
      return false;
    }
    vector <string> assignees = filter_string_explode (response, '\n');
    database_notes.setAssignees (identifier, assignees);
    
    post ["a"] = convert_to_string (Sync_Logic::notes_get_status);
    syncclient_notes_kick_watchdog ();
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (syncclient_notes_text () + "Failure requesting status: " + error, Filter_Roles::translator ());
      return false;
    }
    database_notes.setStatus (identifier, response);
    
    post ["a"] = convert_to_string (Sync_Logic::notes_get_passages);
    syncclient_notes_kick_watchdog ();
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (syncclient_notes_text () + "Failure requesting passages: " + error, Filter_Roles::translator ());
      return false;
    }
    // The server sent the raw passage contents, and store that on the client as well.
    // The reason for this is as follows:
    // There is a difference between Bibledit as written in PHP and Bibledit in C++,
    // with regard to whether the passages file has a new line at the end or not.
    // To ensure that the client passage file has exactly the same contents as on the server,
    // the contents should be passed in its raw form, without processing it.
    // If it were processed, then there will be a situation that the client keeps downloading
    // notes from the server, and never stops doing so, because the passage file contents
    // will always remain different. Raw passages transfer fix this.
    database_notes.setRawPassage (identifier, response);
    
    post ["a"] = convert_to_string (Sync_Logic::notes_get_severity);
    syncclient_notes_kick_watchdog ();
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (syncclient_notes_text () + "Failure requesting severity: " + error, Filter_Roles::translator ());
      return false;
    }
    database_notes.setRawSeverity (identifier, convert_to_int (response));
    
    post ["a"] = convert_to_string (Sync_Logic::notes_get_bible);
    syncclient_notes_kick_watchdog ();
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (syncclient_notes_text () + "Failure requesting bible: " + error, Filter_Roles::translator ());
      return false;
    }
    database_notes.setBible (identifier, response);
    
    // Set time modified field last as it is affected by the previous store actions.
    post ["a"] = convert_to_string (Sync_Logic::notes_get_modified);
    syncclient_notes_kick_watchdog ();
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (syncclient_notes_text () + "Failure requesting time modified: " + error, Filter_Roles::translator ());
      return false;
    }
    database_notes.setModified (identifier, convert_to_int (response));
    
    // Update search and checksum.
    database_notes.updateSearchFields (identifier);
    database_notes.updateChecksum (identifier);

    Database_Logs::log (syncclient_notes_text () + "Received from server: " + summary, Filter_Roles::manager ());

    database_notes.getChecksum (identifier);
  }
  
  return true;
}


void syncclient_notes_kick_watchdog ()
{
  syncclient_notes_watchdog = filter_date_seconds_since_epoch ();
}