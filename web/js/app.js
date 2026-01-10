async function loadUsers() {
    const users = await api.get('api/users.php');
    const tbody = document.querySelector('#users-table tbody');
    tbody.innerHTML = '';

    if (users.length === 0 || (typeof users === 'object' && 'error' in users)) {
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td colspan="3">No users found. Try Adding a new one</td>
        `;
        tbody.appendChild(tr);
        return;
    }

    users.forEach(user => {
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>${user.id}</td>
            <td>${user.name}</td>
            <td>${user.email}</td>
            <td>
                <button onclick='viewRelatedItems(${JSON.stringify(user)})'>Related Items</button>
                <button onclick='openEditUserModal(${JSON.stringify(user)})'>Edit</button>
                <button onclick='deleteUser(${user.id})'>Delete</button>
            </td>
        `;
        tbody.appendChild(tr);
    });

    //set the user's dropdown in books creation moddal
    const userSelect = document.getElementById('book-user-id');
    userSelect.innerHTML = '';
    users.forEach(user => {
        const option = document.createElement('option');
        option.value = user.id;
        option.innerText = user.name;
        userSelect.appendChild(option);
    });

}

async function loadBooks() {
    const books = await api.get('api/books.php');
    const tbody = document.querySelector('#books-table tbody');
    tbody.innerHTML = '';

    if (books.length === 0) {
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td colspan="5">No books found. Try Adding a new one</td>
        `;
        tbody.appendChild(tr);
        return;
    }


    books.forEach(book => {
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>${book.id}</td>
            <td>${book.title}</td>
            <td>${book.author}</td>
            <td>${book.user_id || 'None'}</td>
            <td>
                <button onclick='openEditBookModal(${JSON.stringify(book)})'>Edit</button>
                <button onclick='deleteBook(${book.id})'>Delete</button>
            </td>
        `;
        tbody.appendChild(tr);
    });
}

async function viewRelatedItems(user) {
    document.getElementById('related-user-name').innerText = user.name;
    const books = await api.get(`api/books.php?user_id=${user.id}`);
    const list = document.getElementById('related-books-list');
    list.innerHTML = '';
    if (books.length === 0) {
        list.innerHTML = '<p>No books associated with this user.</p>';
    } else {
        const ul = document.createElement('ul');
        books.forEach(book => {
            const li = document.createElement('li');
            li.innerText = `${book.title} by ${book.author}`;
            ul.appendChild(li);
        });
        list.appendChild(ul);
    }
    openModal('related-modal');
}

document.getElementById('user-form').onsubmit = async (e) => {
    e.preventDefault();
    const idOrig = document.getElementById('user-id-orig').value;
    const data = {
        id: document.getElementById('user-id').value,
        name: document.getElementById('user-name').value,
        email: document.getElementById('user-email').value
    };

    if (idOrig) {
        await api.put('api/users.php', data);
    } else {
        await api.post('api/users.php', data);
    }
    closeModal('user-modal');
    loadUsers();
};

document.getElementById('book-form').onsubmit = async (e) => {
    e.preventDefault();
    const idOrig = document.getElementById('book-id-orig').value;
    const data = {
        id: document.getElementById('book-id').value,
        title: document.getElementById('book-title').value,
        author: document.getElementById('book-author').value,
        user_id: document.getElementById('book-user-id').value
    };

    if (idOrig) {
        await api.put('api/books.php', data);
    } else {
        await api.post('api/books.php', data);
    }
    closeModal('book-modal');
    loadBooks();
};

async function deleteUser(id) {
    if (confirm('Are you sure?')) {
        await api.delete(`api/users.php?id=${id}`);
        loadUsers();
    }
}

async function deleteBook(id) {
    if (confirm('Are you sure?')) {
        await api.delete(`api/books.php?id=${id}`);
        loadBooks();
    }
}

// init
loadUsers();
