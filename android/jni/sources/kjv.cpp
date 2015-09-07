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


#include <sources/kjv.h>
#include <database/logs.h>
#include <database/kjv.h>
#include <database/sqlite.h>
#include <filter/string.h>
#include <filter/url.h>
#include <libxml/xmlreader.h>


// Parses the XML data from kjv.xml.
// The parser is supposed to be ran only by the developers.
// No memory is freed: It leaks a lot of memory.
void sources_kjv_parse ()
{
  Database_Logs::log ("Parsing data from Crosswire's KJV XML file");
  Database_Kjv database_kjv;
  database_kjv.create ();

  int book = 0;
  int chapter = 0;
  int verse = 0;
  string lemma;
  string english;

  xmlTextReaderPtr reader = xmlNewTextReaderFilename ("sources/kjv.xml");
  while ((xmlTextReaderRead(reader) == 1)) {
    switch (xmlTextReaderNodeType (reader)) {
      case XML_READER_TYPE_ELEMENT:
      {
        string element = (char *) xmlTextReaderName (reader);
        if (element == "div") {
          string type = (char *) xmlTextReaderGetAttribute (reader, BAD_CAST "type");
          if (type == "book") {
            book++;
            Database_Logs::log ("Book " + convert_to_string (book));
            chapter = 0;
            verse = 0;
          }
        }
        if (element == "chapter") {
          chapter++;
          verse = 0;
        }
        if (element == "verse") {
          char * sID = (char *) xmlTextReaderGetAttribute (reader, BAD_CAST "sID");
          if (sID) {
            verse++;
          }
        }
        if (element == "w") {
          char * attr = (char *) xmlTextReaderGetAttribute (reader, BAD_CAST "lemma");
          if (attr) {
            lemma = filter_string_trim (attr);
          }
        }
        break;
      }
      case XML_READER_TYPE_TEXT:
      {
        string value = (char *) xmlTextReaderValue (reader);
        english = filter_string_trim (value);
        break;
      }
      case XML_READER_TYPE_END_ELEMENT:
      {
        string element = (char *) xmlTextReaderName (reader);
        if (element == "w") {
          vector <string> lemmas = filter_string_explode (lemma, ' ');
          for (auto strong : lemmas) {
            if (strong.find ("strong") == string::npos) continue;
            strong = filter_string_str_replace ("strong:", "", strong);
            database_kjv.store (book, chapter, verse, strong, english);
          }
          lemma.clear ();
          english.clear ();
        }
        break;
      }
    }
  }
  database_kjv.optimize ();
  Database_Logs::log ("Finished parsing data from the KJV XML file");
}