set -xe
FLAGS="-Wall -Wextra -g -pedantic -lncurses"
cc src/*.c -o main $FLAGS