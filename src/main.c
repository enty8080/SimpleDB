#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "linenoise.h"
#include "db.h"

int main(void)
{
    char *query;
    Database *db;
    
    db = create_db();

    printf("Simple SQL-like Database\n");
    printf("Copyright (c) 2025 Ivan Nikolskiy, All Rights Reserved.\n\n");
    printf("Supported commands: CREATE TABLE, INSERT INTO, SELECT * FROM, SAVE, LOAD\n\n");

    while (1)
    {
        query = linenoise("Enter SQL query: ");
        if (!query)
        {
            break;
        }

        trim_whitespace(query);

        if (strcmp(query, "EXIT") == 0)
        {
            free(query);
            break;
        }

        if (strlen(query) > 0)
        {
            db = parse_query(db, query);
            linenoiseHistoryAdd(query);
        }

        free(query);
    }

    free_database(db);
    
    return 0;
}
