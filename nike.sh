#!/bin/bash

python3 osmaker.py $@ | ./bin/pleb | ./assembler.pl
