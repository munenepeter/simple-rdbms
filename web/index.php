<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="description" content="A simple web application to manage users and their books.">
    <meta name="author" content="Peter Munene">
    <title>Book Management Web App</title>
    <style>
        .section { display: none; }
        .section.active { display: block; }
        .modal { display: none; position: fixed; z-index: 1; left: 0; top: 0; width: 100%; height: 100%; overflow: auto; background-color: rgba(0,0,0,0.4); }
        .modal-content { background-color: #fefefe; margin: 15% auto; padding: 20px; border: 1px solid #888; width: 80%; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        .nav { margin-bottom: 20px; }
    </style>
</head>
<body>
    <h1>Book Management</h1>
    
    <div class="nav">
        <button onclick="showSection('users-section')">Users</button>
        <button onclick="showSection('books-section')">Books</button>
    </div>

    <!-- Users Section -->
    <div id="users-section" class="section active">
        <h2>Users</h2>
        <button onclick="openCreateUserModal()">Create User</button>
        <table id="users-table">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Name</th>
                    <th>Email</th>
                    <th>Actions</th>
                </tr>
            </thead>
            <tbody></tbody>
        </table>
    </div>

    <!-- Books Section -->
    <div id="books-section" class="section">
        <h2>Books</h2>
        <button onclick="openCreateBookModal()">Create Book</button>
        <table id="books-table">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Title</th>
                    <th>Author</th>
                    <th>User ID</th>
                    <th>Actions</th>
                </tr>
            </thead>
            <tbody></tbody>
        </table>
    </div>

    <!-- Related Items Modal (Show books for a user) -->
    <div id="related-modal" class="modal">
        <div class="modal-content">
            <span onclick="closeModal('related-modal')" style="cursor:pointer;float:right;">&times;</span>
            <h3>Books for <span id="related-user-name"></span></h3>
            <div id="related-books-list"></div>
        </div>
    </div>

    <!-- Create/Edit User Modal -->
    <div id="user-modal" class="modal">
        <div class="modal-content">
            <span onclick="closeModal('user-modal')" style="cursor:pointer;float:right;">&times;</span>
            <h3 id="user-modal-title">Create User</h3>
            <form id="user-form">
                <input type="hidden" id="user-id-orig">
                <label>ID: <input type="number" id="user-id" required></label><br><br>
                <label>Name: <input type="text" id="user-name"></label><br><br>
                <label>Email: <input type="email" id="user-email"></label><br><br>
                <button type="submit">Save</button>
            </form>
        </div>
    </div>

    <!-- Create/Edit Book Modal -->
    <div id="book-modal" class="modal">
        <div class="modal-content">
            <span onclick="closeModal('book-modal')" style="cursor:pointer;float:right;">&times;</span>
            <h3 id="book-modal-title">Create Book</h3>
            <form id="book-form">
                <input type="hidden" id="book-id-orig">
                <label>ID: <input type="number" id="book-id"></label><br><br>
                <label>Title: <input type="text" id="book-title"></label><br><br>
                <label>Author: <input type="text" id="book-author"></label><br><br>
                <select name="book-user-id" id="book-user-id">
                    <option value="">Select User</option>
                </select><br><br>
                <button type="submit">Save</button>
            </form>
        </div>
    </div>

    <script src="js/api.js"></script>
    <script src="js/modals.js"></script>
    <script src="js/app.js"></script>
</body>
</html>
