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


#include <checks/space.h>
#include <filter/string.h>
#include <database/check.h>


void Checks_Space::doubleSpaceUsfm (string bible, int book, int chapter, int verse, string data)
{
  size_t pos = data.find ("  ");
  if (pos != string::npos) {
    int start = pos - 10;
    if (start < 0) start = 0;
    string fragment = data.substr (start, 20);
    Database_Check database_check;
    database_check.recordOutput (bible, book, chapter, verse, "Double space: ... " + fragment + " ...");
  }
}


void Checks_Space::spaceBeforePunctuation (string bible, int book, int chapter, map <int, string> texts)
{
  Database_Check database_check;
  for (auto element : texts) {
    int verse = element.first;
    string text = element.second;
    if (text.find (" ,") != string::npos) {
      database_check.recordOutput (bible, book, chapter, verse, "Space before a comma");
    }
    if (text.find (" ;") != string::npos) {
      database_check.recordOutput (bible, book, chapter, verse, "Space before a semicolon");
    }
    if (text.find (" :") != string::npos) {
      database_check.recordOutput (bible, book, chapter, verse, "Space before a colon");
    }
    if (text.find (" .") != string::npos) {
      database_check.recordOutput (bible, book, chapter, verse, "Space before a full stop");
    }
    if (text.find (" ?") != string::npos) {
      database_check.recordOutput (bible, book, chapter, verse, "Space before a question mark");
    }
    if (text.find (" !") != string::npos) {
      database_check.recordOutput (bible, book, chapter, verse, "Space before an exclamation mark");
    }
  }
}
