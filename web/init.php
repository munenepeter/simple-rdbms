<?php

declare(strict_types=1); //10.01.2026
/**
 * init.php
 * 
 * we can do some stuff here, like time things or something
 * this will be my 'app/bootstrap.php' like in laravel
 *
 * @author Peter Munene <munenenjega@gmail.com>
 */



//CHANGE THIS

define('DATABASE_NAME', 'webapp_db');








set_error_handler('myErrorHandler');

//Handle fatal errors and exceptions (shutdown handler will call our handler)
register_shutdown_function(function () {
    $error = error_get_last();
    if ($error && in_array($error['type'], [E_ERROR, E_CORE_ERROR, E_COMPILE_ERROR, E_USER_ERROR, E_PARSE])) {
        myErrorHandler($error['type'], $error['message'], $error['file'], $error['line']);
    }
});

set_exception_handler(function ($exception) {
    myErrorHandler(
        E_ERROR,
        $exception->getMessage(),
        $exception->getFile(),
        $exception->getLine(),
        []
    );
});

//initate the db
require_once __DIR__ . '/db_handler.php';


//must define database
$db = Database::getInstance(DATABASE_NAME);

/// utils
//handle for backend simple errors
$handleErrors = function ($response) {
    if (!$response['success']) {
        http_response_code(500);
        echo json_encode(['error' => $response['error']]);
        exit;
    }
};


function myErrorHandler($severity, $message, $file, $line, $context = []) {
    header('Content-Type: text/html');
    http_response_code(500);

    // Don't handle errors that are suppressed with @
    if (!(error_reporting() & $severity)) {
        return false;
    }

    // Get error type name
    $errorTypes = [
        E_ERROR => 'Fatal Error',
        E_WARNING => 'Warning',
        E_PARSE => 'Parse Error',
        E_NOTICE => 'Notice',
        E_CORE_ERROR => 'Core Error',
        E_CORE_WARNING => 'Core Warning',
        E_COMPILE_ERROR => 'Compile Error',
        E_COMPILE_WARNING => 'Compile Warning',
        E_USER_ERROR => 'User Error',
        E_USER_WARNING => 'User Warning',
        E_USER_NOTICE => 'User Notice',
        E_RECOVERABLE_ERROR => 'Recoverable Error',
        E_DEPRECATED => 'Deprecated',
        E_USER_DEPRECATED => 'User Deprecated'
    ];

    $errorType = $errorTypes[$severity] ?? 'Unknown Error';

    // Get stack trace
    $trace = debug_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS);

    // Display the error
    displayError($errorType, $message, $file, $line, $trace);

    // Stop execution for fatal errors (including user-generated fatal errors)
    if (in_array($severity, [E_ERROR, E_USER_ERROR, E_CORE_ERROR, E_COMPILE_ERROR, E_PARSE])) {
        exit(1);
    }

    return true;
}


