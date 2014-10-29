/*
Copyright (©) 2003-2014 Teus Benschop.

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


#include <database/sqlite.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <libgen.h>
#include <sys/stat.h>
#include <cstring>
#include <filter/url.h>
#include <filter/string.h>
#include <stdexcept>
#include <database/logs.h>


using namespace std;


/*
It has been seen on a shared hosting platform that the MySQL server did not have
sufficient available concurrent connections at times.
Other processes were using many connections, so that none remained for Bibledit-Web.
That is a reason against using MySQL on shared hosting.

A reason for using SQLite is that it is easier to set up.
No users, no privileges, no server.

Another reason is that a backup of the web space also back ups the SQLite databases,
provided they are stored in the web space.
With MySQL this is hard to do.

Usually with shared hosting, the web space is big, but the MySQL database space is small.
With a few Bibles, and several notes, plus resources, the MySQL database is full.
SQLite uses the huge web space that usually come with shared hosting.
Therefore it can store much more data with the same shared hosting package.

While MySQL is a faster database than SQLite in isolated experiments, 
on shared hosts it may be different.
The reasons is that on a shared host, SQLite gets the data in the same process 
straight from disk. This works differently for MySQL. In most cases, the shared
host uses a separate database server. Thus the web server fetches its data
from another host through the network. This introduces some delays.
For smaller data sets SQLite is much faster in this scenario.

Some often-used databases at times display database errors:
  disk I/O error
  unable to open database file
It was tried wiether the errors go away when "PRAGMA busy_timeout = 100;"
is executed after opening the database connection. The errors did not go away.

It was tried whether the above errors go away on shared hosting with this command:
PRAGMA journal_mode = TRUNCATE;
The errors did not go away.

It was tried whether the above errors go away on shared hosting with this command:
PRAGMA temp_store = MEMORY;
The errors did not go away.

It was tried whether the above errors go away on shared hosting with this command:
PRAGMA journal_mode = MEMORY;
The errors went away, but the database behaved in an inconsistent way.
It did not keeps its data properly, and did not update it properly.
The same behaviour was found with:
PRAGMA journal_mode = OFF;

It was tried whether setting the environment variable TMPDIR to a directory
in our own web space would improve SQLite, but this did not improve SQLite.
However, in other areas it looked as if this did give improvement.

What made a big difference is this:
1. Changing the database for the logbook from SQLite to the filesystem.
2. Setting the TMPDIR to a folder inside Bibledit's own webspace.
3. Letting the tasks runner not use a SQLite database, but the file system and memory.
The database errors went away.

*/


string database_sqlite_file (string database)
{
  return filter_url_create_root_path ("databases", database + ".sqlite");
}


sqlite3 * database_sqlite_connect (string database)
{
  sqlite3 *db;
  int rc;
  try {
    rc = sqlite3_open (database_sqlite_file (database).c_str(), &db);
    if (rc) {
      throw runtime_error (sqlite3_errmsg (db));
    }
    sqlite3_busy_timeout (db, 1000);
  } catch (exception & ex) {
    string message = "Database " + database + ": " + ex.what();
    Database_Logs::log (message);
    return NULL;
  }
  return db;
}


string database_sqlite_no_sql_injection (string sql)
{
  return filter_string_str_replace ("'", "''", sql);
}


void database_sqlite_exec (sqlite3 * db, string sql)
{
  int rc;
  try {
    char *error = NULL;
    rc = sqlite3_exec (db, sql.c_str(), NULL, NULL, &error);
    if (rc != SQLITE_OK) {
      throw runtime_error (error);
    }
  } catch (exception & ex) {
    string message = "SQL " + sql + ": " + ex.what();
    Database_Logs::log (message);
  }
}


map <string, vector <string> > database_sqlite_query (sqlite3 * db, string sql)
{
  int rc;
  SqliteReader reader (0);
  try {
    char *error = NULL;
    rc = sqlite3_exec (db, sql.c_str(), reader.callback, &reader, &error);
    if (rc != SQLITE_OK) {
      throw runtime_error (error);
    }
  } catch (exception & ex) {
    string message = "SQL " + sql + ": " + ex.what();
    Database_Logs::log (message);
  }
  return reader.result;
}


void database_sqlite_disconnect (sqlite3 * database)
{
  sqlite3_close (database);
}


// Does an integrity check on the database.
// Returns true if healthy, false otherwise.
bool database_sqlite_healthy (string database)
{
  string file = database_sqlite_file (database);
  bool ok = false;
  // Do an integrity check on the database.
  // An empty file appears healthy too, so deal with that.
  if (filter_url_filesize (file) > 0) {
    sqlite3 * db = database_sqlite_connect (database);
    string query = "PRAGMA integrity_check;";
    map <string, vector <string> > result = database_sqlite_query (db, query);
    vector <string> health = result ["integrity_check"];
    if (health.size () == 1) {
      if (health [0] == "ok") ok = true;
    }
    database_sqlite_disconnect (db);
  }
  return ok;
}


SqliteReader::SqliteReader (int dummy)
{
  if (dummy) {};
}


SqliteReader::~SqliteReader ()
{
}


int SqliteReader::callback (void *userdata, int argc, char **argv, char **column_names)
{
  for (int i = 0; i < argc; i++) {
    // Handle NULL field.
    if (argv [i] == NULL) ((SqliteReader *) userdata)->result [column_names [i]].push_back ("0");
    else ((SqliteReader *) userdata)->result [column_names [i]].push_back (argv[i]);
  }
  return 0;
}
