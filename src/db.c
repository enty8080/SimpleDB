#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "db.h"

/* Writes a string to a file.
 * The string is preceded by its length (including the null terminator).
 */
static void write_string(FILE *file, const char *str)
{
    int len;
    len = (int)strlen(str) + 1;
    fwrite(&len, sizeof(int), 1, file);
    fwrite(str, sizeof(char), len, file);
}

/* Reads a string from a file that was written using write_string.
 * Returns a pointer to the heap-allocated string, or NULL on failure.
 */
static char *read_string(FILE *file)
{
    int len;
    char *str;

    if (fread(&len, sizeof(int), 1, file) != 1)
    {
        return NULL;
    }

    str = malloc(len);
    if (str == NULL)
    {
        return NULL;
    }

    fread(str, sizeof(char), len, file);
    return str;
}

/* Trims leading and trailing whitespace from a string in place.
 * Returns a pointer to the trimmed string.
 */
char *trim_whitespace(char *str)
{
    char *end;

    /* Trim leading whitespace */
    while (isspace((unsigned char)*str))
    {
        str++;
    }

    if (*str == '\0')
    {
        return str;
    }

    /* Trim trailing whitespace */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
    {
        end--;
    }

    /* Write new null terminator */
    end[1] = '\0';
    return str;
}

/* Validates if the provided string is in valid IPv4 format.
 * Returns 1 if valid, 0 otherwise.
 */
int validate_ipv4_address(const char *ip)
{
    int segments, ch_count;
    const char *ptr;

    segments = 0;
    ch_count = 0;
    ptr = ip;

    while (*ptr)
    {
        if (*ptr == '.')
        {
            if (ch_count == 0 || ch_count > 3)
            {
                return 0;
            }
            segments++;
            ch_count = 0;
        }
        else if (!isdigit((unsigned char)*ptr))
        {
            return 0;
        }
        else
        {
            ch_count++;
        }
        ptr++;
    }

    if (segments != 3 || ch_count == 0 || ch_count > 3)
    {
        return 0;
    }
    return 1;
}

/* Creates a new Database instance.
 * Returns a pointer to the new Database or NULL on failure.
 */
Database *create_db(void)
{
    Database *db;

    db = malloc(sizeof(Database));
    if (db == NULL)
    {
        printf("Failed to allocate memory for DB.\n");
        return NULL;
    }

    db->tables = NULL;
    db->table_count = 0;
    return db;
}

/* Frees all memory associated with the Database.
 */
void free_database(Database *db)
{
    int i, j, r;
    Table *currTable;
    Column *currColumn;

    if (db == NULL)
    {
        return;
    }

    for (i = 0; i < db->table_count; i++)
    {
        currTable = db->tables[i];
        free(currTable->name);

        for (j = 0; j < currTable->column_count; j++)
        {
            currColumn = currTable->columns[j];
            free(currColumn->name);

            for (r = 0; r < currTable->row_count; r++)
            {
                free(currColumn->data[r]);
            }
            free(currColumn->data);
            free(currColumn);
        }
        free(currTable->columns);
        free(currTable);
    }
    free(db->tables);
    free(db);
}

/* Searches for a table by name in the Database.
 * Returns a pointer to the Table if found, or NULL otherwise.
 */
Table *find_table(Database *db, const char *table_name)
{
    int i;

    for (i = 0; i < db->table_count; i++)
    {
        if (strcmp(db->tables[i]->name, table_name) == 0)
        {
            return db->tables[i];
        }
    }
    return NULL;
}

/* Creates a new table with the given name and comma-separated column definitions.
 */
void create_table(Database *db, const char *table_name, const char *columns_str)
{
    Table *table;
    char *cols_copy;
    char *token;
    Column *col;

    if (find_table(db, table_name) != NULL)
    {
        printf("Error: Table '%s' already exists.\n", table_name);
        return;
    }

    table = malloc(sizeof(Table));
    if (table == NULL)
    {
        printf("Error: Memory allocation failed for table '%s'.\n", table_name);
        return;
    }
    table->name = strdup(table_name);
    table->row_count = 0;
    table->column_count = 0;
    table->columns = NULL;

    cols_copy = strdup(columns_str);
    if (cols_copy == NULL)
    {
        printf("Error: Memory allocation failed for columns copy.\n");
        free(table);
        return;
    }

    token = strtok(cols_copy, ",");
    while (token != NULL)
    {
        token = trim_whitespace(token);
        col = malloc(sizeof(Column));
        if (col == NULL)
        {
            printf("Error: Memory allocation failed for column '%s'.\n", token);
            free(cols_copy);
            return;
        }
        col->name = strdup(token);
        col->data = NULL;

        table->columns = realloc(table->columns, sizeof(Column*) * (table->column_count + 1));
        if (table->columns == NULL)
        {
            printf("Error: Memory allocation failed while adding column '%s'.\n", token);
            free(col->name);
            free(col);
            free(cols_copy);
            return;
        }
        table->columns[table->column_count++] = col;
        token = strtok(NULL, ",");
    }
    free(cols_copy);

    if (table->column_count == 0)
    {
        printf("Error: No columns defined for table '%s'.\n", table_name);
        free(table->name);
        free(table);
        return;
    }

    db->tables = realloc(db->tables, sizeof(Table*) * (db->table_count + 1));
    if (db->tables == NULL)
    {
        printf("Error: Memory allocation failed while adding table '%s'.\n", table_name);
        free_database(db);
        exit(EXIT_FAILURE);
    }
    db->tables[db->table_count++] = table;

    printf("Table '%s' with %d columns created successfully.\n", table->name, table->column_count);
}

