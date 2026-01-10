const api = {
    async _handleErrorResponse(response) {
        const ct = (response.headers.get('Content-Type') || '').toLowerCase();
        const text = await response.text();
        if (ct.includes('text/html') || text.startsWith('<!DOCTYPE html>')) {
            // show server HTML error in modal, sent by my error handler
            if (window.showErrorHtmlModal) window.showErrorHtmlModal(text);
            throw new Error(`Server returned HTML error (status: ${response.status})`);
        }
        // try JSON
        try {
            const json = JSON.parse(text || '{}');
            const msg = json.error || json.message || text;
            throw new Error(msg || `Network request failed with status: ${response.status}`);
        } catch (e) {
            throw new Error(text || `Network request failed with status: ${response.status}`);
        }
    },

    async get(url) {
        const response = await fetch(url, { headers: { 'X-Requested-With': 'XMLHttpRequest', 'Accept': 'application/json, text/html' } });
        if (!response.ok) {
            await this._handleErrorResponse(response);
        }
        const data = await response.json().catch(() => {
            throw new Error('Invalid JSON response');
        });

        return data;
    },

    async post(url, data) {
        const response = await fetch(url, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json', 'X-Requested-With': 'XMLHttpRequest', 'Accept': 'application/json, text/html' },
            body: JSON.stringify(data)
        });
        if (!response.ok) {
            await this._handleErrorResponse(response);
        }
        return response.json();
    },

    async put(url, data) {
        const response = await fetch(url, {
            method: 'PUT',
            headers: { 'Content-Type': 'application/json', 'X-Requested-With': 'XMLHttpRequest', 'Accept': 'application/json, text/html' },
            body: JSON.stringify(data)
        });
        if (!response.ok) {
            await this._handleErrorResponse(response);
        }
        return response.json();
    },

    async delete(url) {
        const response = await fetch(url, {
            method: 'DELETE',
            headers: { 'X-Requested-With': 'XMLHttpRequest', 'Accept': 'application/json, text/html' }
        });
        if (!response.ok) {
            await this._handleErrorResponse(response);
        }
        return response.json();
    }
};