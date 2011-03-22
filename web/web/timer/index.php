<?php
/**
* @package bibledit
*/
/*
 ** Copyright (©) 2003-2009 Teus Benschop.
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **  
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **  
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **  
 */


require_once ("../bootstrap/bootstrap.php");
// No check on page level access because everybody should be able to start this Poor Man's Crontab.


$crontable = Database_Cron::getInstance ();


// If the timer ran less than 15 minutes ago, we take it that it is still alive.
$watchdog = $crontable->getWatchdog ();
$time = time ();
if ($time < ($watchdog + 900)) die;



// Ok, we're going to run: Set the stage.
ignore_user_abort(true);
set_time_limit(0);
register_shutdown_function('shutdown');
$log = Database_Logs::getInstance();
$log->log ("Timer start");


// Run the loop.
while(1)
{
  $shutdown = $crontable->getShutdown ();
  if ($shutdown > 0) {
    $log->log ("Shutdown request: Timer stop");
    die;
  }  


  $crontable->setWatchdog ();


  $config_general = Database_Config_General::getInstance ();
  $current_timestamp = time ();


  $previous_timestamp = $config_general->getTimerMinute ();
  // Notice when the mysql database is down. 
  // Without this it will spawn many shell processes that effectively hang the system.
  $database_down = $previous_timestamp === "";
  if ($current_timestamp >= ($previous_timestamp + 60)) {
    $config_general->setTimerMinute ($current_timestamp);
    
    // Tasks to be done once a minute:
    
    // Mailer.
    $timer_mailer = new Timer_Mailer ();
    $timer_mailer->run ();
    unset ($timer_mailer);
    
  }


  $previous_timestamp = $config_general->getTimerFiveMinutes ();
  if ($current_timestamp >= ($previous_timestamp + 300)) {
    $config_general->setTimerFiveMinutes ($current_timestamp);
    
    // Tasks to be done each five minutes:

    $timer_receiver = new Timer_Receiver ();
    $timer_receiver->run ();
    unset ($timer_receiver);
    
  }


  $previous_timestamp = $config_general->getTimerDay ();
  if ($current_timestamp >= ($previous_timestamp + 86400)) {
    $config_general->setTimerDay ($current_timestamp);
    
    // Tasks to be done once a day:

    // Trim databases.
    $workingdirectory = dirname (__FILE__);
    if (!$database_down) shell_exec ("cd $workingdirectory; php trimdatabases.php > /dev/null 2>&1 &");

    // Log memory usage.    
    $memory_usage = memory_get_usage();
    $log->log ("Memory usage is $memory_usage bytes");
    
  }


  $backup_timestamp = $config_general->getTimerBackup ();
  if ($current_timestamp >= $backup_timestamp) {
    $backup_timestamp += 86400;
    while ($current_timestamp >= $backup_timestamp) {
      // This loop updates the backup timestamp to a value larger than the current time.
      // This avoids calling many backup processes when backups have not been made for some time.
      $backup_timestamp += 86400;
    }
    $config_general->setTimerBackup ($backup_timestamp);
    $workingdirectory = dirname (__FILE__);
    if (!$database_down) shell_exec ("cd $workingdirectory; php backup.php > /dev/null 2>&1 &");
  }
  unset ($backup_timestamp);


  $diff_timestamp = $config_general->getTimerDiff ();
  if ($current_timestamp >= $diff_timestamp) {
    $diff_timestamp += 86400;
    while ($current_timestamp >= $diff_timestamp) {
      // This loop updates the timestamp to a value larger than the current time.
      // This avoids calling many processes when the task has not been done for a while.
      $diff_timestamp += 86400;
    }
    $config_general->setTimerDiff ($diff_timestamp);
    $workingdirectory = escapeshellarg (dirname (__FILE__));
    if (!$database_down) shell_exec ("cd $workingdirectory; php changes.php > /dev/null 2>&1 &");
  }
  unset ($diff_timestamp);


  $exports_timestamp = $config_general->getTimerExports ();
  if ($current_timestamp >= $exports_timestamp) {
    $exports_timestamp += 86400;
    while ($current_timestamp >= $exports_timestamp) {
      // This loop updates the timestamp to a value larger than the current time.
      // This avoids calling many processes when the task has not been done for a while.
      $exports_timestamp += 86400;
    }
    $config_general->setTimerExports ($exports_timestamp);
    $workingdirectory = escapeshellarg (dirname (__FILE__));
    if (!$database_down) shell_exec ("cd $workingdirectory; php exports.php > /dev/null 2>&1 &");
  }
  unset ($exports_timestamp);


  // Wait a little for the next cycle.
  sleep(5);
}


function shutdown()
{
  $crontable = Database_Cron::getInstance ();
  $crontable->clearFlags ();
  $log = Database_Logs::getInstance();
  $log->log ("Timer shutdown");
}


?>
