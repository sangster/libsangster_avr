#!/bin/sh
set -e

autoreconf -vi
exec ./configure --host=avr
