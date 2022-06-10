This project is an assignment I was given in one of my courses

- It performs file transfer through a relay server

- Files are encoded before sending with (31, 26) hamming code, and decoded by the receiver

`FileTransferChannel` - acts as a relay / middleman server for transferring files

`FileTransferSender` - connects to Channel, then sends files as prompted by user

`FileTransferReceiver` - connects to Channel, then receives files as sent from Channel

`FileTransferCommon` - contains common utilities, types, and methods

Channel can flip bits randomly if prompted (can be configured in CLI)

31,26 hamming encoding is used for error correction (no total parity bit)

It utilizes WinSock2.h for socket connectivity, due to course requirements, but `FileTransferCommon/socket_utils.c` can be modified easily to support socket.h

`relay.py` is a Python implementation of `FileTransferChannel` without error injection, used for debugging
