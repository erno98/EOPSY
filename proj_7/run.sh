#!/bin/sh

gcc main.c -o main -lpthread

chmod +x main

./main

rm main

