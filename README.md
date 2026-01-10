# Simple RDBMS
A simple Relational Database Management System (RDBMS) implemented in C, supporting basic SQL operations, indexing, and an interactive REPL.

This project was built as part of an assessment for an open call for junior developers at Pesapal. The challenge was to design and implement a simple RDBMS with support for:

- Declaring tables with a few column data types
- Full CRUD operations
- Basic indexing (Primary and Unique keying)
- Joining tables
- Interactive SQL REPL mode

## Features

- **SQL Lexer**: Robust tokenization using `stb_c_lexer.h`
- **SQL Parser**: AST generation for `CREATE TABLE`, `INSERT`, `SELECT`, `UPDATE`, `DELETE`, and more
- **REPL**: Interactive shell for executing SQL statements
- **Storage Engine**: On-disk persistence with row serialization and catalog management
- **Indexing**: Automatic indexing for PRIMARY KEY and UNIQUE columns
- **Constraints**: Support for PRIMARY KEY, UNIQUE, and NOT NULL constraints
- **Joins**: INNER and OUTER JOIN support

## Platform Support

> [!NOTE]  This project has only been tested on **Windows x64** systems. While the code should be portable to other platforms, it has not been tested on Linux or macOS.

**Pre-built executables** for Windows x64 are available in the [Releases](https://github.com/munenepeter/simple-rdbms/releases) section. Simply download and extract the executable to get started without building from source.

## Prerequisites

- **C Compiler** (GCC recommended, MinGW on Windows)
- **CMake** (version 3.10 or higher) - optional but recommended
- **Make** (optional, but recommended)

## Quick Start

### Option 1: Using Pre-built Executable

1. Download the latest release from the [Releases](https://github.com/munenepeter/simple-rdbms/releases) section
2. Extract the executable
3. Run the REPL:
   ```bash
   ./db.exe mydb
   ```

### Option 2: Building from Source

#### 1. Build the System

Using CMake (recommended):
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Or manually using GCC:
```bash
gcc -std=c99 -Wall -Wextra src/main.c src/repl.c src/lexer.c src/sql.c src/storage.c src/catalog.c src/table.c src/row.c src/index.c src/parser.c src/util.c -o db.exe
```

#### 2. Initialize Database

The system requires an explicit database directory within the `data/` folder. You can create a database using SQL:

```bash
./build/db.exe "" "CREATE DATABASE mydb;"
```

#### 3. Launch REPL

Start the interactive shell by specifying the database name:
```bash
./build/db.exe mydb
```

Or execute a single SQL command:
```bash
./build/db.exe mydb "SELECT * FROM users;"

#or create a table
./build/db.exe mydb "CREATE TABLE users (id INT PRIMARY KEY, name TEXT);"
```

## SQL Support

### DDL: Create Table

```sql
CREATE TABLE users (
    id INT PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    age INT
);
```

Supports `PRIMARY KEY`, `UNIQUE`, and `NOT NULL` constraints.

### DML: Insert, Select, Update, Delete

```sql
INSERT INTO users VALUES (1, "alice", 25);
SELECT * FROM users WHERE id = 1;
SELECT name, id FROM users WHERE id = 10;
UPDATE users SET age = 26 WHERE id = 1;
DELETE FROM users WHERE id = 1;
```

### Joins

```sql
SELECT * FROM users JOIN books ON users.id = books.user_id;
```

## Example Usage in REPL

```sql
db> CREATE TABLE users (id INT PRIMARY KEY, name TEXT);
Table 'users' created.
db> INSERT INTO users VALUES (1, "alice");
1 row inserted.
db> SELECT * FROM users;
name | id
----------
'alice' | 1
db> exit
```

## Architecture

The system follows a traditional UNIX-era database architecture with explicit stages:

1. **SQL Lexer**: Converts raw SQL strings into a stream of tokens
2. **SQL Parser**: Generates an Abstract Syntax Tree (AST) from tokens
3. **Executor**: Traverses the AST and calls the storage engine
4. **Storage Engine**: Handles on-disk persistence, row serialization, and catalog management

### On-Disk Layout

```
data/
└── mydb/
    ├── db.meta          # Database header
    ├── catalog.meta     # Table and column metadata
    ├── tables/
    │   ├── users.tbl    # Row data for 'users'
    │   └── posts.tbl    # Row data for 'posts'
    └── indexes/
        ├── users_id.idx # Index for users.id
        └── users_email.idx # Index for users.email
```

## Project Structure

```
.
├── src/           # Source code files
│   ├── libs/      # External libraries (stb_c_lexer.h)
│   ├── *.c        # Implementation files
│   └── *.h        # Header files
├── docs/          # Documentation
│   └── index.html # Full documentation website
├── tests/         # Test suite (PHP)
├── web/           # Web application demo (PHP) (to demonstrate usage)
├── data/          # Database storage directory
├── build/         # Build artifacts (gitignored)
└── CMakeLists.txt # Build configuration
```

## Web Application Demo

A simple web application demonstrating CRUD operations can be found in the `web/` directory. It's a PHP application that uses the RDBMS executable to manage users and books.

To run it, you just need a local web server with PHP, create a server 

```bash
cd web

#define the path to the db.exe executable
php -S localhost:8000 -d db.exe=/path/to/db.exe

#or use env

export DB_EXE=/path/to/db.exe
php -S localhost:8000

#place the db.exe executable in the web directory & run the server as normal

php -S localhost:8000

```

and navigate to `http://localhost:8000`.

## Why C?

> "I have been learning and teaching myself C for the last year and therefore I saw this as the best project to practice what I have been learning. Also, I'd think for a RDBMS, it needs to be fast and very efficient with its storage, and to be honest there is no faster language I know, other than raw Assembly, that comes close to C when it comes to performance and efficiency."

## Credits

- **[Stb C Lexer](https://github.com/nothings/stb)** - For the exceptional lexer utility
- **[Tsoding Daily](https://www.youtube.com/@TsodingDaily)** - For many lessons, tricks, and guidance
- **Google's Antigravity IDE** - For superhuman debugging assistance
