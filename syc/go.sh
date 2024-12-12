#!/bin/bash

./setup.sh
for p in `ls`; do
    ([ -d $p ] && cd $p && ../test.sh && ../run.sh)
done
