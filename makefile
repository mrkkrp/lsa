.PHONY : clear

build/lsa : src/main.o src/analyze.o
	gcc -msse -msse2 -laudiofile -lpthread -lm -o build/lsa \
	build/main.o build/analyze.o

src/main.o :
	mkdir -p build
	gcc -c -o build/main.o src/main.c

src/analyze.o :
	mkdir -p build
	gcc -c -o build/analyze.o src/analyze.c

clear :
	rm -vr build
