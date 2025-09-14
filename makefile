build:
	gcc -Wall -Wextra -O2 -std=c11 ./*.c -o bin/pinky -lm

debug:
	gcc -Wall -Wextra -O1 -std=c11 -g ./*.c -o bin/pinky -lm

clean:
	rm pinky
