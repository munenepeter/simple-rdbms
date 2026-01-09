#!/usr/bin/env php
<?php

declare(strict_types=1); //09.01.2026

/**
 * Basic RDBMS Functionality Test
 * 
 * Test core operations: CREATE TABLE, INSERT, SELECT
 * 
 * @author Peter Munene <munenenjega@gmail.com>
 */

require_once __DIR__ . '/utils.php';

define('DATABASE_NAME', 'basic_test_db');
define('DATABASE_PATH', DATA_DIR . '/' . DATABASE_NAME);

info("--- Basic RDBMS Functionality Test ---");

cleanup(DATABASE_PATH);

info("0. Initializing database...");
$res = run("\"\" \"CREATE DATABASE " . DATABASE_NAME . ";\"");
if ($res['code'] !== 0) {
    error("FAILED to initialize database. Output: " . $res['output']);
    exit(1);
}

info("1. Creating table...");
$res = run(DATABASE_NAME . " \"CREATE TABLE users (id INT PRIMARY KEY, name TEXT UNIQUE);\"");
if ($res['code'] !== 0 || strpos($res['output'], "Table 'users' created") === false) {
    error("FAILED to create table. Output: " . $res['output']);
    exit(1);
}
success("Table 'users' created.");

info("2. Inserting data...");
run(DATABASE_NAME . " \"INSERT INTO users VALUES (1, 'alice');\"");
run(DATABASE_NAME . " \"INSERT INTO users VALUES (2, 'bob');\"");
$res = run(DATABASE_NAME . " \"INSERT INTO users VALUES (3, 'charlie');\"");
if ($res['code'] !== 0 || strpos($res['output'], "1 row inserted") === false) {
    error("FAILED to insert data. Output: " . $res['output']);
    exit(1);
}
success("Data inserted.");

info("3. Selecting all data...");
$res = run(DATABASE_NAME . " \"SELECT * FROM users;\"");
display("Output:\n" . $res['output']);
if (strpos($res['output'], "alice") === false || strpos($res['output'], "bob") === false || strpos($res['output'], "charlie") === false) {
    error("FAILED select all test.");
    exit(1);
}
success("Select all verified.");

info("4. Selecting with WHERE...");
$res = run(DATABASE_NAME . " \"SELECT * FROM users WHERE id = 2;\"");
display("Output:\n" . $res['output']);
if (strpos($res['output'], "bob") === false || strpos($res['output'], "alice") !== false) {
    error("FAILED select WHERE test.");
    exit(1);
}
success("Select WHERE verified.");

success("\nPASSED: Basic functionality verified.");
