<?php

declare(strict_types=1); //10.01.2026

/**
 * Database handler, something like pdo
 * 
 * This is responsible for the actual communication with the database
 * 
 * @internal Please remembe i did thi as a part of recruitemnt challenge, so i haven't really
 *           checked if this is safe to be used in prod, or tested all edge cases or bugs
 *           if you find a bug, please let me know
 * 
 * 
 * @author Peter Munene <munenenjega@gmail.com>
 */

define('BUILD_DIR', __DIR__ . '/../build/');


class Database {
    private static ?Database $instance = null;
    private string $dbExe;
    private string $dbName;

    private function __construct(string $dbname = '') {
        if (empty($dbname)) $dbname = 'webapp_db';

        $this->dbName = $dbname;
        $this->dbExe = getDatabaseExcutable();

        //what if we do not have the database yet? or tables?
        //how do we check?
        //for now i will do it manually cause am tired, 
        // $this->setUpDatabase($dbname);
    }

    // sholuld be able to clone of the instance
    // no other instances should exit
    private function __clone() {
    }

    // should not be able to create an instance via unserializtion
    // only one active instance, withput being stored
    public function __wakeup() {
        throw new Exception("Cannot unserialize singleton");
    }

    public static function getInstance(string $dbname = ''): Database {
        if (self::$instance === null) {
            self::$instance = new self($dbname);
        }
        return self::$instance;
    }

    public function execute(string $sql, bool $internal = false): array {
        $cmd = "\"{$this->dbExe}\" {$this->dbName} \"$sql\" 2>&1";
        $output = shell_exec($cmd);

        if ($output === null) {
            return ['success' => false, 'error' => 'Failed to execute command'];
        }

        writeLog("info", "Executed: $cmd Output: $output");

        //check if there was an error
        //1. Database does not exist -> it has quotes so cannot check the exact string
        //2. e.g Table 'users' not found -> i have no time to parse the table from the query

        //and yeah before you start complaining, yes i can do this, after all i am the author of the db
        //TODO: parse the tale from the query
        if (str_contains($output, "Database") || str_contains($output, "' not found")) {

            if ($internal && str_contains($output, "' not found")) {
                writeLog("debug", "Some tabels do not exist attempting to create them.");
                // When running internal checks we should still surface failures (success=false)
                // so callers like setUpDatabase() can react and create missing tables.
                return ['success' => false, 'error' => trim($output)];
            }
            writeLog("error", "Table or Database does not exist.");
            writeLog("debug", "Output: $output");

            trigger_error(trim($output), E_USER_ERROR);
            return ['success' => false, 'error' => trim($output)];
        }

        return $this->parseOutput($output);
    }

    private function parseOutput(string $output): array {
        $lines = explode("\n", trim($output));
        if (empty($lines)) {
            return ['success' => true, 'data' => [], 'raw' => $output];
        }

        // common messages
        if (strpos($output, "1 row inserted") !== false || strpos($output, "rows inserted") !== false) {
            return ['success' => true, 'message' => trim($output)];
        }
        if (strpos($output, "Table created") !== false) {
            return ['success' => true, 'message' => trim($output)];
        }
        if (strpos($output, "Database created") !== false) {
            return ['success' => true, 'message' => trim($output)];
        }

        // try to parse table output (markdown style)
        // format:
        // col1 | col2
        // ----------
        // val1 | val2
        if (count($lines) >= 2 && strpos($lines[1], '---') !== false) {
            $headers = array_map('trim', explode('|', $lines[0]));
            $data = [];
            for ($i = 2; $i < count($lines); $i++) {
                if (empty(trim($lines[$i]))) continue;
                $rowValues = array_map(function ($val) {
                    $val = trim($val);
                    // rm quotes from strings if present
                    if (strpos($val, "'") === 0 && strrpos($val, "'") === strlen($val) - 1) {
                        $val = substr($val, 1, -1);
                    }
                    return $val;
                }, explode('|', $lines[$i]));

                if (count($rowValues) === count($headers)) {
                    $data[] = array_combine($headers, $rowValues);
                }
            }
            return ['success' => true, 'data' => $data];
        }

        return ['success' => true, 'message' => trim($output)];
    }

    public function createDatabase(): bool {

       //change dir to executable's path\
        chdir(__DIR__.'/../');

        $cmd = "\"{$this->dbExe}\" \"\" \"CREATE DATABASE {$this->dbName};\" 2>&1";

        $output = [];
        $returnCode = 0;

        exec($cmd, $output, $returnCode);

        // Join output lines for logging and checking
        $outputString = implode("\n", $output);

        writeLog("debug", "Create database command: $cmd");
        writeLog("debug", "Return code: $returnCode");
        writeLog("debug", "Output: $outputString");

        // Check if command execution failed at OS level
        if ($returnCode !== 0) {
            writeLog("error", "Failed to create database. Exit code: $returnCode");
            writeLog("error", "Error output: $outputString");
           // trigger_error("Failed to create database. Exit code: $returnCode", E_USER_ERROR);
            return false;
        }

        // Check if database was successfully created based on output
        if (str_contains($outputString, sprintf("Database '%s' initialized.", $this->dbName))) {
            writeLog("info", "Database '{$this->dbName}' created successfully.");
            return true;
        }

        // Command ran but database wasn't created (unexpected output)
        writeLog("warning", "Database creation command completed but confirmation message not found.");
        writeLog("warning", "Output received: $outputString");
        return false;
    }