/* Inserts a new row into the specified table using comma-separated values.
 */
void insert_into_table(Database *db, const char *table_name, const char *values_str)
{
    Table *table;
    char *vals_copy;
    char *token;
    int col_index;
    char **values;
    int i;
    Column *col;

    table = find_table(db, table_name);
    if (table == NULL)
    {
        printf("Error: Table '%s' does not exist.\n", table_name);
        return;
    }

    vals_copy = strdup(values_str);
    if (vals_copy == NULL)
    {
        printf("Error: Memory allocation failed for values copy.\n");
        return;
    }

    token = strtok(vals_copy, ",");
    col_index = 0;
    values = malloc(sizeof(char*) * table->column_count);
    if (values == NULL)
    {
        printf("Error: Memory allocation failed for values array.\n");
        free(vals_copy);
        return;
    }

    while (token != NULL && col_index < table->column_count)
    {
        token = trim_whitespace(token);
        /* Validate IPv4 address if required */
        if (strcmp(table->columns[col_index]->name, "IPv4") == 0)
        {
            if (!validate_ipv4_address(token))
            {
                printf("Error: Invalid IPv4 address '%s'.\n", token);
                free(vals_copy);
                for (i = 0; i < col_index; i++)
                {
                    free(values[i]);
                }
                free(values);
                return;
            }
        }
        values[col_index++] = strdup(token);
        token = strtok(NULL, ",");
    }
    free(vals_copy);

    if (col_index != table->column_count)
    {
        printf("Error: Column count mismatch for table '%s'.\n", table_name);
        for (i = 0; i < col_index; i++)
        {
            free(values[i]);
        }
        free(values);
        return;
    }

    for (i = 0; i < table->column_count; i++)
    {
        col = table->columns[i];
        col->data = realloc(col->data, sizeof(char*) * (table->row_count + 1));
        if (col->data == NULL)
        {
            printf("Error: Memory allocation failed while inserting row.\n");
            free(values);
            return;
        }
        col->data[table->row_count] = values[i];
    }
    free(values);
    table->row_count++;
    printf("Row inserted into table '%s'.\n", table_name);
}

/* Displays the contents of the specified table.
 */
void select_from_table(Database *db, const char *table_name)
{
    Table *table;
    int i, c, r;

    table = find_table(db, table_name);
    if (table == NULL)
    {
        printf("Error: Table '%s' does not exist.\n", table_name);
        return;
    }

    printf("Table: %s\n", table->name);
    for (i = 0; i < table->column_count; i++)
    {
        printf("%s\t", table->columns[i]->name);
    }
    printf("\n");

    for (r = 0; r < table->row_count; r++)
    {
        for (c = 0; c < table->column_count; c++)
        {
            printf("%s\t", table->columns[c]->data[r]);
        }
        printf("\n");
    }
}

/* Saves the database to a binary file.
 */
void save_database_to_file(Database *db, const char *filename)
{
    FILE *file;
    int i, j, r;
    Table *table;
    Column *col;

    file = fopen(filename, "wb");
    if (file == NULL)
    {
        printf("Error: Could not open file '%s' for writing.\n", filename);
        return;
    }

    fwrite(&db->table_count, sizeof(int), 1, file);
    for (i = 0; i < db->table_count; i++)
    {
        table = db->tables[i];
        write_string(file, table->name);
        fwrite(&table->column_count, sizeof(int), 1, file);
        fwrite(&table->row_count, sizeof(int), 1, file);

        for (j = 0; j < table->column_count; j++)
        {
            col = table->columns[j];
            write_string(file, col->name);
            for (r = 0; r < table->row_count; r++)
            {
                write_string(file, col->data[r]);
            }
        }
    }
    fclose(file);
    printf("Database saved to '%s'.\n", filename);
}

/* Loads a database from a binary file.
 * Frees the current database and returns a new one loaded from the file.
 */
