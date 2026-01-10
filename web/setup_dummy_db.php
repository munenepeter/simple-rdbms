<?php

declare(strict_types=1); //10.01.2026

/**
 * 
 * Set up dummy data on 1st run
 * 
 * @author Peter Munene <munenenjega@gmail.com>
 */

// require database helper (do not include full init to avoid recursion)
require_once __DIR__ . '/db_handler.php';

// ensure this is a POST and contains the expected flag
if ($_SERVER['REQUEST_METHOD'] !== 'POST' || !isset($_POST['run_migration'])) {
    http_response_code(400);
    echo json_encode(['error' => 'Invalid request']);
    exit;
}

// prefer posted db name if provided (sanitized: letters, numbers, underscores)
$postedDb = isset($_POST['db']) ? trim((string)$_POST['db']) : '';
if ($postedDb !== '') {
    if (!preg_match('/^[A-Za-z0-9_]+$/', $postedDb)) {
        http_response_code(400);
        echo json_encode(['error' => 'Invalid database name']);
        exit;
    }
    $databaseName = $postedDb;
} else {
    $databaseName = defined('DATABASE_NAME') ? DATABASE_NAME : 'webapp_db';
}

try {
    writeLog('info', 'Run migration requested for db: ' . $databaseName);

    $db = Database::getInstance($databaseName);

    $status = $db->createDatabase();

    if ($status) {
        $db->setUpDatabase($databaseName);
        echo json_encode(['success' => true, 'message' => 'Database setup completed']);
        exit;
    } else {
        http_response_code(500);
        echo json_encode(['error' => 'Database creation failed.']);
        exit;
    }
} catch (Throwable $e) {
    http_response_code(500);
    echo json_encode(['error' => 'Exception: ' . $e->getMessage()]);
    exit;
}
