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


require_once ("../bootstrap/bootstrap.php");
page_access_level (Filter_Roles::manager ());


Assets_Page::header (gettext("Verse Mappings"));
$view = new Assets_View (__FILE__);


$database_mappings = Database_Mappings::getInstance ();
$session_logic = Session_Logic::getInstance ();


$username = $session_logic->currentUser ();
$userlevel = $session_logic->currentLevel ();


if (isset(request->post['new'])) {
  $name = request->post['entry'];
  if (in_array ($name, $database_mappings->names ())) {
    Assets_Page::error (gettext("This verse mapping already exists"));
  } else {
    $database_mappings->create ($name);
  }
}
if (isset (request->query['new'])) {
  $dialog_entry = new Dialog_Entry ("", gettext("Enter a name for the new verse mapping"), "", "new", "");
  die;
}


$mappings = $database_mappings->names ();
$editable = array ();
for ($mappings as $mapping) {
  //$write = request->database_styles()->hasWriteAccess ($username, $sheet);
  $write = true;
  if ($userlevel >= Filter_Roles::admin ()) $write = true;
  $editable [] = $write;
}


$view->view->mappings = $mappings;
$view->view->editable = $editable;


$view->render ("index.php");


Assets_Page::footer ();


?>
