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


#include <resource/logic.h>
#include <webserver/request.h>
#include <access/bible.h>
#include <database/usfmresources.h>
#include <database/imageresources.h>
#include <database/mappings.h>
#include <database/config/bible.h>
#include <database/config/general.h>
#include <database/logs.h>
#include <database/cache.h>
#include <database/books.h>
#include <database/versifications.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <filter/shell.h>
#include <filter/roles.h>
#include <resource/external.h>
#include <locale/translate.h>
#include <client/logic.h>
#include <lexicon/logic.h>
#include <sword/logic.h>
#include <demo/logic.h>
#include <sync/resources.h>
#include <tasks/logic.h>
#include <related/logic.h>
#include <developer/logic.h>


/*

Stages to retrieve resource content and serve it.
 
 * fetch http: Fetch raw page from the internet through http.
 * fetch sword: Fetch a verse from a SWORD module.
 * fetch cloud: Fetch content from the dedicated or demo Bibledit Cloud.
 * check cache: Fetch http or sword content from the cache.
 * store cache: Store http or sword content in the cache.
 * extract: Extract the desired snipped from the larger page content.
 * postprocess: Postprocessing of the content to fine-tune it.
 * display: Display content to the user.
 * serve: Serve content to the client.
 
 A Bibledit client uses the following sequence to display a resource to the user:
 * check cache or fetch cloud -> store cache -> postprocess -> display.
 
 Bibledit Cloud uses the following sequence to display a resource to the user:
 * fetch http or fetch sword or check cache -> store cache -> extract -> postprocess - display.

 A Bibledit client uses the following sequence to install a resource:
 * check cache -> fetch cloud -> store cache.
 
 Bibledit Cloud uses the following sequence to serve a resource to a client:
 * fetch http or fetch sword or check cache -> store cache -> extract -> serve.
 
 In earlier versions of Bibledit,
 the client would download external resources straight from the Internet.
 To also be able to download content via https (secure http),
 the client needs the cURL library.
 And libcurl has not yet been compiled for all operating systems Bibledit runs on.
 In the current version of Bibledit, it works differently.
 It now requests all external content from Bibledit Cloud.
 An important advantage of this method is that this minimizes data transfer on the Bibledit Client.
 The client no longer fetches the full web page. Bibledit Cloud does that.
 And the Cloud then extracts the small relevant snipped from the web page,
 and serves that to the Client.
 
*/


vector <string> resource_logic_get_names (void * webserver_request)
{
  vector <string> names;
  
  // Bibles the user has read access to.
  vector <string> bibles = access_bible_bibles (webserver_request);
  names.insert (names.end(), bibles.begin (), bibles.end());
  
  // USFM resources.
  Database_UsfmResources database_usfmresources;
  vector <string> usfm_resources = database_usfmresources.getResources ();
  names.insert (names.end(), usfm_resources.begin(), usfm_resources.end());
  
  // External resources.
  vector <string> external_resources = resource_external_names ();
  names.insert (names.end (), external_resources.begin(), external_resources.end());
  
  // Image resources.
  Database_ImageResources database_imageresources;
  vector <string> image_resources = database_imageresources.names ();
  names.insert (names.end (), image_resources.begin(), image_resources.end());
  
  // Lexicon resources.
  vector <string> lexicon_resources = lexicon_logic_resource_names ();
  names.insert (names.end (), lexicon_resources.begin(), lexicon_resources.end());
  
  // SWORD resources
  vector <string> sword_resources = sword_logic_get_available ();
  names.insert (names.end (), sword_resources.begin(), sword_resources.end());
  
  sort (names.begin(), names.end());
  
  return names;
}


string resource_logic_get_html (void * webserver_request,
                                string resource, int book, int chapter, int verse,
                                bool add_verse_numbers)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  string html;
  
  Database_UsfmResources database_usfmresources;
  Database_ImageResources database_imageresources;
  Database_Mappings database_mappings;
  
  // Lists of the various types of resources.
  vector <string> bibles = request->database_bibles()->getBibles ();
  vector <string> usfms;
