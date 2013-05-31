#!/usr/bin/evn python
# -*- coding:utf-8 -*-

from socket import *
import commands

HOST = "localhost"
PORT = 4444
ADDR = (HOST, PORT)
BUFLEN = 256
BAKLOG = 10

def client():
    while True:
        clisock = socket(AF_INET, SOCK_STREAM)
        clisock.connect(ADDR)
        cmd = raw_input("\n>>> ")
        clisock.send(cmd)
        while True:
            data = clisock.recv(BUFLEN)
            print data
            if not data:
                break

if __name__ == '__main__':
    client()


