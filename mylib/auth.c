#include "auth.h"
#include <stdio.h>

int auth_db_connect(sqlite3 **db, const char *filename)
{
    return sqlite3_open(filename, db);
}

int auth_db_init(sqlite3 *db)
{
    const char *sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "  id       INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  login    TEXT NOT NULL UNIQUE,"
        "  password TEXT NOT NULL"
        ");";
    char *errmsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) { fprintf(stderr, "auth_db_init: %s\n", errmsg); sqlite3_free(errmsg); }
    return rc;
}

int auth_login(sqlite3 *db, const char *login, const char *password)
{
    const char *sql = "SELECT COUNT(*) FROM users WHERE login = ? AND password = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;

    sqlite3_bind_text(stmt, 1, login,    -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        found = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return found;
}

int auth_register(sqlite3 *db, const char *login, const char *password)
{
    const char *sql = "INSERT INTO users (login, password) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return sqlite3_errcode(db);

    sqlite3_bind_text(stmt, 1, login,    -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}

void auth_db_close(sqlite3 *db)
{
    sqlite3_close(db);
}