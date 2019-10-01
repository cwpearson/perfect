#! /bin/bash

while read i; do
  echo $i;
  echo $i > /dev/cpuset/tasks;
done < /dev/cpuset/unshielded/tasks

while read i; do
  echo $i;
  echo $i > /dev/cpuset/tasks;
done < /dev/cpuset/shielded/tasks

rmdir /dev/cpuset/shielded
rmdir /dev/cpuset/unshielded