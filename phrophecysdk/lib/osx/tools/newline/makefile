newline : newline.o
	gcc -o newline -w newline.o

newline.o : newline.c
	gcc -g -c -w newline.c

install:
	cp -f newline /usr/bin

.PHONY : clean

clean:
	-rm core newline *.o *~ 2> /dev/null
