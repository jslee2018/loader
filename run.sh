#!/bin/bash

cd /home/workspace/src/apager
make
rm -f *.o
mv ./loader ../../apager

cd ../../tests
make
cd ..
ls -1 ./tests/build | while read line; do
    echo test apager with $line
    ./apager ./tests/build/$line
done