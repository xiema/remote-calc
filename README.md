# Remote Calculator

A client-server calculator application. The client sends a mathematical expression composed of integers, operators and parentheses to the server, which the server evaluates, returning the value to the client. Created as an exercise in C development.

# Building

Using make

    make app

To do unit tests

    make test

Currently compatible only on Windows systems (tested on Windows 10).

# Usage

Run as server

    ./app.exe server

Connect a client to a server

    ./app.exe client <IP ADDRESS>

Can handle addition, subtraction, multiplication and division of positive integers, with nested parentheses. Spaces are ignored.

    1 + (4 / (1 + 1)) * 2 - 1
    Received: 4
    9/2
    Received: 4

## References:

WinSock: [Tutorial](https://www.binarytides.com/winsock-socket-programming-tutorial/)

# To do:
> Make Linux-compatible
> Improve documentation

