#!/usr/bin/env php
<?php

declare(strict_types=1); //09.01.2026

/**
 * DML and JOIN Test
 * 
 * Test UPDATE, DELETE, and INNER JOIN operations
 * 
 * @author Peter Munene <munenenjega@gmail.com>
 */

require_once __DIR__ . '/utils.php';

define('DATABASE_NAME', 'dml_join_db');
define('DATABASE_PATH', DATA_DIR . '/' . DATABASE_NAME);

info("--- Testing DML and JOIN ---");

cleanup(DATABASE_PATH);

info("0. Initializing database...");
run("\"\" \"CREATE DATABASE " . DATABASE_NAME . ";\"");

// 2. Create Tables
info("1. Creating tables...");
run(DATABASE_NAME . " \"CREATE TABLE users (id INT PRIMARY KEY, name TEXT UNIQUE);\"");
run(DATABASE_NAME . " \"CREATE TABLE posts (id INT PRIMARY KEY, user_id INT, title TEXT);\"");

// 3. Insert Data
info("2. Inserting data...");
run(DATABASE_NAME . " \"INSERT INTO users VALUES (1, 'alice');\"");
run(DATABASE_NAME . " \"INSERT INTO users VALUES (2, 'bob');\"");
run(DATABASE_NAME . " \"INSERT INTO posts VALUES (101, 1, 'Hello World');\"");
run(DATABASE_NAME . " \"INSERT INTO posts VALUES (102, 1, 'My Second Post');\"");
run(DATABASE_NAME . " \"INSERT INTO posts VALUES (103, 2, 'Bobs Post');\"");

// 4. Test JOIN
info("3. Testing INNER JOIN...");
$res = run(DATABASE_NAME . " \"SELECT * FROM users JOIN posts ON id = user_id;\"");
display("Output:\n" . $res['output']);
if (strpos($res['output'], "Hello World") !== false && strpos($res['output'], "Bobs Post") !== false) {
    success("PASSED: JOIN verified.");
} else {
    error("FAILED: JOIN verification failed.");
    exit(1);
}

// 5. Test UPDATE
info("4. Testing UPDATE...");
run(DATABASE_NAME . " \"UPDATE users SET name = 'alice_updated' WHERE id = 1;\"");
$res = run(DATABASE_NAME . " \"SELECT * FROM users WHERE id = 1;\"");
display("Output:\n" . $res['output']);
if (strpos($res['output'], "alice_updated") !== false) {
    success("PASSED: UPDATE verified.");
} else {
    error("FAILED: UPDATE verification failed.");
    exit(1);
}

// 6. Test DELETE
info("5. Testing DELETE...");
run(DATABASE_NAME . " \"DELETE FROM posts WHERE user_id = 2;\"");
$res = run(DATABASE_NAME . " \"SELECT * FROM posts;\"");
display("Output:\n" . $res['output']);
if (strpos($res['output'], "Bobs Post") === false) {
    success("PASSED: DELETE verified.");
} else {
    error("FAILED: DELETE verification failed.");
    exit(1);
}

success("\n--- All DML and JOIN tests completed ---");
