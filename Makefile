S=source
I=include

semaphore:
	gcc -o semaphore $S/semaphore.c -lm -I$I

message:
	gcc -o message $S/message.c -lm -I$I

clean:
	rm semaphore
	rm message