    public function setUpDatabase($dbName): void {
        // please don't worry as this can only be true if no data

        // assuming the 1st ID Wil be always be 1, actually i'll make sure it is
        // check a specific user row to see if the table exists
        $result = $this->execute("SELECT * FROM users WHERE id = 1;", true);
        $success = $result['success'];
        $error = $result['error'] ?? ''; 

        //Database 'webapp_db' not found. Tried:\n  - data\/webapp_db\/db.meta\n  - ..\/data\/webapp_db\/db.meta\n  - ..\/..\/data\/webapp_db\/db.meta\nFailed to open database 'webapp_db'

        if (!$success && ($error === 'Table \'users\' not found' || str_contains($error, "Database '$dbName' not found"))) {

            //if db does not exist
            if (str_contains($error, "Database '$dbName' not found")) {
                writeLog("info", "Database not found, creating...");
                $this->createDatabase();
                writeLog("info", "Database created.");
            }

            writeLog("info", "Users table not found, creating...");
            $this->execute("CREATE TABLE users (id INT PRIMARY KEY, name TEXT, email TEXT UNIQUE);");
            writeLog("info", "Users table created.");
            //similarly for books table, cause am sure one can't be missing in isolation
            $this->execute("CREATE TABLE books (id INT PRIMARY KEY, title TEXT, author TEXT, user_id INT);");
            writeLog("info", "Books table created.");

            //insert 
            $this->execute("INSERT INTO users VALUES (1, 'Peter', 'munenenjega@gmail.com');");
            $this->execute("INSERT INTO users VALUES (2, 'Jane', 'jane@gmail.com');");
            $this->execute("INSERT INTO users VALUES (3, 'John', 'john@gmail.com');");
            writeLog("info", "Inserted default users.");


            //books
            $this->execute("INSERT INTO books VALUES (1, 'The Great Gatsby', 'F. Scott Fitzgerald', 1);");
            $this->execute("INSERT INTO books VALUES (2, 'To Kill a Mockingbird', 'Harper Lee', 2);");
            $this->execute("INSERT INTO books VALUES (3, '1984', 'George Orwell', 3);");
            $this->execute("INSERT INTO books VALUES (4, 'The Catcher in the Rye', 'J.D. Salinger', 1);");
            $this->execute("INSERT INTO books VALUES (5, 'Pride and Prejudice', 'Jane Austen', 2);");
            $this->execute("INSERT INTO books VALUES (6, 'The Lord of the Rings', 'J.R.R. Tolkien', 3);");
            writeLog("info", "Inserted default books.");
        }
    }
}

function writeLog($level = 'info', $message = ''): void {

    $file = __DIR__ . '/logs/web.log';

    if (!is_dir(__DIR__ . '/logs')) {
        mkdir(__DIR__ . '/logs', 0777, true);
    }
    if (!is_file($file)) {
        file_put_contents($file, "");
    }

    $cmd = date('Y-m-d H:i:s') . " - $level: $message";
    file_put_contents($file, $cmd . "\n", FILE_APPEND);
}

function getDatabaseExcutable(): string {
    // user can either
    // 1. php -S localhost:8000 -d db.exe=/path/to/db.exe -> via ini

    // 2. export DB_EXE=/path/to/db.exe &&  php -S localhost:8000

    // 3. place the excutable in the web directory & run the server as normal
    // php -S localhost:8000

    //cache to avoid re-runs on every request
    static $cached = null;
    if ($cached !== null) {
        return $cached;
    }

    //1. check if passed via the dev server
    //2. check if set in env
    $opts = [
        ini_get('db.exe'),
        $_ENV['DB_EXE'] ?? null,
    ];

    foreach ($opts as $path) {
        if ($path && is_file($path) && is_executable($path)) {
            return $cached = realpath($path);
        }
    }

    //3. check in build & current directory
    foreach ([__DIR__, BUILD_DIR] as $dir) {
        if (!is_dir($dir)) {
            continue;
        }

        foreach (scandir($dir) as $file) {
            if ($file === '.' || $file === '..') {
                continue;
            }

            $path = $dir . DIRECTORY_SEPARATOR . $file;
            if (is_file($path) && is_executable($path)) {
                return $cached = realpath($path);
            }
        }
    }

    throw new RuntimeException('Database executable not found. Please build the project.');
}
