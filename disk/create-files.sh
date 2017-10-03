#!/bin/bash

touch file.in
dd if=/dev/urandom iflag=fullblock of=file.in bs=1G count=10

touch file.out
cp file.in file.out
