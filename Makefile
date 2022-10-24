
S=source
I=include

semaphore:
	gcc -o lab_06_sema $S/lab_06_sema.c -lm -I$I

message:
	gcc -o lab_06_msg $S/lab_06_msg.c -lm -I$I

clean:
	rm lab_06_sema
	rm lab_06_msg