<?php
header('Content-Type: application/json');
require_once __DIR__ . '/../db_handler.php';

$db = new Database('webapp_db');

$method = $_SERVER['REQUEST_METHOD'];

$handleErrors = function ($response) {
    if (!$response['success']) {
        http_response_code(500);
        echo json_encode(['error' => $response['error']]);
        exit;
    }
};

switch ($method) {
    case 'GET':
        if (isset($_GET['id'])) {
            $id = (int)$_GET['id'];
            $res = $db->execute("SELECT * FROM users WHERE id = $id;");

            $handleErrors($res);

            echo json_encode($res['data'][0] ?? null);
        } else {

            writeLog("info", "Getting all users...");

            $res = $db->execute("SELECT * FROM users;");

            $handleErrors($res);

            echo json_encode($res['data'] ?? []);
        }
        break;

    case 'POST':
        $data = json_decode(file_get_contents('php://input'), true);
        if (!$data || !isset($data['id']) || !isset($data['name'])) {
            http_response_code(400);
            echo json_encode(['error' => 'Missing required fields']);
            break;
        }
        $id = (int)$data['id'];
        $name = addslashes($data['name']);
        $res = $db->execute("INSERT INTO users VALUES ($id, '$name');");
        $handleErrors($res);
        echo json_encode($res);
        break;

    case 'PUT':
        $data = json_decode(file_get_contents('php://input'), true);
        if (!$data || !isset($data['id']) || !isset($data['name'])) {
            http_response_code(400);
            echo json_encode(['error' => 'Missing required fields']);
            break;
        }
        $id = (int)$data['id'];
        $name = addslashes($data['name']);
        $res = $db->execute("UPDATE users SET name = '$name' WHERE id = $id;");
        $handleErrors($res);
        echo json_encode($res);
        break;

    case 'DELETE':
        $id = (int)$_GET['id'];
        $res = $db->execute("DELETE FROM users WHERE id = $id;");
        $handleErrors($res);
        echo json_encode($res);
        break;

    default:
        http_response_code(405);
        echo json_encode(['error' => 'Method not allowed']);
        break;
}
