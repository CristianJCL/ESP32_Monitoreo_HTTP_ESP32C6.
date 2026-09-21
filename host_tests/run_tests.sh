#!/bin/sh
set -eu

cc -std=c11 -Wall -Wextra -Werror \
  -I../main \
  test_monitor_logic.c ../main/monitor_logic.c \
  -o test_monitor_logic

./test_monitor_logic
rm -f test_monitor_logic
