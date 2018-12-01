all: main.c
	gcc main.c -o fbp -Wno-pointer-to-int-cast -lm -lcjson

clean:
	$(RM) main
