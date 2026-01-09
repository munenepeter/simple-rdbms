#!/usr/bin/env php
<?php

declare(strict_types=1); //09.01.2026

/**
 * RDBMS Constraint Enforcement Test
 * 
 * Test PRIMARY KEY and UNIQUE constraints
 * 
 * @author Peter Munene <munenenjega@gmail.com>
 */

require_once __DIR__ . '/utils.php';

define('DATABASE_NAME', 'constraint_test_db');
define('DATABASE_PATH', DATA_DIR . '/' . DATABASE_NAME);

info("--- RDBMS Constraint Enforcement Test ---");

cleanup(DATABASE_PATH);

info("0. Initializing database...");
$res = run("\"\" \"CREATE DATABASE " . DATABASE_NAME . ";\"");
if ($res['code'] !== 0) {
    error("FAILED to initialize database. Output: " . $res['output']);
    exit(1);
}

run(DATABASE_NAME . " \"CREATE TABLE users (id INT PRIMARY KEY, name TEXT UNIQUE);\"");
run(DATABASE_NAME . " \"INSERT INTO users VALUES (1, 'alice');\"");

info("1. Testing Duplicate PRIMARY KEY...");
$res = run(DATABASE_NAME . " \"INSERT INTO users VALUES (1, 'bob');\"");
if ($res['code'] === 0) {
    error("FAILED: Accepted duplicate PRIMARY KEY (id=1).");
    exit(1);
}
success("Rejected duplicate PRIMARY KEY. Error: " . $res['output']);

info("2. Testing Duplicate UNIQUE column...");
$res = run(DATABASE_NAME . " \"INSERT INTO users VALUES (2, 'alice');\"");
if ($res['code'] === 0) {
    error("FAILED: Accepted duplicate UNIQUE name ('alice').");
    exit(1);
}
success("Rejected duplicate UNIQUE name. Error: " . $res['output']);

success("\nPASSED: Constraint enforcement verified.");
