#ifndef _DB_H_
#define _DB_H_

#define MAX_TABLES 10
#define MAX_COLUMNS 10
#define MAX_ROWS 100
#define MAX_QUERY_LENGTH 256
#define MAX_NAME_LENGTH 50
#define DB_FILE "database.db"

typedef struct
{
    char name[MAX_NAME_LENGTH];
    char data[MAX_ROWS][MAX_NAME_LENGTH];
} Column;

typedef struct
{
    char name[MAX_NAME_LENGTH];
    Column columns[MAX_COLUMNS];
    int column_count;
    int row_count;
} Table;

typedef struct
{
    Table tables[MAX_TABLES];
    int table_count;
} Database;

Database *create_db(void);
void create_table(Database *db, const char *table_name, char *columns);
void insert_into_table(Database *db, const char *table_name, char *values);
void select_from_table(Database *db, const char *table_name);
void parse_query(Database *db, const char *query);
Table *find_table(Database *db, const char *table_name);
void save_database_to_file(Database *db, const char *filename);
void load_database_from_file(Database *db, const char *filename);
int validate_ipv4_address(const char *ip);
void trim_whitespace(char *str);

#endif