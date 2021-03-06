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


#ifndef INCLUDED_EDITOR_IMPORT_H
#define INCLUDED_EDITOR_IMPORT_H


#include <config/libraries.h>
#include <database/styles.h>
#include <pugixml/pugixml.hpp>


using namespace pugi;


class Editor_Usfm2Html
{
public:
  void load (string usfm);
  void stylesheet (string stylesheet);
  void quill ();
  void run ();
  string get ();
  size_t textLength;
  map <int, int> verseStartOffsets;
  string currentParagraphStyle;
private:
  vector <string> markersAndText; // Strings alternating between USFM and text.
  unsigned int markersAndTextPointer = 0;
  
  map <string, Database_Styles_Item> styles; // All the style information.
  
  // XML nodes.
  xml_document document;
  xml_node body_node;
  xml_node notes_node;
  
  // Standard content markers for notes.
  string standardContentMarkerFootEndNote;
  string standardContentMarkerCrossReference;
  
  xml_node currentPnode; // The current p node.
  bool current_p_open = false;
  string currentParagraphContent;
  vector <string> currentTextStyles;
  
  int noteCount = 0;
  xml_node notePnode; // The p DOM element of the current footnote, if any.
  bool note_p_open = false;
  vector <string> currentNoteTextStyles;
  
  // Whether note is open.
  bool noteOpened = false;
  
  // Lengths and offsets.
  bool quill_enabled = false;
  bool first_line_done = false;
  
  void preprocess ();
  void process ();
  void postprocess ();
  void outputAsIs (string marker, bool isOpeningMarker);
  void newParagraph (string style = "");
  void openTextStyle (Database_Styles_Item & style, bool embed);
  void closeTextStyle (bool embed);
  void addText (string text);
  void addNote (string citation, string style, bool endnote = false);
  void addNoteText (string text);
  void closeCurrentNote ();
  void addLink (xml_node domNode, string reference, string identifier, string style, string text);
  
  bool roadIsClear ();
};


#endif
