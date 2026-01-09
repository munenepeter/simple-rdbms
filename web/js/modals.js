function showSection(sectionId) {
    document.querySelectorAll('.section').forEach(s => s.classList.remove('active'));
    document.getElementById(sectionId).classList.add('active');
    if (sectionId === 'users-section') loadUsers();
    if (sectionId === 'books-section') loadBooks();
}

function openModal(modalId) {
    document.getElementById(modalId).style.display = 'block';
}

function closeModal(modalId) {
    document.getElementById(modalId).style.display = 'none';
}

// global click to close modals
window.onclick = function (event) {
    if (event.target.classList.contains('modal')) {
        event.target.style.display = 'none';
    }
}

function openCreateUserModal() {
    document.getElementById('user-modal-title').innerText = 'Create User';
    document.getElementById('user-id').disabled = false;
    document.getElementById('user-form').reset();
    document.getElementById('user-id-orig').value = '';
    openModal('user-modal');
}

function openEditUserModal(user) {
    document.getElementById('user-modal-title').innerText = 'Edit User';
    document.getElementById('user-id').value = user.id;
    document.getElementById('user-id').disabled = true;
    document.getElementById('user-name').value = user.name;
    document.getElementById('user-id-orig').value = user.id;
    openModal('user-modal');
}

function openCreateBookModal() {
    document.getElementById('book-modal-title').innerText = 'Create Book';
    document.getElementById('book-id').disabled = false;
    document.getElementById('book-form').reset();
    document.getElementById('book-id-orig').value = '';
    openModal('book-modal');
}

function openEditBookModal(book) {
    document.getElementById('book-modal-title').innerText = 'Edit Book';
    document.getElementById('book-id').value = book.id;
    document.getElementById('book-id').disabled = true;
    document.getElementById('book-title').value = book.title;
    document.getElementById('book-author').value = book.author;
    document.getElementById('book-user-id').value = book.user_id;
    document.getElementById('book-id-orig').value = book.id;
    openModal('book-modal');
}
