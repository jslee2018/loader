#!/bin/bash

cd /home/workspace/src
make
rm -f *.o
mv ./loader ../loader

cd ../tests
make
cd ..
ls -1 ./tests/build | while read line; do
    echo start $line
    ./loader ./tests/build/$line
done