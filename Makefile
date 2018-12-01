all: main.c
	gcc main.c -o main -Wno-pointer-to-int-cast -lm

clean:
	$(RM) main
