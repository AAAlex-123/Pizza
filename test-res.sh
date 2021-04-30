rm pizza.out
gcc -o pizza.out -pthread -Wall -Wextra -Werror p3190106-p3190205.c

if [ -z "$1" ]
then
	./pizza.out 100 1000
else
	./pizza.out $1 1000
fi