//this was generated using ai to resemble what laravel provides
function displayError($errorType, $message, $file, $line) {
    // Ensure status code is 500

    // Clear any existing output
    if (ob_get_level()) {
        ob_clean();
    }

    // If client expects JSON (API/XHR), return JSON
    // fo now it does not help
    // $accept = $_SERVER['HTTP_ACCEPT'] ?? '';
    // $isAjax = isset($_SERVER['HTTP_X_REQUESTED_WITH']) && strtolower($_SERVER['HTTP_X_REQUESTED_WITH']) === 'xmlhttprequest';
    // if (strpos($accept, 'application/json') !== false || $isAjax) {
    //     header('Content-Type: application/json; charset=UTF-8');
    //     echo json_encode(['error' => $message]);
    //     exit;
    // }

    // Intercept Missing DATABASE_NAME error
    //e.g
    // //Database 'webapp_db' not found. Tried:\n  - data\/webapp_db\/db.meta\n  - ..\/data\/webapp_db\/db.meta\n  - ..\/..\/data\/webapp_db\/db.meta\nFailed to open database 'webapp_db'
    $databaseName = defined('DATABASE_NAME') ?  DATABASE_NAME :  'webapp_db';



    if (str_contains($message, "Database '$databaseName' not found") || str_contains($message, "Tried:\n - data\/")) {
        // Web: show a styled notice and button to run migrations
        $migrationHTML = '
    <div style="padding: 30px;">
        <div style="background: #fff3cd; padding: 15px; margin-bottom: 25px; border-radius: 4px;">
            <strong style="color: #856404; display: block; margin-bottom: 5px;">Database Not Found</strong>
            <p style="margin: 0; color: #856404; font-size: 14px; line-height: 1.6;">
                The database <strong>' . $databaseName . '</strong> doesn\'t exist yet, or the database name in <code style="background: #f5f5f5; padding: 2px 6px; border-radius: 3px;">init.php</code> needs to be updated.
            </p>
        </div>
        
        <p style="color: #555; margin-bottom: 20px; font-size: 15px; line-height: 1.6;">
            You can automatically create the database with the schema and sample data below. This will set up two tables: <strong>users</strong> and <strong>books</strong>.
        </p>
        
        <div style="background: #f8f9fa; border: 1px solid #dee2e6; border-radius: 6px; padding: 20px; margin-bottom: 25px; overflow-x: auto;">
            <pre style="margin: 0; font-family: \'Courier New\', monospace; font-size: 13px; line-height: 1.6; color: #212529; white-space: pre-wrap; word-wrap: break-word;">CREATE DATABASE ' . $databaseName . ';

CREATE TABLE users (
    id INT PRIMARY KEY,
    name TEXT,
    email TEXT UNIQUE
);

CREATE TABLE books (
    id INT PRIMARY KEY,
    title TEXT,
    author TEXT,
    user_id INT
);

INSERT INTO users VALUES (1, "Peter", "munenenjega@gmail.com");
INSERT INTO users VALUES (2, "John", "john@gmail.com");
INSERT INTO users VALUES (3, "Jane", "jane@gmail.com");

INSERT INTO books VALUES (1, "My First Book", "Francine Smith", 1);
INSERT INTO books VALUES (2, "My Second Book", "Chris Smith", 2);
INSERT INTO books VALUES (3, "My Third Book", "Steve Anita", 1);</pre>
        </div>
        
        <!-- Action Button -->
        <form id="migration-form" method="post" style="text-align: center;">
        <input type="hidden" name="db" id="db-name" value="' . $databaseName . '">
            <button type="submit" name="run_migration" value="1" 
                style="background: blue; color: white; border: none; padding: 14px 35px; font-size: 16px; font-weight: 500; border-radius: 6px; cursor: pointer; box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4); transition: all 0.3s ease;"
                >
                Run Database Setup
            </button>
            <p style="margin: 15px 0 0 0; font-size: 13px; color: #6c757d;">
                This will create the database and populate it with sample data
            </p>
        </form>
        <script>
        // Intercept the migration form and send a message to the parent window (only once)
        (function () {
            var form = document.getElementById("migration-form");
            if (form) {
                form.addEventListener("submit", function (e) {
                    e.preventDefault();
                    // avoid double-posting
                    if (window.__migration_posted) return;
                    window.__migration_posted = true;

                    // read db name from hidden input if present
                    var dbInput = document.getElementById("db-name");
                    var db = dbInput ? dbInput.value : null;

                    // send message to parent to run migration (parent will perform the POST)
                    if (window.parent && window.parent !== window) {
                        window.parent.postMessage({ type: "run_migration", db: db }, "*");
                    } else {
                        // If not embedded, create a small form and submit so "db" is included
                        var hidden = document.createElement("input");
                        hidden.type = "hidden";
                        hidden.name = "db";
                        hidden.value = db || ' . $databaseName . ';
                        form.appendChild(hidden);
                        form.submit();
                    }
                });
            }
        })();
        </script>
        
    </div>

';
    }




    // Set content type for HTML
    header('Content-Type: text/html; charset=UTF-8');

    $html = '<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Error - ' . htmlspecialchars($errorType) . '</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background-color: #c3c1c13d;
            color: #374151;
            line-height: 1.6;
        }
    </style>