#ifdef HAVE_CLIENT
  usfms = client_logic_usfm_resources_get ();
  // As from February 2016 a client no longer automatically downloads USFM resources from the server.
  // A client still takes in account existing USFM resources it has downloaded before.
  vector <string> old_usfms = database_usfmresources.getResources ();
  usfms.insert (usfms.end (), old_usfms.begin (), old_usfms.end ());
#else
  usfms = database_usfmresources.getResources ();
#endif
  vector <string> externals = resource_external_names ();
  vector <string> images = database_imageresources.names ();
  vector <string> lexicons = lexicon_logic_resource_names ();

  // Possible SWORD details.
  string sword_module = sword_logic_get_remote_module (resource);
  string sword_source = sword_logic_get_source (resource);
  
  // Determine the type of the current resource.
  bool isBible = in_array (resource, bibles);
  bool isUsfm = in_array (resource, usfms);
  bool isExternal = in_array (resource, externals);
  bool isImage = in_array (resource, images);
  bool isLexicon = in_array (resource, lexicons);
  bool isSword = (!sword_source.empty () && !sword_module.empty ());

  // Retrieve versification system of the active Bible.
  string bible = request->database_config_user ()->getBible ();
  string bible_versification = Database_Config_Bible::getVersificationSystem (bible);

  // Determine the versification system of the current resource.
  string resource_versification;
  if (isBible || isUsfm) {
    resource_versification = Database_Config_Bible::getVersificationSystem (bible);
  } else if (isExternal) {
    resource_versification = resource_external_mapping (resource);
  } else if (isImage) {
  } else if (isLexicon) {
    resource_versification = database_mappings.original ();
    if (resource == KJV_LEXICON_NAME) resource_versification = english ();
  } else if (isSword) {
    resource_versification = english ();
  } else {
  }

  // If the resource versification system differs from the Bible's versification system,
  // map the focused verse of the Bible to a verse in the Resource.
  // There are resources without versification system: Do nothing about them.
  vector <Passage> passages;
  if ((bible_versification != resource_versification) && !resource_versification.empty ()) {
    passages = database_mappings.translate (bible_versification, resource_versification, book, chapter, verse);
  } else {
    passages.push_back (Passage ("", book, chapter, convert_to_string (verse)));
  }

  // If there's been a mapping, the resource should include the verse number for clarity.
  if (passages.size () != 1) add_verse_numbers = true;
  for (auto passage : passages) {
    if (verse != convert_to_int (passage.verse)) {
      add_verse_numbers = true;
    }
  }
  
  // Flag for whether to add the full passage (e.g. Matthew 1:1) to the text of that passage.
  bool add_passages_in_full = false;

  // Deal with user's preference whether to include related passages.
  if (request->database_config_user ()->getIncludeRelatedPassages ()) {
    
    // Take the Bible's active passage and mapping, and translate that to the original mapping.
    vector <Passage> related_passages = database_mappings.translate (bible_versification, database_mappings.original (), book, chapter, verse);
    
    // Look for related passages.
    related_passages = related_logic_get_verses (related_passages);
    
    add_passages_in_full = !related_passages.empty ();
    
    // If there's any related passages, map them to the resource's versification system.
    if (!related_passages.empty ()) {
      if (!resource_versification.empty ()) {
        if (resource_versification != database_mappings.original ()) {
          passages.clear ();
          for (auto & related_passage : related_passages) {
            vector <Passage> mapped_passages = database_mappings.translate (database_mappings.original (), resource_versification, related_passage.book, related_passage.chapter, convert_to_int (related_passage.verse));
            passages.insert (passages.end (), mapped_passages.begin (), mapped_passages.end ());
          }
        }
      }
    }
  }
  
  for (auto passage : passages) {
    string possible_included_passage;
    if (add_verse_numbers) possible_included_passage = passage.verse + " ";
    if (add_passages_in_full) possible_included_passage = filter_passage_display (passage.book, passage.chapter, passage.verse) + " ";
    if (isImage) possible_included_passage.clear ();
    html.append (possible_included_passage);
    html.append (resource_logic_get_verse (webserver_request, resource, passage.book, passage.chapter, convert_to_int (passage.verse)));
  }
  
  return html;
}


