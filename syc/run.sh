#!/bin/bash

SCRIPT=$0
SCRIPTPATH=$(realpath `readlink ${SCRIPT} || echo ${SCRIPT}`)
ROOT=`dirname ${SCRIPTPATH}`

SYCR=${ROOT}/.ut/.3p/systemc-3.0.1
SYC=${SYCR}/the_install
SYCI=${SYC}/include
SYCL=${SYC}`cat ${SYCR}/config.log|grep "Libraries"|cut -d">" -f2`
VG="valgrind"
if [ `command -v $VG` ]; then
VGO="-q --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --exit-on-first-error=yes --error-exitcode=128 --suppressions=${ROOT}/ut_systemc.supp --gen-suppressions=all"
else
VG=""
fi
for t in `find . -name sc_main.cc`; do
g++ -pipe -O0 -g $t -o ${t}.exe -I${SYCI} -L${SYCL} -lsystemc && \
    LD_LIBRARY_PATH=${SYCL} ${VG} ${VGO} ${t}.exe && ([ "x$KEEP" != "x1" ] && rm -f ${t}.exe || true)
done
