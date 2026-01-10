<?php


declare(strict_types=1); //10.01.2026

/**
 * 
 * Set up dummy data on 1st run
 * 
 * @author Peter Munene <munenenjega@gmail.com>
 */

if ($_SERVER['REQUEST_METHOD'] !== 'POST' && !isset($_POST['run_migration'])) {
    trigger_error(" What are you trying to do?, you are not supposed to be here", E_USER_ERROR);
}

//can't use init here cause it is what has taken us her

// i hope db is defined?
$databaseName = defined('DATABASE_NAME') ?  DATABASE_NAME :  'webapp_db';

//create database if it does not exist
$db = new Database($databaseName);
$status = $db->createDatabase();

if($status){
    $db->setUpDatabase($databaseName);
}
