#ifndef OMEGADB_H
#define OMEGADB_H

#include <sqlite3.h>

#ifdef __cplusplus
extern "C" {
#endif

    /* connect / disconnect */
    int  db_connect(sqlite3 **db, const char *filename);
    void db_close(sqlite3 *db);

    /* create table and load test data */
    int db_init_table(sqlite3 *db);
    int db_fill_test_data(sqlite3 *db);

    /*
     * Select functions.
     * All return a malloc'd string — caller must free it with db_free_result().
     * Returns NULL on SQL error.
     */
    char *db_select_all(sqlite3 *db);
    char *db_select_by_group(sqlite3 *db, const char *group);
    char *db_select_by_name(sqlite3 *db, const char *name);
    char *db_select_by_surname(sqlite3 *db, const char *surname);
    char *db_select_by_name_and_surname(sqlite3 *db, const char *name, const char *surname);
    char *db_select_by_degree(sqlite3 *db, const char *degree);
    char *db_select_by_course(sqlite3 *db, const char *course);

    void db_free_result(char *str);

#ifdef __cplusplus
}
#endif

#endif /* OMEGADB_H */