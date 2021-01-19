default:
	gcc main.c -Wall -o calculator -lncurses

run:
	./calculator

clean:
	rm calculator

cr: default run
