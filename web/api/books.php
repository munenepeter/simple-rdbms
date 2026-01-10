<?php
header('Content-Type: application/json');
require_once __DIR__ . '/../init.php';

//check if $db is available
if (!$db) {
    http_response_code(500);
    echo json_encode(['error' => 'Database connection error']);
    exit;
}

$method = $_SERVER['REQUEST_METHOD'];

switch ($method) {
    case 'GET':
        if (isset($_GET['user_id'])) {
            $userId = (int)$_GET['user_id'];
            $res = $db->execute("SELECT * FROM books WHERE user_id = $userId;");
            $handleErrors($res);
            echo json_encode($res['data'] ?? []);
        } elseif (isset($_GET['id'])) {
            $id = (int)$_GET['id'];
            $res = $db->execute("SELECT * FROM books WHERE id = $id;");
            $handleErrors($res);
            echo json_encode($res['data'][0] ?? null);
        } else {
            $res = $db->execute("SELECT * FROM books;");
            $handleErrors($res);
            echo json_encode($res['data'] ?? []);
        }
        break;

    case 'POST':
        $data = json_decode(file_get_contents('php://input'), true);
        if (!$data || !isset($data['id']) || !isset($data['title']) || !isset($data['author'])) {
            http_response_code(400);
            echo json_encode(['error' => 'Missing required fields']);
            break;
        }
        $id = (int)$data['id'];
        $title = addslashes($data['title']);
        $author = addslashes($data['author']);
        $userId = isset($data['user_id']) ? (int)$data['user_id'] : 0;
        $res = $db->execute("INSERT INTO books VALUES ($id, '$title', '$author', $userId);");
        $handleErrors($res);
        echo json_encode($res);
        break;

    case 'PUT':
        $data = json_decode(file_get_contents('php://input'), true);
        if (!$data || !isset($data['id'])) {
            http_response_code(400);
            echo json_encode(['error' => 'Missing required fields']);
            break;
        }
        $id = (int)$data['id'];
        $title = isset($data['title']) ? addslashes($data['title']) : null;
        $author = isset($data['author']) ? addslashes($data['author']) : null;
        $userId = isset($data['user_id']) ? (int)$data['user_id'] : null;

        $sets = [];
        if ($title !== null) $sets[] = "title = '$title'";
        if ($author !== null) $sets[] = "author = '$author'";
        if ($userId !== null) $sets[] = "user_id = $userId";

        if (empty($sets)) {
            echo json_encode(['success' => true, 'message' => 'No changes']);
            break;
        }

        $sql = "UPDATE books SET " . implode(", ", $sets) . " WHERE id = $id;";
        $res = $db->execute($sql);
        $handleErrors($res);
        echo json_encode($res);
        break;

    case 'DELETE':
        $id = (int)$_GET['id'];
        $res = $db->execute("DELETE FROM books WHERE id = $id;");
        $handleErrors($res);
        echo json_encode($res);
        break;

    default:
        http_response_code(405);
        echo json_encode(['error' => 'Method not allowed']);
        break;
}
