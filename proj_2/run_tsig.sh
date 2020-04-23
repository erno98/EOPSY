#!/bin/sh

gcc tsig.c -o tsig

chmod +x tsig

./tsig

rm tsig
