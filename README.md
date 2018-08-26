blockchain
===========

Who wrote it ?
--------------

    Author:

        uladzimir.marushka@yahoo.com

Canonical source code repo:
---------------------------

    git clone github.com:umarushka/blockchain.git

License:
--------

    Code is in the public domain.

What is it ?
------------

    A barebone C++ block parser that parses the entire block chain from scratch
    to extract various types of information from it.

How do I build it ?
-------------------

    You'll need a 64-bit Unix box.

    You'll need a block chain somewhere on your hard drive. This is typically created
    by a statoshi bitcoin client such as this one: https://github.com/bitcoin/bitcoin.git

Install dependencies:

        sudo apt-get install libssl-dev build-essential g++ libboost-all-dev libsparsehash-dev git-core perl

Get the source:

        git clone git://github.com/umarushka/blockchain.git

    Build it:

        cd blockchain
        make        