// This is the most basic version that fetches the text of a $resource.
// It works on server and on client.
// It uses the cache.
string resource_logic_get_verse (void * webserver_request, string resource, int book, int chapter, int verse)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  string data;

  Database_UsfmResources database_usfmresources;
  Database_ImageResources database_imageresources;
  
  // Lists of the various types of resources.
  // As from February 2016 a client no longer automatically downloads USFM resources from the Cloud.
  // But a client takes in account existing USFM resources it has downloaded before.
  vector <string> bibles = request->database_bibles()->getBibles ();
  vector <string> local_usfms = database_usfmresources.getResources ();
  vector <string> remote_usfms;
#ifdef HAVE_CLIENT
  remote_usfms = client_logic_usfm_resources_get ();
#endif
  vector <string> externals = resource_external_names ();
  vector <string> images = database_imageresources.names ();
  vector <string> lexicons = lexicon_logic_resource_names ();
  
  // Possible SWORD details.
  string sword_module = sword_logic_get_remote_module (resource);
  string sword_source = sword_logic_get_source (resource);
  
  // Determine the type of the current resource.
  bool isBible = in_array (resource, bibles);
  bool isLocalUsfm = in_array (resource, local_usfms);
  bool isRemoteUsfm = in_array (resource, remote_usfms);
  bool isExternal = in_array (resource, externals);
  bool isImage = in_array (resource, images);
  bool isLexicon = in_array (resource, lexicons);
  bool isSword = (!sword_source.empty () && !sword_module.empty ());
  
  if (isBible || isLocalUsfm) {
    string chapter_usfm;
    if (isBible) chapter_usfm = request->database_bibles()->getChapter (resource, book, chapter);
    if (isLocalUsfm) chapter_usfm = database_usfmresources.getUsfm (resource, book, chapter);
    string verse_usfm = usfm_get_verse_text (chapter_usfm, verse);
    string stylesheet = styles_logic_standard_sheet ();
    Filter_Text filter_text = Filter_Text (resource);
    filter_text.html_text_standard = new Html_Text ("");
    filter_text.addUsfmCode (verse_usfm);
    filter_text.run (stylesheet);
    data = filter_text.html_text_standard->getInnerHtml ();
  } else if (isRemoteUsfm) {
    data = resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
  } else if (isExternal) {
#ifdef HAVE_CLIENT
    // A client fetches it from the cache or from the Cloud.
    data = resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
#else
    // The server fetches it from the web, via the http cache.
    data.append (resource_external_cloud_fetch_cache_extract (resource, book, chapter, verse));
#endif
    
  } else if (isImage) {
    vector <string> images = database_imageresources.get (resource, book, chapter, verse);
    for (auto & image : images) {
      data.append ("<div><img src=\"/resource/imagefetch?name=" + resource + "&image=" + image + "\" alt=\"Image resource\" style=\"width:100%\"></div>");
    }
  } else if (isLexicon) {
    data = lexicon_logic_get_html (request, resource, book, chapter, verse);
  } else if (isSword) {
    data = sword_logic_get_text (sword_source, sword_module, book, chapter, verse);
  } else {
    // Nothing found.
  }
  
  // Any font size given in a paragraph style may interfere with the font size setting for the resources
  // as given in Bibledit. For that reason remove the class name from a paragraph style.
  for (unsigned int i = 0; i < 5; i++) {
    string fragment = "p class=\"";
    size_t pos = data.find (fragment);
    if (pos != string::npos) {
      size_t pos2 = data.find ("\"", pos + fragment.length () + 1);
      if (pos2 != string::npos) {
        data.erase (pos + 1, pos2 - pos);
      }
    }
  }
  
  // NET Bible updates.
  data = filter_string_str_replace ("<span class=\"s ", "<span class=\"", data);

  return data;
}


