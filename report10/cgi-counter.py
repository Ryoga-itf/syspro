#!/usr/local3/coins/linux/bin/python3
# -*- coding: utf-8 -*-

import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)

import cgi
import bleach
import sys

DATA_FILENAME = "file-counter-value.data"

def main():
    print_header()
    counter = handle_counter()
    print_content(counter)
    exit(0)

def print_header():
    print("Content-Type: text/html")
    print("")

def handle_counter():
    counter = counter_load()
    qh = cgi.FieldStorage(keep_blank_values=True)
    op = qh.getfirst("op","")
    if op == "inc":
        counter += 1
        counter_save(counter)
    elif op == "set":
        val = int(qh.getfirst("val",""))
        counter = val
        counter_save(counter)
    return counter
   
def print_content(counter):
    print("<HTML><HEAD></HEAD><BODY><PRE>",flush=True)
    print(counter)
    print("</PRE></BODY></HTML>\n")

def counter_load():
    counter = 0
    try:
        with open(DATA_FILENAME, "rb") as f:
            counter_bytes = f.read(4)
            counter = int.from_bytes(counter_bytes, byteorder=sys.byteorder)
    except FileNotFoundError:
        pass
    return counter

def counter_save(counter):
    counter_bytes = counter.to_bytes(4, byteorder=sys.byteorder)
    with open(DATA_FILENAME, "wb") as f:
        f.write(counter_bytes)
    return

main()
