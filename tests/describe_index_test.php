#!/usr/bin/env php
<?php

declare(strict_types=1); //09.01.2026

/**
 * RDBMS DESCRIBE and INDEX Test
 * 
 * Test schema introspection and index lookup performance
 * 
 * @author Peter Munene <munenenjega@gmail.com>
 */

require_once __DIR__ . '/utils.php';

define('DATABASE_NAME', 'describe_index_db');
define('DATABASE_PATH', DATA_DIR . '/' . DATABASE_NAME);

info("--- RDBMS DESCRIBE and INDEX Test ---");

cleanup(DATABASE_PATH);

info("1. Initializing database...");
run("\"\" \"CREATE DATABASE " . DATABASE_NAME . ";\"");

info("2. Creating table with PRIMARY KEY and UNIQUE...");
run(DATABASE_NAME . " \"CREATE TABLE users (id INT PRIMARY KEY, name TEXT UNIQUE, age INT);\"");

info("3. Testing DESCRIBE...");
$res = run(DATABASE_NAME . " \"DESCRIBE users;\"");
display("Output:\n" . $res['output']);
$output_lower = strtolower($res['output']);
if (strpos($output_lower, "id") === false || strpos($output_lower, "pk") === false || strpos($output_lower, "unique") === false) {
    error("FAILED: DESCRIBE output missing expected info.");
    display($res['output']);
    exit(1);
}
success("DESCRIBE verified.");

info("4. Testing INDEX lookups (via SELECT)...");
run(DATABASE_NAME . " \"INSERT INTO users VALUES (1, 'alice', 30);\"");
run(DATABASE_NAME . " \"INSERT INTO users VALUES (2, 'bob', 25);\"");

$res = run(DATABASE_NAME . " \"SELECT * FROM users WHERE id = 1;\"");
display("Output:\n" . $res['output']);
if (strpos($res['output'], "alice") === false) {
    error("FAILED: Indexed lookup (id=1) failed.");
    exit(1);
}

$res = run(DATABASE_NAME . " \"SELECT * FROM users WHERE name = 'bob';\"");
display("Output:\n" . $res['output']);
if (strpos($res['output'], "25") === false) {
    error("FAILED: Indexed lookup (name='bob') failed.");
    exit(1);
}
success("Indexed SELECT lookups verified.");

info("5. Testing uniqueness constraint enforcement...");
$res = run(DATABASE_NAME . " \"INSERT INTO users VALUES (3, 'alice', 40);\"");
if ($res['code'] === 0) {
    error("FAILED: Accepted duplicate UNIQUE name.");
    exit(1);
}
success("Uniqueness enforced.");

success("\nALL DESCRIBE and INDEX tests PASSED.");