// This runs on the server.
// It gets the html or text contents for a $resource for serving it to a client.
string resource_logic_get_contents_for_client (string resource, int book, int chapter, int verse)
{
  // Lists of the various types of resources.
  Database_UsfmResources database_usfmresources;
  vector <string> externals = resource_external_names ();
  vector <string> usfms = database_usfmresources.getResources ();
  
  // Possible SWORD details in case the client requests a SWORD resource.
  string sword_module = sword_logic_get_remote_module (resource);
  string sword_source = sword_logic_get_source (resource);
  
  // Determine the type of the current resource.
  bool isExternal = in_array (resource, externals);
  bool isUsfm = in_array (resource, usfms);
  bool isSword = (!sword_source.empty () && !sword_module.empty ());
  
  if (isExternal) {
    // The server fetches it from the web.
    return resource_external_cloud_fetch_cache_extract (resource, book, chapter, verse);
  }
  
  if (isUsfm) {
    // Fetch from database and convert to html.
    string chapter_usfm = database_usfmresources.getUsfm (resource, book, chapter);
    string verse_usfm = usfm_get_verse_text (chapter_usfm, verse);
    string stylesheet = styles_logic_standard_sheet ();
    Filter_Text filter_text = Filter_Text (resource);
    filter_text.html_text_standard = new Html_Text ("");
    filter_text.addUsfmCode (verse_usfm);
    filter_text.run (stylesheet);
    return filter_text.html_text_standard->getInnerHtml ();
  }
  
  if (isSword) {
    // Fetch it from a SWORD module.
    return sword_logic_get_text (sword_source, sword_module, book, chapter, verse);
  }

  // Nothing found.
  return "Bibledit Cloud could not localize this resource";
}


// The client runs this function to fetch a general resource $name from the Cloud,
// or from its local cache,
// and to update the local cache with the fetched content, if needed,
// and to return the requested content.
string resource_logic_client_fetch_cache_from_cloud (string resource, int book, int chapter, int verse)
{
  // Ensure that the cache for this resource exists on the client.
  if (!Database_Cache::exists (resource, book)) {
    Database_Cache::create (resource, book);
  }
  
  // If the content exists in the cache, return that content.
  if (Database_Cache::exists (resource, book, chapter, verse)) {
    return Database_Cache::retrieve (resource, book, chapter, verse);
  }
  
  // Fetch this resource from Bibledit Cloud or from the cache.
  string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  if (!client_logic_client_enabled ()) {
    // If the client has not been connected to a cloud instance,
    // fetch the resource from the Bibledit Cloud demo.
    address = demo_address ();
    port = demo_port ();
  }
  
  string url = client_logic_url (address, port, sync_resources_url ());
  url = filter_url_build_http_query (url, "r", filter_url_urlencode (resource));
  url = filter_url_build_http_query (url, "b", convert_to_string (book));
  url = filter_url_build_http_query (url, "c", convert_to_string (chapter));
  url = filter_url_build_http_query (url, "v", convert_to_string (verse));
  string error;
  string content = filter_url_http_get (url, error, false);
  
  if (error.empty ()) {
    // No error: Cache content.
    Database_Cache::cache (resource, book, chapter, verse, content);
  } else {
    // Error: Log it, and return it.
    Database_Logs::log (resource + ": " + error);
    content.append (error);
  }

  // Done.
  return content;
}


