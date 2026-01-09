#!/usr/bin/env php
<?php

declare(strict_types=1); //09.01.2026

/**
 * Integration Test for Simple RDBMS
 * 
 * Test that the built RDBMS can:
 * 1. Initialize a database
 * 2. Create a table
 * 3. Insert data
 * 4. Query data
 * 
 * Usage: php tests/integration_test.php
 * 
 * @internal Please remember this was part of recruitement assesment and is no way to be used in prod
 * 
 * License: MIT
 * This source file is subject to the MIT license that is bundled
 * with this source code in the file LICENSE
 * 
 * @author Peter Munene <munenenjega@gmail.com>
 */

//include utils
require_once __DIR__ . '/utils.php';

define('DATABASE_NAME', 'integration_test_db');
define('DATABASE_PATH', DATA_DIR . '/' . DATABASE_NAME);

info("--- Starting Integration Tests ---");

if (is_dir(DATABASE_PATH)) {
    info("Cleaning up previous test database...");
    cleanup(DATABASE_PATH);
}


info("Test 1: Database Initialization");
$res = run("\"\" \"CREATE DATABASE " . DATABASE_NAME . ";\"");
if ($res['code'] !== 0 || !is_dir(DATABASE_PATH)) {
    error("FAILED: Database Initialization");
    error($res['output']);
    exit(1);
}
success("PASSED: Database Initialization\n");

info("Test 2: Create Table");
$res = run(DATABASE_NAME . " \"CREATE TABLE users (id INT PRIMARY KEY, name TEXT);\"");
if ($res['code'] !== 0) {
    error("FAILED: Create Table");
    error($res['output']);
    exit(1);
}
success("PASSED: Create Table\n");

info("Test 3: Insert Data");
$res = run(DATABASE_NAME . " \"INSERT INTO users VALUES (10, 'James');\"");
if ($res['code'] !== 0 || strpos($res['output'], "1 row inserted") === false) {
    error("FAILED: Insert Data");
    error($res['output']);
    exit(1);
}
success("PASSED: Insert Data\n");

info("Test 4: Selection with columns");
$res = run(DATABASE_NAME . " \"SELECT name, id FROM users WHERE id = 10;\"");
if ($res['code'] !== 0) {
    error("FAILED: Selection");
    error($res['output']);
    exit(1);
}
display("Output:\n" . $res['output'] . "\n");
if (strpos($res['output'], "James") !== false && strpos($res['output'], "10") !== false) {
    success("PASSED: Selection verified.\n");
} else {
    error("FAILED: Row data not found in SELECT output.");
    exit(1);
}

info("--- All Tests Passed Successfully ---\n");
