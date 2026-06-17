#include "omegadb.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* ── connection */

int db_connect(sqlite3 **db, const char *filename)
{
    return sqlite3_open(filename, db);
}

void db_close(sqlite3 *db)
{
    if (db) sqlite3_close(db);
}

/* ── table init */

int db_init_table(sqlite3 *db)
{
    const char *sql =
        "DROP TABLE IF EXISTS OmegaArchive;"
        "CREATE TABLE OmegaArchive ("
        "  id            INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  student_group TEXT,"
        "  degree        TEXT,"
        "  course        TEXT,"
        "  name          TEXT,"
        "  surname       TEXT,"
        "  phone         TEXT"
        ");";

    char *err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) { fprintf(stderr, "Init table error: %s\n", err); sqlite3_free(err); }
    return rc;
}

/* ── insert one student */

static int add_student(sqlite3 *db,
                       const char *group, const char *degree, const char *course,
                       const char *name,  const char *surname, const char *phone)
{
    const char *sql =
        "INSERT INTO OmegaArchive (student_group, degree, course, name, surname, phone) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rc;

    sqlite3_bind_text(stmt, 1, group,   -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, degree,  -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, course,  -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, name,    -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, surname, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, phone,   -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}

/* ── test data: 20 students ───────────────────────────────────── */

int db_fill_test_data(sqlite3 *db)
{
    // just a struct array, easier to read than 20 separate insert calls
    static const struct { const char *group, *degree, *course, *name, *surname, *phone; } s[] = {
        {"IT-101", "bachelor",  "bachelors_1course", "Oleksandr", "Kovalenko",   "+380501234001"},
        {"IT-101", "bachelor",  "bachelors_1course", "Iryna",     "Shevchenko",  "+380501234002"},
        {"IT-101", "bachelor",  "bachelors_1course", "Mykola",    "Bondarenko",  "+380501234003"},
        {"IT-101", "bachelor",  "bachelors_1course", "Oksana",    "Melnyk",      "+380501234004"},
        {"IT-102", "bachelor",  "bachelors_2course", "Vasyl",     "Savchenko",   "+380671234005"},
        {"IT-102", "bachelor",  "bachelors_2course", "Halyna",    "Kravchenko",  "+380671234006"},
        {"IT-102", "bachelor",  "bachelors_2course", "Taras",     "Petrenko",    "+380671234007"},
        {"IT-102", "bachelor",  "bachelors_2course", "Yulia",     "Ivanchenko",  "+380671234008"},
        {"IT-103", "bachelor",  "bachelors_3course", "Serhiy",    "Lysenko",     "+380931234009"},
        {"IT-103", "bachelor",  "bachelors_3course", "Natalia",   "Marchenko",   "+380931234010"},
        {"IT-103", "bachelor",  "bachelors_3course", "Andrii",    "Tkachenko",   "+380931234011"},
        {"IT-104", "bachelor",  "bachelors_4course", "Kateryna",  "Rudenko",     "+380661234012"},
        {"IT-104", "bachelor",  "bachelors_4course", "Bohdan",    "Yakovenko",   "+380661234013"},
        {"IT-104", "bachelor",  "bachelors_4course", "Daryna",    "Ponomarenko", "+380661234014"},
        {"IT-201", "master",    "masters_course1",   "Vladyslav", "Karpenko",    "+380731234015"},
        {"IT-201", "master",    "masters_course1",   "Viktoria",  "Hrytsenko",   "+380731234016"},
        {"IT-201", "master",    "masters_course1",   "Maksym",    "Savenko",     "+380731234017"},
        {"IT-202", "master",    "masters_course2",   "Olha",      "Tkach",       "+380991234018"},
        {"IT-202", "master",    "masters_course2",   "Roman",     "Klymenko",    "+380991234019"},
        {"IT-301", "graduates", "graduates",         "Anna",      "Moroz",       "+380951234020"},
    };

    int n = (int)(sizeof(s) / sizeof(s[0]));
    for (int i = 0; i < n; i++)
    {
        int rc = add_student(db, s[i].group, s[i].degree, s[i].course,
                                 s[i].name,  s[i].surname, s[i].phone);
        if (rc != SQLITE_OK) return rc;
    }
    return SQLITE_OK;
}

/* ── string buffer (grows as needed) ────────────────────────────────────── */

typedef struct { char *data; size_t len; size_t cap; } StrBuf;

static void sb_init(StrBuf *sb)
{
    sb->cap  = 2048;
    sb->len  = 0;
    sb->data = (char *)malloc(sb->cap);
    sb->data[0] = '\0';
}

static void sb_append(StrBuf *sb, const char *fmt, ...)
{
    va_list args, tmp;
    va_start(args, fmt);
    va_copy(tmp, args);
    int need = vsnprintf(NULL, 0, fmt, tmp);
    va_end(tmp);
    if (need < 0) { va_end(args); return; }

    if (sb->len + (size_t)need + 1 > sb->cap)
    {
        while (sb->len + (size_t)need + 1 > sb->cap) sb->cap *= 2;
        sb->data = (char *)realloc(sb->data, sb->cap);
    }
    vsnprintf(sb->data + sb->len, sb->cap - sb->len, fmt, args);
    sb->len += (size_t)need;
    va_end(args);
}

/* ── result table formatting */

static void sb_append_header(StrBuf *sb)
{
    sb_append(sb, "%-4s | %-8s | %-9s | %-18s | %-12s | %-14s | %s\n",
              "ID", "Group", "Degree", "Course", "Name", "Surname", "Phone");
    // separator line so the table looks readable
    sb_append(sb, "%.4s-+-%.8s-+-%.9s-+-%.18s-+-%.12s-+-%.14s-+-%.13s\n",
              "----", "--------", "---------", "------------------",
              "------------", "--------------", "-------------");
}

static void sb_append_row(StrBuf *sb, sqlite3_stmt *stmt)
{
    sb_append(sb, "%-4d | %-8s | %-9s | %-18s | %-12s | %-14s | %s\n",
              sqlite3_column_int (stmt, 0),
              sqlite3_column_text(stmt, 1),
              sqlite3_column_text(stmt, 2),
              sqlite3_column_text(stmt, 3),
              sqlite3_column_text(stmt, 4),
              sqlite3_column_text(stmt, 5),
              sqlite3_column_text(stmt, 6));
}

/* ── internal select helpers */

static char *run_select(sqlite3 *db, const char *sql, const char *value)
{
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Select error: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    if (value != NULL)
        sqlite3_bind_text(stmt, 1, value, -1, SQLITE_STATIC);

    StrBuf sb;
    sb_init(&sb);
    sb_append_header(&sb);

    while (sqlite3_step(stmt) == SQLITE_ROW)
        sb_append_row(&sb, stmt);

    sqlite3_finalize(stmt);
    return sb.data;
}

static char *run_select_two(sqlite3 *db, const char *sql,
                             const char *value1, const char *value2)
{
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Select error: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    if (value1 != NULL) sqlite3_bind_text(stmt, 1, value1, -1, SQLITE_STATIC);
    if (value2 != NULL) sqlite3_bind_text(stmt, 2, value2, -1, SQLITE_STATIC);

    StrBuf sb;
    sb_init(&sb);
    sb_append_header(&sb);

    while (sqlite3_step(stmt) == SQLITE_ROW)
        sb_append_row(&sb, stmt);

    sqlite3_finalize(stmt);
    return sb.data;
}

static char *select_by_field(sqlite3 *db, const char *field, const char *value)
{
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM OmegaArchive WHERE %s = ?", field);
    return run_select(db, sql, value);
}

/* ── public select API  */

void db_free_result(char *str) { free(str); }

char *db_select_all(sqlite3 *db)
{
    return run_select(db, "SELECT * FROM OmegaArchive", NULL);
}

char *db_select_by_group(sqlite3 *db, const char *group)
{
    return select_by_field(db, "student_group", group);
}

char *db_select_by_name(sqlite3 *db, const char *name)
{
    return select_by_field(db, "name", name);
}

char *db_select_by_surname(sqlite3 *db, const char *surname)
{
    return select_by_field(db, "surname", surname);
}

char *db_select_by_name_and_surname(sqlite3 *db, const char *name, const char *surname)
{
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT * FROM OmegaArchive WHERE name = ? AND surname = ?");
    return run_select_two(db, sql, name, surname);
}

char *db_select_by_degree(sqlite3 *db, const char *degree)
{
    return select_by_field(db, "degree", degree);
}

char *db_select_by_course(sqlite3 *db, const char *course)
{
    return select_by_field(db, "course", course);
}