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
page_access_level (Filter_Roles::translator ());


$header = new Assets_Header (gettext("Consistency"));
$header->run ();


$database_config_user = Database_Config_User::getInstance ();
$database_bibles = Database_Bibles::getInstance ();
$database_usfmresources = Database_UsfmResources::getInstance ();
$ipc_focus = Ipc_Focus::getInstance();


@$add = request->query ['add'];
if (isset ($add)) {
  if ($add == "") {
    $dialog_list = new Dialog_List2 (gettext("Would you like to add a Resource?"));
    $resources = Resource_Logic::getNames ();
    for ($resources as $resource) {
      $dialog_list->add_row ($resource, "&add=$resource");
    }
    $dialog_list->run();
  } else {
    $resources = request->database_config_user()->getConsistencyResources ();
    $resources [] = $add;
    $resources = array_unique ($resources, SORT_STRING);
    request->database_config_user()->setConsistencyResources ($resources);
  }
}


@$remove = request->query ['remove'];
if (isset ($remove)) {
  $resources = request->database_config_user()->getConsistencyResources ();
  $resources = filter_string_array_diff ($resources, array ($remove));
  $resources = array_values ($resources);
  request->database_config_user()->setConsistencyResources ($resources);
}


$view = new Assets_View (__FILE__);


$view->view->resources = request->database_config_user()->getConsistencyResources ();


$view->render ("index.php");
Assets_Page::footer ();
?>
