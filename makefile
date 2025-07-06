build:
	gcc -Wall -Wextra -O2 -std=c11 -lm ./*.c -o bin/pinky

clean:
	rm pinky
