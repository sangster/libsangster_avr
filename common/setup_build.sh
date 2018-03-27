#!/bin/sh
set -e

autoreconf -fvi
exec ./configure --build=$(./config.guess) --host=avr
