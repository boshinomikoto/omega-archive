#pragma once
#include <sqlite3.h>

#ifdef __cplusplus
extern "C" {
#endif

    int  auth_db_connect (sqlite3 **db, const char *filename);
    int  auth_db_init    (sqlite3 *db);
    int  auth_login      (sqlite3 *db, const char *login, const char *password); // 1=ok, 0=fail, -1=error
    int  auth_register   (sqlite3 *db, const char *login, const char *password); // SQLITE_OK или SQLITE_CONSTRAINT
    void auth_db_close   (sqlite3 *db);

#ifdef __cplusplus
}
#endif