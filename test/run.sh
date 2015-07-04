#!/bin/bash

cpp_file=$1
out_file=${cpp_file%.*}
out_file=$(tempfile)
g++ $1 -I. -std=c++0x -o $out_file

$out_file

rm $out_file