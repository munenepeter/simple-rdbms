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


//this is special, cause we are getting a full html document
// so think allow-forms does not work? 
// to research more later
function showErrorHtmlModal(html) {
    const iframe = document.getElementById('error-iframe');
    if (!iframe) return;

    // ensure sandbox allows forms and top navigation force, coz it is not working with markup in index
    try {
        iframe.setAttribute('sandbox', 'allow-same-origin allow-scripts allow-forms allow-top-navigation-by-user-activation');
    } catch (err) {
        console.warn('Could not set iframe sandbox attribute', err);
    }

    // srcdoc so full HTML renders inside iframe
    iframe.srcdoc = html;

    // fallback: after iframe loads, attach a submit interceptor to migration form (if same-origin)
    iframe.onload = function () {
        try {
            const doc = iframe.contentDocument || iframe.contentWindow.document;
            const form = doc.getElementById('migration-form') || doc.querySelector('form');
            if (form) {
                form.addEventListener('submit', function (e) {
                    e.preventDefault();
                    // notify parent to run migration (parent will perform the POST)
                    window.parent.postMessage({ type: 'run_migration' }, '*');
                });
            }
        } catch (e) {
            // could be cross-origin if allow-same-origin wasn't applied
            console.warn('Could not attach form interceptor inside iframe', e);
        }
    };

    openModal('error-modal');
}

function closeErrorModal() {
    const iframe = document.getElementById('error-iframe');
    if (iframe) iframe.srcdoc = '';
    closeModal('error-modal');
}

// listen for messages from the error iframe (e.g., run_migration)
window.addEventListener('message', async function (e) {
    try {
        if (!e.data || e.data.type !== 'run_migration') return;

        if (!confirm('Run database setup on server?')) return;

        // POST from parent context to avoid sandbox restrictions
        const resp = await fetch('setup_dummy_db.php', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: 'run_migration=1'
        });

        if (!resp.ok) {
            const txt = await resp.text();
            alert('Migration failed: ' + resp.status + '\n' + txt);
            return;
        }

        alert('Migration started successfully; reloading...');
        // close the error modal and reload to pick up the new DB
        closeErrorModal();
        window.location.reload();

    } catch (err) {
        alert('Migration failed: ' + err.message);
    }
});
