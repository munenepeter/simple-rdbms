<?php

declare(strict_types=1); //09.01.2026

/**
 * Utility functions for tests
 * 
 * License: MIT
 * This source file is subject to the MIT license that is bundled
 * with this source code in the file LICENSE
 * 
 * @author Peter Munene <munenenjega@gmail.com>
 */

if (PHP_SAPI !== 'cli') {
    echo "This script must be run from the command line.\n";
    exit(1);
}

//define paths
define('DATA_DIR', __DIR__ . '/../data');
define('BUILD_DIR', __DIR__ . '/../build');
define('RELEASE_DIR', BUILD_DIR . '/release');

// Check for executable: prefer release build, then regular build
$exe_name = PHP_OS_FAMILY === 'Windows' ? 'db.exe' : 'db';
$exe_path = null;

// First check release directory (for Release builds)
if (is_file(RELEASE_DIR . '/' . $exe_name)) {
    $exe_path = RELEASE_DIR . '/' . $exe_name;
}
// Fall back to regular build directory
elseif (is_file(BUILD_DIR . '/' . $exe_name)) {
    $exe_path = BUILD_DIR . '/' . $exe_name;
}

if ($exe_path === null) {
    die("Database executable not found. Please build the project first.\n");
}

define('DATABASE_EXCUTABLE', $exe_path);


function run(string $cmd): array {
    //note the space
    $cmd = DATABASE_EXCUTABLE . "  " . $cmd;
    info("Running: $cmd");
    $output = [];
    $resultCode = 0;
    exec($cmd, $output, $resultCode);
    return [
        'output' => implode("\n", $output),
        'code' => $resultCode
    ];
}

function cleanup(string $path): void {
    if (!is_dir($path)) return;
    $files = array_diff(scandir($path), ['.', '..']);
    foreach ($files as $file) {
        (is_dir("$path/$file")) ? cleanup("$path/$file") : unlink("$path/$file");
    }
    rmdir($path);
}


//show red cli color
function error(string $message): void {
    echo "\033[31mERROR: $message\033[0m\n";
}

//show green cli color
function success(string $message): void {
    echo "\033[32mSUCCESS: $message\033[0m\n";
}

//show blue cli color
function info(string $message): void {
    $message = cleanCmd($message);
    echo "\033[34mINFO: $message\033[0m\n";
}

//default with no color for outputs
function display(string $message): void {
    echo "$message\n";
}

//remove full path to the executable for cleaner output
function cleanCmd(string $cmd): string {
    // eg -> C:\Users\Peter\laragon\www\simple-rdbms\tests/../build/db.exe
    // or -> C:\Users\Peter\laragon\www\simple-rdbms\tests/../build/release/db.exe
    // return build/db.exe or build/release/db.exe only
    $cmd = preg_replace('/' . preg_quote(__DIR__ . '/../build/', '/') . '/', 'build/', $cmd);
    return $cmd;
}