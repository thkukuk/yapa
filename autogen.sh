#!/bin/sh -x

rm -rf autom4te.cache aclocal.m4
aclocal
autoheader
automake --add-missing --copy --force
autoreconf
chmod 755 configure