</head>
<body>
    <div style="max-width: 1200px; margin: 0 auto; padding: 2rem;">
    <!-- Error Header -->
    <div style="background: white; border-radius: 8px; box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1); margin-bottom: 1.5rem; padding: 2rem;">
    ';

    if (isset($migrationHTML)) {
        $html .= $migrationHTML;
    } else {

        $html . '
            <div style="display: flex; align-items: center; margin-bottom: 1rem;">
                <div style="width: 48px; height: 48px; background: #fef2f2; border-radius: 50%; display: flex; align-items: center; justify-content: center; margin-right: 1rem;">
                    <svg style="width: 24px; height: 24px; color: #dc2626;" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 8v4m0 4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"></path>
                    </svg>
                </div>
                <div>
                    <h1 style="font-size: 1.5rem; font-weight: 600; color: #111827; margin-bottom: 0.25rem;">
                        ' . htmlspecialchars($errorType) . '
                    </h1>
                    <p style="color: #6b7280; font-size: 0.875rem;">
                        Thrown in <code style="background: #f3f4f6; padding: 0.125rem 0.25rem; border-radius: 4px; font-size: 0.75rem;">' . htmlspecialchars($file) . '</code> at line <strong>' . $line . '</strong>
                    </p>
                </div>
            </div>
            
            <div style="background: #f9fafb; padding: 1rem; border-radius: 0 4px 4px 0;">
                <p style="font-size: 1.125rem; color: #111827; font-weight: 500;">';


        //separate the message from the Actual SQL query if it exists

        $html .= displayErrorMsg($message);

        //close out the err div
        $html .= '</p></div>';
    }

    $html .= '</div><div></body></html>';

    echo $html;
    exit;
}

function displayErrorMsg(string $message, $forceFormatSQL = false): string {
    $html = '';
    if (strpos($message, ':QUERY:') !== false || $forceFormatSQL) {
        $parts = explode(':QUERY:', $message, 2);
        $sqlRaw = trim($parts[1]);

        // Decode to normal text before highlighting (remove HTML escaping and slashes)
        $sql = html_entity_decode($sqlRaw, ENT_QUOTES | ENT_HTML5);
        $sql = stripslashes($sql); // removes unnecessary slashes

        // Highlight SQL keywords (case-insensitive)
        $keywords = [
            'SELECT',
            'FROM',
            'WHERE',
            'INSERT',
            'INTO',
            'VALUES',
            'UPDATE',
            'SET',
            'DELETE',
            'JOIN',
            'LEFT',
            'RIGHT',
            'INNER',
            'OUTER',
            'ON',
            'AS',
            'AND',
            'OR',
            'ORDER BY',
            'GROUP BY',
            'LIMIT',
            'OFFSET',
            'DISTINCT',
            'COUNT',
            'AVG',
            'SUM',
            'MIN',
            'MAX',
            'LIKE',
            'IN',
            'NOT',
            'IS',
            'NULL',
            'EXISTS',
            'UNION',
            'CASE',
            'WHEN',
            'THEN',
            'ELSE',
            'END',
            'CAST',
            'CONCAT',
            'COALESCE',
            'IF',
            'SUBSTRING',
            'TRIM',
            'DESC'
        ];

        foreach ($keywords as $keyword) {
            $sql = preg_replace_callback(
                '/\b(' . preg_quote($keyword, '/') . ')\b/i',
                function ($matches) {
                    return '<span style="color: #ca9ee6; font-weight: bold;">' . $matches[0] . '</span>';
                },
                $sql
            );
        }

        // highlight string literals in green (Catppuccin style)
        $sql = preg_replace_callback(
            "/'([^']*)'/",
            function ($matches) {
                return '<span style="color: #16a34a;">' . htmlspecialchars($matches[0]) . '</span>';
            },
            $sql
        );

        $html .= htmlspecialchars(trim($parts[0])) . '<br><br><code style="background: #303446; color: #f8fafc; padding: 0.5rem; border-radius: 6px; font-size: 0.875rem; display: block; white-space: pre-wrap;">' . $sql . '</code>';
    } else {
        $html .= htmlspecialchars($message);
    }

    return $html;
}
