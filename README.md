## Simple DB

Simple DB proof of concept that utilizes SQL syntax for REPL.

### Examples

**Loading DB from file**

```
Simple SQL-like Database
Copyright (c) 2025 Ivan Nikolskiy, All Rights Reserved.

Supported commands: CREATE TABLE, INSERT INTO, SELECT * FROM, SAVE, LOAD

Enter SQL query: LOAD
Database loaded from 'database.db'.
Enter SQL query: SELECT * FROM Students
Table: Students
Name     Age     Major  
Alice    20      CS     
Bob      20      CS     
Enter SQL query: EXIT
```

**Writing your DB to file**

```
Simple SQL-like Database
Copyright (c) 2025 Ivan Nikolskiy, All Rights Reserved.

Supported commands: CREATE TABLE, INSERT INTO, SELECT * FROM, SAVE, LOAD

Enter SQL query: CREATE TABLE Students (Name, Age, Major)
Table 'Students' with 3 columns created successfully.
Enter SQL query: INSERT INTO Students VALUES (Alice, 20, CS)
Row inserted into table 'Students'.
Enter SQL query: INSERT INTO Students VALUES (Bob, 20, CS)
Row inserted into table 'Students'.
Enter SQL query: SELECT * FROM Students
Table: Students
Name	 Age	 Major
Alice	 20	 CS
Bob	 20	 CS
Enter SQL query: SAVE
Database saved to 'database.db'.
Enter SQL query: EXIT
```
