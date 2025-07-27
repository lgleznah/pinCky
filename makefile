build:
	gcc -Wall -Wextra -O2 -std=c11 ./*.c -o bin/pinky -lm

build_debug:
	gcc -Wall -Wextra -O2 -std=c11 -g ./*.c -o bin/pinky -lm

clean:
	rm pinky
