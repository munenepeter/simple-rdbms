<?php
require_once __DIR__ . '/db_handler.php';

$db = new Database('webapp_db');

writeLog("info", "Initializing database...");
$db->createDatabase();

writeLog("info", "Creating users table...");
$db->execute("CREATE TABLE users (id INT PRIMARY KEY, name TEXT, email TEXT UNIQUE);");

writeLog("info", "Creating books table...");
$db->execute("CREATE TABLE books (id INT PRIMARY KEY, title TEXT, author TEXT, user_id INT);");

writeLog("info", "Initialization complete.\n");