// Imports the file at $path into $resource.
void resource_logic_import_images (string resource, string path)
{
  Database_ImageResources database_imageresources;
  
  Database_Logs::log ("Importing: " + filter_url_basename (path));
  
  // To begin with, add the path to the main file to the list of paths to be processed.
  vector <string> paths = {path};
  
  while (!paths.empty ()) {
  
    // Take and remove the first path from the container.
    path = paths[0];
    paths.erase (paths.begin());
    string basename = filter_url_basename (path);
    string extension = filter_url_get_extension (path);
    extension = unicode_string_casefold (extension);

    if (extension == "pdf") {
      
      Database_Logs::log ("Processing PDF: " + basename);
      
      // Retrieve PDF information.
      filter_shell_run ("", "pdfinfo", {path}, NULL, NULL);

      // Convert the PDF file to separate images.
      string folder = filter_url_tempfile ();
      filter_url_mkdir (folder);
      filter_shell_run (folder, "pdftocairo", {"-jpeg", path}, NULL, NULL);
      // Add the images to the ones to be processed.
      filter_url_recursive_scandir (folder, paths);
      
    } else if (filter_archive_is_archive (path)) {
      
      Database_Logs::log ("Unpacking archive: " + basename);
      string folder = filter_archive_uncompress (path);
      filter_url_recursive_scandir (folder, paths);
      
    } else {

      if (!extension.empty ()) {
        basename = database_imageresources.store (resource, path);
        Database_Logs::log ("Storing image " + basename );
      }
      
    }
  }

  Database_Logs::log ("Ready importing images");
}


string resource_logic_yellow_divider ()
{
  return "Yellow Divider";
}


string resource_logic_green_divider ()
{
  return "Green Divider";
}


string resource_logic_blue_divider ()
{
  return "Blue Divider";
}


string resource_logic_violet_divider ()
{
  return "Violet Divider";
}


string resource_logic_red_divider ()
{
  return "Red Divider";
}


string resource_logic_orange_divider ()
{
  return "Orange Divider";
}


bool resource_logic_is_divider (string resource)
{
  if (resource == resource_logic_yellow_divider ()) return true;
  if (resource == resource_logic_green_divider ()) return true;
  if (resource == resource_logic_blue_divider ()) return true;
  if (resource == resource_logic_violet_divider ()) return true;
  if (resource == resource_logic_red_divider ()) return true;
  if (resource == resource_logic_orange_divider ()) return true;
  return false;
}


string resource_logic_get_divider (string resource)
{
  vector <string> bits = filter_string_explode (resource, ' ');
  string colour = unicode_string_casefold (bits [0]);
  // The $ influences the resource's embedding through javascript.
  string html = "$<div class=\"width100\" style=\"background-color: " + colour + ";\">&nbsp;</div>";
  return html;
}


// In Cloud mode, this function wraps around http GET.
// It fetches existing content from the cache, and caches new content.
string resource_logic_web_cache_get (string url, string & error)
{
#ifndef HAVE_CLIENT
  // On the Cloud, check if the URL is in the cache.
  if (database_filebased_cache_exists (url)) {
    return database_filebased_cache_get (url);
  }
#endif
  // Fetch the URL from the network.
  // Do not cache the response in an error situation.
  error.clear ();
  string html = filter_url_http_get (url, error, false);
  if (!error.empty ()) {
    return html;
  }
#ifndef HAVE_CLIENT
  // In the Cloud, cache the response.
  database_filebased_cache_put (url, html);
#endif
  // Done.
  return html;
}


// Returns the page type for the resource selector.
string resource_logic_selector_page (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string page = request->query["page"];
  return page;
}


// Returns the page which called the resource selector.
string resource_logic_selector_caller (void * webserver_request)
{
  string caller = resource_logic_selector_page (webserver_request);
  if (caller == "view") caller = "organize";
  if (caller == "consistency") caller = "../consistency/index";
  if (caller == "print") caller = "print";
  return caller;
}


string resource_logic_default_user_url ()
{
  return "http://bibledit.org/resource-[book]-[chapter]-[verse].html";
}