Database *load_database_from_file(Database *db, const char *filename)
{
    FILE *file;
    Database *new_db;
    int table_count;
    int i, j, r;
    Table *table;
    Column *col;
    char *cell;

    file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("Error: Could not open file '%s' for reading.\n", filename);
        return db;
    }

    free_database(db);
    new_db = create_db();
    if (new_db == NULL)
    {
        fclose(file);
        return NULL;
    }

    table_count = 0;
    fread(&table_count, sizeof(int), 1, file);
    for (i = 0; i < table_count; i++)
    {
        table = malloc(sizeof(Table));
        if (table == NULL)
        {
            printf("Error: Memory allocation failed while loading table.\n");
            continue;
        }

        table->name = read_string(file);
        fread(&table->column_count, sizeof(int), 1, file);
        fread(&table->row_count, sizeof(int), 1, file);
        table->columns = NULL;

        for (j = 0; j < table->column_count; j++)
        {
            col = malloc(sizeof(Column));
            if (col == NULL)
            {
                printf("Error: Memory allocation failed while loading column.\n");
                continue;
            }
            col->name = read_string(file);
            col->data = NULL;

            for (r = 0; r < table->row_count; r++)
            {
                cell = read_string(file);
                col->data = realloc(col->data, sizeof(char*) * (r + 1));
                if (col->data == NULL)
                {
                    printf("Error: Memory allocation failed while loading row data.\n");
                    free(cell);
                    continue;
                }
                col->data[r] = cell;
            }
            table->columns = realloc(table->columns, sizeof(Column*) * (j + 1));
            if (table->columns == NULL)
            {
                printf("Error: Memory allocation failed while loading columns array.\n");
                free(col);
                continue;
            }
            table->columns[j] = col;
        }
        new_db->tables = realloc(new_db->tables, sizeof(Table*) * (i + 1));
        if (new_db->tables == NULL)
        {
            printf("Error: Memory allocation failed while adding table to database.\n");
            free(table);
            continue;
        }
        new_db->tables[i] = table;
        new_db->table_count++;
    }
    fclose(file);
    printf("Database loaded from '%s'.\n", filename);
    return new_db;
}

/* Parses and executes a query string.
 * Supported commands: CREATE TABLE, INSERT INTO, SELECT, SAVE, LOAD.
 */
Database *parse_query(Database *db, const char *query)
{
    char query_copy[MAX_QUERY_LENGTH];
    char *command;
    char *next_token;
    char *table_name;
    char *columns;
    char *closing_paren;
    char *values;

    strncpy(query_copy, query, MAX_QUERY_LENGTH - 1);
    query_copy[MAX_QUERY_LENGTH - 1] = '\0';

    command = strtok(query_copy, " ");
    if (command == NULL)
    {
        printf("Error: Empty query.\n");
        return db;
    }

    if (strcmp(command, "CREATE") == 0)
    {
        next_token = strtok(NULL, " ");
        if (next_token == NULL || strcmp(next_token, "TABLE") != 0)
        {
            printf("Error: Invalid CREATE TABLE syntax.\n");
            return db;
        }

        table_name = strtok(NULL, " ");
        if (table_name == NULL)
        {
            printf("Error: Table name is missing.\n");
            return db;
        }

        columns = strchr(query, '(');
        if (columns == NULL)
        {
            printf("Error: Missing column definitions.\n");
            return db;
        }
        columns++;
        closing_paren = strchr(columns, ')');
        if (closing_paren == NULL)
        {
            printf("Error: Missing closing parenthesis in column definitions.\n");
            return db;
        }
        *closing_paren = '\0';

        columns = trim_whitespace(columns);
        if (strlen(columns) == 0)
        {
            printf("Error: No columns defined for table '%s'.\n", table_name);
            return db;
        }
        create_table(db, table_name, columns);
    }
    else if (strcmp(command, "INSERT") == 0)
    {
        next_token = strtok(NULL, " ");
        if (next_token == NULL || strcmp(next_token, "INTO") != 0)
        {
            printf("Error: Invalid INSERT INTO syntax.\n");
            return db;
        }

        table_name = strtok(NULL, " ");
        if (table_name == NULL)
        {
            printf("Error: Table name is missing.\n");
            return db;
        }

        values = strchr(query, '(');
        if (values == NULL)
        {
            printf("Error: Missing values.\n");
            return db;
        }
        values++;
        closing_paren = strchr(values, ')');
        if (closing_paren == NULL)
        {
            printf("Error: Missing closing parenthesis in values.\n");
            return db;
        }
        *closing_paren = '\0';

        values = trim_whitespace(values);
        if (strlen(values) == 0)
        {
            printf("Error: No values provided for table '%s'.\n", table_name);
            return db;
        }
        insert_into_table(db, table_name, values);
    }
    else if (strcmp(command, "SELECT") == 0)
    {
        /* Expected syntax: SELECT * FROM table_name */
        strtok(NULL, " ");  /* Skip '*' */
        strtok(NULL, " ");  /* Skip 'FROM' */
        table_name = strtok(NULL, " ");
        if (table_name == NULL)
        {
            printf("Error: Table name is missing in SELECT query.\n");
            return db;
        }
        select_from_table(db, table_name);
    }
    else if (strcmp(command, "SAVE") == 0)
    {
        save_database_to_file(db, DB_FILE);
    }
    else if (strcmp(command, "LOAD") == 0)
    {
        db = load_database_from_file(db, DB_FILE);
    }
    else
    {
        printf("Error: Unsupported query.\n");
    }

    return db;
}
