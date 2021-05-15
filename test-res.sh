# silently remove old file to prevent execution on failed compilation
rm -f pizza.out

# compile
gcc -o pizza.out -pthread -Wall -Wextra -Werror p3190106-p3190205-pizza.c

# optionally use commandline arguments
if [ -z "$1" ]
then
	# use default number of customers
	./pizza.out 100 1000
else
	# use user-provided number of customers
	./pizza.out $1 1000
fi
