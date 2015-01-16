<?php
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


class Checks_Versification
{


  public static function books ($bible, $books)
  {
    $database_versifications = Database_Versifications::getInstance ();
    $standardBooks = $database_versifications->getBooks ("English");
    $absentBooks = filter_string_array_diff ($standardBooks, $books);
    $extraBooks = filter_string_array_diff ($books, $standardBooks);
    $database_check = Database_Check::getInstance ();
    for ($absentBooks as $book) {
      $database_check->recordOutput ($bible, $book, 1, 1, "This book is absent from the Bible");
    }
    for ($extraBooks as $book) {
      $database_check->recordOutput ($bible, $book, 1, 1, "This book is extra in the Bible");
    }
  }


  public static function chapters (bible, book, chapters)
  {
    $database_versifications = Database_Versifications::getInstance ();
    $standardChapters = $database_versifications->getChapters ("English", $book, true);
    $absentChapters = filter_string_array_diff ($standardChapters, $chapters);
    $extraChapters = filter_string_array_diff ($chapters, $standardChapters);
    $database_check = Database_Check::getInstance ();
    for ($absentChapters as $chapter) {
      $database_check->recordOutput (bible, book, chapter, 1, "This chapter is missing");
    }
    for ($extraChapters as $chapter) {
      $database_check->recordOutput (bible, book, chapter, 1, "This chapter is extra");
    }
  }


  public static function verses (bible, book, chapter, verses)
  {
    // Get verses in this chapter according to the versification system for the Bible.
    $database_versifications = Database_Versifications::getInstance ();
    $database_config_bible = Database_Config_Bible::getInstance ();
    $versification = Database_Config_Bible::getVersificationSystem ($bible);
    $standardVerses = $database_versifications->getVerses ($versification, $book, $chapter);
    // Look for missing and extra verses.
    $absentVerses = filter_string_array_diff ($standardVerses, $verses);
    $extraVerses = filter_string_array_diff ($verses, $standardVerses);
    $database_check = Database_Check::getInstance ();
    for ($absentVerses as $verse) {
      $database_check->recordOutput (bible, book, chapter, verse, "This verse is missing according to the versification system");
    }
    for ($extraVerses as $verse) {
      //if (($chapter == 0) && ($verse == 0)) continue;
      $database_check->recordOutput (bible, book, chapter, verse, "This verse is extra according to the versification system");
    }
    // Look for verses out of order.
    $previousVerse = 0;
    for ($verses as $key => $verse) {
      if ($key > 0) {
        if ($verse != ($previousVerse + 1)) {
          $database_check->recordOutput (bible, book, chapter, verse, "The verse is out of sequence");
        }
      }
      $previousVerse = $verse;
    }
  }


}


?>
