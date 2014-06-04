/******************************************************************************\

                    Copyright 2014. Cauldron Development LLC
                              All Rights Reserved.

        This software is free software: you can redistribute it and/or
        modify it under the terms of the GNU Lesser General Public License
        as published by the Free Software Foundation, either version 2.1 of
        the License, or (at your option) any later version.

        This software is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
        Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public
        License along with the C! library.  If not, see
        <http://www.gnu.org/licenses/>.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/

// Utility functions ***********************************************************
function debug() {
    if (typeof console == 'undefined' || typeof console.log == 'undefined')
        return;

    var msg = [];
    for (var i = 0; i < arguments.length; i++) {
        var item = arguments[i];

        if (typeof item !== 'string' && typeof JSON !== 'undefined')
            msg.push(JSON.stringify(item));
        else msg.push(item);
    }

    console.log('DEBUG: ' + msg.join(' '));
}


function byte2hex(x) {
    var hex = Number(x).toString(16);
    if (hex.length < 2) hex = '0' + hex;
    return hex;
}


function ab2hex(buf) {
    var s = "";
    var view = new Uint8Array(buf);

    for (var i = 0; i < view.length; i++)
        s += byte2hex(view[i]);

    return '0x' + s;
}


// CryptoCore ******************************************************************
function CryptoCore(id, on_loaded) {
    this.test = on_loaded;
    this.id = id;
    this.on_loaded = on_loaded;

    var self = this;
    var div = $('#' + this.id).get(0);
    div.addEventListener('loadstart', function () {self.loading();}, true);
    div.addEventListener('progress', function (e) {self.progress(e);}, true);
    div.addEventListener('load', function () {self.loaded();}, true);
    div.addEventListener('message', function (e) {self.message(e);}, true);
    div.addEventListener('error', function (e) {self.error(e);}, true);
    div.addEventListener('crash', function () {self.crash();}, true);

    $(div).html($('<embed>').attr({
        id: this.id + '-embed',
        src: 'cryptocore.nmf',
        type: 'application/x-pnacl'
    }));
}


CryptoCore.prototype.loading = function() {
    debug("NaCl module loading");
    $('#loading').text('Loading NaCl module');
    return false;
}


CryptoCore.prototype.loaded = function() {
    debug("NaCl module loaded");

    $('#loading').text('Done loading NaCl module');

    this.module = $('#' + this.id + '-embed').get(0);

    if (typeof this.on_loaded != 'undefined') this.on_loaded(this);

    return false;
}


CryptoCore.prototype.progress = function(event) {
    var default_total = 5000000;
    var total = event.total ? event.total : default_total;
    if (default_total * 100 < total ) total = default_total // Sanity check

    var percent = (event.loaded / total * 100.0).toFixed(1);

    $('#loading').text('Loading NaCl module ' + percent + '%');
    return false;
}


CryptoCore.prototype.message = function(event) {
    var cmd = (typeof event.data == 'string') ? event.data : event.data[0];

    switch (true) {
    case cmd == 'add_result':
        this.add_cb(event.data[1]);
        break;

    case cmd == 'multiply_result':
        this.multiply_cb(event.data[1]);
        break;

    default:
        debug(event.data);
        break;
    }

    return false;
}


CryptoCore.prototype.error = function(event) {
    debug('module error: ', event);
    return false;
}


CryptoCore.prototype.crash = function() {
    debug("NaCl module crashed");
    this.module = undefined;
    return false;
}


CryptoCore.prototype.post = function(msg) {
    if (typeof this.module != 'undefined') this.module.postMessage(msg);
}


CryptoCore.prototype.add = function(point0, point1, cb) {
    this.add_cb = cb;
    this.post(['add', point0, point1]);
}


CryptoCore.prototype.multiply = function(point0, x, cb) {
    this.multiply_cb = cb;
    this.post(['multiply', point0, x]);
}


function random_array_buffer(size) {
    var buf = new ArrayBuffer(size);
    var view = new Uint8Array(buf);

    for (var i = 0; i < size; i++)
        view[i] = Math.floor(256 * Math.random());

    return buf;
}


(function() {
    var cryptocore = new CryptoCore('cryptocore', function () {
        var point0 = random_array_buffer(65);
        var point1 = random_array_buffer(65);
        var x = random_array_buffer(32);

        $('#add')
            .text('add(' + ab2hex(point0) + ', ' + ab2hex(point1) + ') = ')

        $('#multiply')
            .text('multiply(' + ab2hex(point0) + ', ' + ab2hex(x) + ') = ')

        cryptocore.add(point0, point1, function (result) {
            $('#add').append(ab2hex(result));
        });

        cryptocore.multiply(point0, x, function (result) {
            $('#multiply').append(ab2hex(result));
        });
    });
})();
