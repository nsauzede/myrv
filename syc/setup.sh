#!/bin/bash

if [ ! -d .ut ]; then
if `command -v ut >& /dev/null`; then
ut --nopy init
else
git clone git@github.com:nsauzede/.ut.git
fi
fi

mkdir -p .ut/.3p
cd .ut/.3p

if [ ! -d googletest ]; then
time {
git clone https://github.com/google/googletest.git && \
(cd googletest ; mkdir build ; cd build ; cmake .. ; make)
}
fi

if [ ! -d systemc-3.0.1 ]; then
time {
if [ ! -f 3.0.1.tar.gz ]; then
wget https://github.com/accellera-official/systemc/archive/refs/tags/3.0.1.tar.gz
fi
tar xf 3.0.1.tar.gz && \
(cd systemc-3.0.1 ; touch docs/DEVELOPMENT.md ; ./configure --prefix=`pwd`/the_install && make && make install) && \
rm -f 3.0.1.tar.gz
}
fi
