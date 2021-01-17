default:
	gcc main.c -o calculator -lncurses

run:
	./calculator

clean:
	rm calculator

cr: default run
