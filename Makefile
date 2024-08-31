all:
	gcc main.c -o autorenamer -Wall -Wextra -g

clean:
	rm -f autorenamer
