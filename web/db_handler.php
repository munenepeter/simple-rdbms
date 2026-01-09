<?php

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

class Database {
    private string $dbExe;
    private string $dbName;

    public function __construct(string $dbName = 'webapp_db') {
        $this->dbName = $dbName;
        // path to db.exe relative to this file
        $this->dbExe = PHP_OS_FAMILY === 'Windows' ? realpath(__DIR__ . '/../build/db.exe') : realpath(__DIR__ . '/../build/db');

        if (!$this->dbExe) {
            throw new Exception("Database executable not found. Please build the project.");
        }

        //what if we do not have the database yet?

        //what about the tables?

        //how do we check?

        //for now i will do it manually cause am tired, 
        // please don't worry as this can only be true if no data

        // assuming the 1st ID Wil be always be 1, actually i'll make sure it is
        $result = $this->execute("SELECT * FROM users id = 1;");
        $success = $result['success'];
        $error = $result['error'] ?? '';

        //Database 'webapp_db' not found. Tried:\n  - data\/webapp_db\/db.meta\n  - ..\/data\/webapp_db\/db.meta\n  - ..\/..\/data\/webapp_db\/db.meta\nFailed to open database 'webapp_db'

        if (!$success && ($error === 'Table \'users\' not found' || str_contains($error, 'Database \'webapp_db\' not found'))) {

            //if db does not exist
            if (str_contains($error, 'Database \'webapp_db\' not found')) {
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
            $this->execute("INSERT INTO users VALUES (1, 'Peter', 'munenenjega@gmail.com'));");
            writeLog("info", "Inserted default admin user.");
        }
    }

    public function execute(string $sql): array {
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
        //
        if (str_contains($output, "Database") || str_contains($output, "' not found")) {
            writeLog("error", "Database or Table does not exist error.");
            writeLog("debug", "Output: $output");
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

    public function createDatabase(): void {
        $cmd = "\"{$this->dbExe}\" \"\" \"CREATE DATABASE {$this->dbName};\" 2>&1";
        shell_exec($cmd);
    }
}
