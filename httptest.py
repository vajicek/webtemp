#!/usr/bin/python

"""Run test in www folder."""


import SimpleHTTPServer
import SocketServer
import os

os.chdir('www')
SocketServer.TCPServer(("", 8080), SimpleHTTPServer.SimpleHTTPRequestHandler).serve_forever()
