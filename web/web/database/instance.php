<?php

require_once ("messages/messages.php");


class Database_Instance {

  public $is_connected;      // Indicates whether the database instance is connected.
  private $mysqli;           // Database object.
  private static $instance;  // Singleton object instance.

  private function __construct($display_errors) { 

    // File with credentials to access the database server.
    require_once ("database/credentials.php");
  
    $this->is_connected = true;

    // Connect to the database server.
    // This checks the MySQL server host, the user and password to connect to the database.
    @$this->mysqli = new mysqli($database_host, $database_user, $database_pass);
    if ($this->mysqli->connect_errno) {
      $this->is_connected = false;
      if ($display_errors) {
        message_warning ($this->mysqli->connect_error);
      }
    }

    // Connect to the database.
    if ($this->is_connected) {
      $this->is_connected = $this->mysqli->select_db ($database_name);
      if (!$this->is_connected) {
        if ($display_errors) {
          message_warning ("Could not select database $database_name. Are the credentials correct?");
        }
        // The database was not found. It needs to be created.
        // It should use utf8 unicode by default.
        $this->mysqli->query ("CREATE DATABASE `$database_name` DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;");
        if ($this->mysqli->errno) {
          $this->is_connected = false;
          if ($display_errors) {
            message_warning ($this->mysqli->error);
          }
        }    
        $this->is_connected = $this->mysqli->select_db ($database_name);
      }
    }

    // If there was no connection, give a short diagnostic message.      
    if (!$this->is_connected) {
      message_warning_header (gettext ("Cannot access the database"));
      message_information (gettext ("The site is not in operation at the moment."));
    }
  }

  public static function getInstance($display_errors = false) {
    if ( empty( self::$instance ) ) {
      self::$instance = new Database_Instance($display_errors);
    }
    return self::$instance;
  }


  public function runQuery ($query)
  {
    $result = $this->mysqli->query ($query);
    if ($result !== true) {
      $database_logs = Database_Logs::getInstance();
      $error = $this->mysqli->error;
      // Don't record empty errors.
      if ($error != "") {
        // Don't record errors generated by the log routine, since this could lead to an infinite loop.
        if (strpos ($error, "INSERT INTO logs VALUES") === false) {
          $error = stripslashes ($error);
          $error = stripslashes ($error);
          $error .= " - Query was: ";
          $error .= stripslashes ($query);
          $database_logs->log ($error);
        }
      }
    }
    return $result;
  }
  

}

?>