// This creates a resource database cache and runs in the Cloud.
void resource_logic_create_cache ()
{
  // Because clients usually request caches in a quick sequence,
  // the Cloud starts to cache several books in parallel.
  // Here's some logic to ensure there's only one book cached at a time.
  static bool resource_logic_create_cache_running = false;
  if (resource_logic_create_cache_running) return;
  resource_logic_create_cache_running = true;
  
  // If there's nothing to cache, bail out.
  vector <string> signatures = Database_Config_General::getResourcesToCache ();
  if (signatures.empty ()) return;

  // Resource and book to cache.
  string signature = signatures [0];
  signatures.erase (signatures.begin ());
  Database_Config_General::setResourcesToCache (signatures);
  size_t pos = signature.find_last_of (" ");
  string resource = signature.substr (0, pos);
  int book = convert_to_int (signature.substr (pos++));
  string bookname = Database_Books::getEnglishFromId (book);

  // Whether it's a SWORD module.
  string sword_module = sword_logic_get_remote_module (resource);
  string sword_source = sword_logic_get_source (resource);
  bool is_sword_module = (!sword_source.empty () && !sword_module.empty ());

  // In case of a  SWORD module, ensure it has been installed.
  if (is_sword_module) {
    vector <string> modules = sword_logic_get_installed ();
    bool already_installed = false;
    for (auto & installed_module : modules) {
      if (installed_module.find ("[" + sword_module + "]") != string::npos) {
        already_installed = true;
      }
    }
    if (!already_installed) {
      sword_logic_install_module (sword_source, sword_module);
    }
  }
  
  // Database layout is per book: Create a database for this book.
  Database_Cache::remove (resource, book);
  Database_Cache::create (resource, book);
  
  Database_Versifications database_versifications;
  vector <int> chapters = database_versifications.getMaximumChapters (book);
  for (auto & chapter : chapters) {

    Database_Logs::log ("Caching " + resource + " " + bookname + " " + convert_to_string (chapter), Filter_Roles::consultant ());

    // The verse numbers in the chapter.
    vector <int> verses = database_versifications.getMaximumVerses (book, chapter);
    
    // In case of a SWORD module, fetch the texts of all verses in bulk.
    // This is because calling vfork once per verse to run diatheke stops working in the Cloud after some time.
    // Forking once per chapter is much better, also for the performance.
    map <int, string> sword_texts;
    if (is_sword_module) {
      sword_texts = sword_logic_get_bulk_text (sword_module, book, chapter, verses);
    }
    
    // Iterate over the verses.
    for (auto & verse : verses) {

      // Fetch the text for the passage.
      bool server_is_installing_module = false;
      int wait_iterations = 0;
      string html, error;
      do {
        // Fetch the resource data.
        if (is_sword_module) html = sword_texts [verse];
        else html = resource_logic_get_contents_for_client (resource, book, chapter, verse);
        // Check on errors.
        server_is_installing_module = (html == sword_logic_installing_module_text ());
        if (server_is_installing_module) {
          Database_Logs::log ("Waiting while installing SWORD module: " + resource);
          this_thread::sleep_for (chrono::seconds (60));
          wait_iterations++;
        }
      } while (server_is_installing_module && (wait_iterations < 5));

      // Cache the verse data, even if there's an error.
      // If it were not cached at, say, Leviticus, then the caching mechanism,
      // after restart, would always continue from that same book, from Leviticus,
      // and never finish. Therefore something should be cached, even if it's an empty string.
      if (server_is_installing_module) html.clear ();
      Database_Cache::cache (resource, book, chapter, verse, html);
    }
  }

  // Done.
  Database_Cache::ready (resource, book, true);
  Database_Logs::log ("Completed caching " + resource + " " + bookname, Filter_Roles::consultant ());
  resource_logic_create_cache_running = false;
  
  // If there's another resource database waiting to be cached, schedule it for caching.
  if (!signatures.empty ()) tasks_logic_queue (CACHERESOURCES);
}
