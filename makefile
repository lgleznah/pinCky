build:
	gcc -Wall -Wextra -O2 -std=c11 -lm ./*.c -o bin/pinky

build_debug:
	gcc -Wall -Wextra -O2 -std=c11 -lm -g ./*.c -o bin/pinky

clean:
	rm pinky
