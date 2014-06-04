nacl_cryptocore
===============

An encrypt test for bitcore.

This example code loads an NaCl module which uses libopenssl to execute
elliptic curve *add* and *multiply* operations.  It choose the inputs using
Javascript's Math.random() and displays the results.

You need the NaCl SDK to build this code and the Google Chrome browser version
31+ to run it.

See https://developer.chrome.com/native-client/sdk/download

build
=====
    export NACL_SDK_ROOT=<path to nacl sdk>
    cd nacl_cryptocore
    make


usage
=====
With cryptocore.pexe built point your Google Chrome browser at the top
directory.
