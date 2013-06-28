<?php


require_once ("../bootstrap/bootstrap.php");
page_access_level (MANAGER_LEVEL);

Assets_Page::header (gettext ("Styles"));
$view = new Assets_View (__FILE__);
$database_styles = Database_Styles::getInstance();


@$delete = $_GET['delete'];
if ($delete != "") {
  @$confirm = $_GET['confirm'];
  if ($confirm != "") {
    $database_styles->deleteSheet ($delete);
  } else {
    $dialog_yes = new Dialog_Yes (NULL, gettext ("Would you like to delete this stylesheet?"), "delete");
    die;
  }
}


if (isset($_POST['new'])) {
  $name = $_POST['entry'];
  $database_styles->createSheet ($name);
}


if (isset ($_GET['new'])) {
  $dialog_entry = new Dialog_Entry ("", gettext ("Please enter the name for the new stylesheet"), "", "new", "");
  die;
}


$sheets = $database_styles->getSheets();
$view->view->sheets = $sheets;
$view->render ("indexm.php");


Assets_Page::footer ();
?>
