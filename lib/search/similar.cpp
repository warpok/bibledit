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


#include <search/similar.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/volatile.h>
#include <database/config/bible.h>
#include <ipc/focus.h>


string search_similar_url ()
{
  return "search/similar";
}


bool search_similar_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string search_similar (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

 
  Database_Volatile database_volatile = Database_Volatile ();
  

  int myIdentifier = filter_string_user_identifier (request);
  
  
  string bible = request->database_config_user()->getBible ();
  if (request->query.count ("b")) {
    bible = request->query ["b"];
  }

  
  if (request->query.count ("load")) {
    int book = Ipc_Focus::getBook (request);
    int chapter = Ipc_Focus::getChapter (request);
    int verse = Ipc_Focus::getVerse (request);
    // Text of the focused verse in the active Bible.
    // Remove all punctuation from it.
    string versetext = request->database_search()->getBibleVerseText (bible, book, chapter, verse);
    vector <string> punctuation = filter_string_explode (Database_Config_Bible::getSentenceStructureEndPunctuation (bible), ' ');
    for (auto & sign : punctuation) {
      versetext = filter_string_str_replace (sign, "", versetext);
    }
    punctuation = filter_string_explode (Database_Config_Bible::getSentenceStructureMiddlePunctuation (bible), ' ');
    for (auto & sign : punctuation) {
      versetext = filter_string_str_replace (sign, "", versetext);
    }
    versetext = filter_string_trim (versetext);
    database_volatile.setValue (myIdentifier, "searchsimilar", versetext);
    return versetext;
  }
  
  
  if (request->query.count ("words")) {
    
    string words = request->query ["words"];
    words = filter_string_trim (words);
    database_volatile.setValue (myIdentifier, "searchsimilar", words);
    vector <string> vwords = filter_string_explode (words, ' ');
    
    // Include items if there are no more search hits than 30% of the total number of verses in the Bible.
    size_t maxcount = round (0.3 * request->database_search()->getVerseCount (bible));
    
    // Store how often a verse occurs in an array.
    // The keys are the identifiers of the search results.
    // The values are how often the identifiers occur in the entire focused verse.
    map <int, int> identifiers;
    
    for (auto & word : vwords) {
      
      // Find out how often this word occurs in the Bible. Skip if too often.
      vector <int> ids = request->database_search()->searchBibleText (bible, word);
      if (ids.size () > maxcount) continue;
      
      // Store the identifiers and their count.
      for (auto & id : ids) {
        if (identifiers.count (id)) identifiers [id]++;
        else identifiers [id] = 1;
      }
      
    }
    
    // Sort on occurence from high to low.
    // Skip identifiers that only occur once.
    vector <int> ids;
    vector <int> counts;
    for (auto & element : identifiers) {
      int id = element.first;
      int count = element.second;
      if (count <= 1) continue;
      ids.push_back (id);
      counts.push_back (count);
    }
    quick_sort (counts, ids, 0, counts.size());
    reverse (ids.begin(), ids.end());

    // Output the passage identifiers to the browser.
    string output;
    for (auto & id : ids) {
      if (!output.empty ()) output.append ("\n");
      output.append (convert_to_string (id));
    }
    return output;
  }
  
  
  if (request->query.count ("id")) {
    int id = convert_to_int (request->query ["id"]);
    
    // Get the Bible and passage for this identifier.
    Passage details = request->database_search()->getBiblePassage (id);
    string bible = details.bible;
    int book = details.book;
    int chapter = details.chapter;
    string verse = details.verse;
    
    // Get the plain text.
    string text = request->database_search()->getBibleVerseText (bible, book, chapter, convert_to_int (verse));
    
    // Get search words.
    vector <string> words = filter_string_explode (database_volatile.getValue (myIdentifier, "searchsimilar"), ' ');
    
    // Format it.
    string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
    text = filter_string_markup_words (words, text);
    string output = "<div>" + link + " " + text + "</div>";
    
    // Output to browser.
    return output;
  }

  
  string page;
  
  Assets_Header header = Assets_Header (gettext("Search"), request);
  page = header.run ();
  
  Assets_View view = Assets_View ();
  
  view.set_variable ("bible", bible);
  
  string script = "var searchBible = \"" + bible + "\";";
  view.set_variable ("script", script);

  page += view.render ("search", "similar");
  
  page += Assets_Page::footer ();
  
  return page;
}
