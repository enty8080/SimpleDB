#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "db.h"

void trim_whitespace(char *str)
{
    char *end;

    while (isspace((unsigned char)*str))
    {
        str++;
    }

    if (*str == 0)
    {
        return;
    }

    end = str + strlen(str) - 1;

    while (end > str && isspace((unsigned char)*end))
    {
        end--;
    }

    end[1] = '\0';
}

int validate_ipv4_address(const char *ip)
{
    int segments;
    int ch_count;
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

Table *find_table(const char *table_name)
{
    int i;

    for (i = 0; i < db.table_count; i++)
    {
        if (strcmp(db.tables[i].name, table_name) == 0)
        {
            return &db.tables[i];
        }
    }

    return NULL;
}

void create_table(const char *table_name, char *columns)
{
    Table *table;
    char *token;

    if (db.table_count >= MAX_TABLES)
    {
        printf("Error: Maximum table limit reached.\n");
        return;
    }

    if (find_table(table_name))
    {
        printf("Error: Table '%s' already exists.\n", table_name);
        return;
    }

    table = &db.tables[db.table_count++];
    strcpy(table->name, table_name);
    table->column_count = 0;
    table->row_count = 0;

    token = strtok(columns, ",");

    while (token && table->column_count < MAX_COLUMNS)
    {
        trim_whitespace(token);
        strcpy(table->columns[table->column_count++].name, token);
        token = strtok(NULL, ",");
    }

    if (table->column_count == 0)
    {
        printf("Error: No columns defined for table '%s'.\n", table_name);
        db.table_count--;
        return;
    }

    printf("Table '%s' with %d columns created successfully.\n", table_name, table->column_count);
}

void insert_into_table(const char *table_name, char *values)
{
    Table *table;
    char *token;
    int col_index;

    table = find_table(table_name);

    if (!table)
    {
        printf("Error: Table '%s' does not exist.\n", table_name);
        return;
    }

    if (table->column_count == 0)
    {
        printf("Error: Table '%s' has no columns defined.\n", table_name);
        return;
    }

    if (table->row_count >= MAX_ROWS)
    {
        printf("Error: Maximum row limit reached for table '%s'.\n", table_name);
        return;
    }

    token = strtok(values, ",");
    col_index = 0;

    while (token && col_index < table->column_count)
    {
        trim_whitespace(token);

        if (strcmp(table->columns[col_index].name, "IPv4") == 0)
        {
            if (!validate_ipv4_address(token))
            {
                printf("Error: Invalid IPv4 address '%s'.\n", token);
                return;
            }
        }

        strcpy(table->columns[col_index].data[table->row_count], token);
        token = strtok(NULL, ",");
        col_index++;
    }

    if (col_index != table->column_count)
    {
        printf("Error: Column count mismatch for table '%s'.\n", table_name);
        return;
    }

    table->row_count++;
    printf("Row inserted into table '%s'.\n", table_name);
}

void select_from_table(const char *table_name)
{
    Table *table;
    int i;
    int j;

    table = find_table(table_name);

    if (!table)
    {
        printf("Error: Table '%s' does not exist.\n", table_name);
        return;
    }

    printf("Table: %s\n", table->name);

    for (i = 0; i < table->column_count; i++)
    {
        printf("%s\t", table->columns[i].name);
    }

    printf("\n");

    for (i = 0; i < table->row_count; i++)
    {
        for (j = 0; j < table->column_count; j++)
        {
            printf("%s\t", table->columns[j].data[i]);
        }

        printf("\n");
    }
}

void save_database_to_file(const char *filename)
{
    FILE *file;

    file = fopen(filename, "wb");

    if (!file)
    {
        printf("Error: Could not open file '%s' for writing.\n", filename);
        return;
    }

    fwrite(&db, sizeof(Database), 1, file);

    fclose(file);
    printf("Database saved to '%s'.\n", filename);
}

void load_database_from_file(const char *filename)
{
    FILE *file;

    file = fopen(filename, "rb");

    if (!file)
    {
        printf("Error: Could not open file '%s' for reading.\n", filename);
        return;
    }

    fread(&db, sizeof(Database), 1, file);

    fclose(file);
    printf("Database loaded from '%s'.\n", filename);
}

void parse_query(const char *query)
{
    char query_copy[MAX_QUERY_LENGTH];
    char *command;
    char *table_name;
    char *columns;

    strcpy(query_copy, query);
    command = strtok(query_copy, " ");

    if (strcmp(command, "CREATE") == 0)
    {
        char *next_token = strtok(NULL, " ");
        if (next_token == NULL || strcmp(next_token, "TABLE") != 0)
        {
            printf("Error: Invalid CREATE TABLE syntax.\n");
            return;
        }

        table_name = strtok(NULL, " ");
        if (table_name == NULL)
        {
            printf("Error: Table name is missing.\n");
            return;
        }

        columns = strchr(query, '(');
        if (columns == NULL)
        {
            printf("Error: Missing column definitions.\n");
            return;
        }

        columns++;
        char *closing_paren = strchr(columns, ')');
        if (closing_paren == NULL)
        {
            printf("Error: Missing closing parenthesis in column definitions.\n");
            return;
        }

        *closing_paren = '\0';
        trim_whitespace(columns);

        if (strlen(columns) == 0)
        {
            printf("Error: No columns defined for table '%s'.\n", table_name);
            return;
        }

        create_table(table_name, columns);
    }
    else if (strcmp(command, "INSERT") == 0)
    {
        char *next_token = strtok(NULL, " ");
        if (next_token == NULL || strcmp(next_token, "INTO") != 0)
        {
            printf("Error: Invalid INSERT INTO syntax.\n");
            return;
        }

        table_name = strtok(NULL, " ");
        if (table_name == NULL)
        {
            printf("Error: Table name is missing.\n");
            return;
        }

        columns = strchr(query, '(');
        if (columns == NULL)
        {
            printf("Error: Missing values.\n");
            return;
        }

        columns++;
        char *closing_paren = strchr(columns, ')');
        if (closing_paren == NULL)
        {
            printf("Error: Missing closing parenthesis in values.\n");
            return;
        }

        *closing_paren = '\0';
        trim_whitespace(columns);

        if (strlen(columns) == 0)
        {
            printf("Error: No values provided for table '%s'.\n", table_name);
            return;
        }

        insert_into_table(table_name, columns);
    }
    else if (strcmp(command, "SELECT") == 0)
    {
        strtok(NULL, " ");
        strtok(NULL, " ");
        table_name = strtok(NULL, " ");
        if (table_name == NULL)
        {
            printf("Error: Table name is missing in SELECT query.\n");
            return;
        }

        select_from_table(table_name);
    }
    else if (strcmp(command, "SAVE") == 0)
    {
        save_database_to_file(DB_FILE);
    }
    else if (strcmp(command, "LOAD") == 0)
    {
        load_database_from_file(DB_FILE);
    }
    else
    {
        printf("Error: Unsupported query.\n");
    }
}