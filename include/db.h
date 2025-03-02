#ifndef DB_H
#define DB_H

#include <stdio.h>

#define MAX_QUERY_LENGTH 256

/* Default filename for saving/loading the database */
#define DB_FILE "database.db"


typedef struct Column
{
    char *name;
    char **data; /* Array of string data for each row */
} Column;

typedef struct Table
{
    char *name;
    int row_count;
    int column_count;
    Column **columns;
} Table;

typedef struct Database
{
    int table_count;
    Table **tables;
} Database;

/* Database Operations */
Database *create_db(void);
void free_database(Database *db);

/* Table Operations */
Table *find_table(Database *db, const char *table_name);
void create_table(Database *db, const char *table_name, const char *columns_str);
void insert_into_table(Database *db, const char *table_name, const char *values_str);
void select_from_table(Database *db, const char *table_name);

/* File Operations */
void save_database_to_file(Database *db, const char *filename);
Database *load_database_from_file(Database *db, const char *filename);

/* Query Parsing */
Database *parse_query(Database *db, const char *query);

/* Utility Functions */
int validate_ipv4_address(const char *ip);
char *trim_whitespace(char *str);

#endif /* DB_H */
