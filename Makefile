mu:
	clear
	gcc main.c -o main.o -std=c99
	./main.o Voice001.wav
test:
	clear
	gcc test.c -o test.o
	./test.o sample.wav
testv:
	clear
	gcc test.c -o test.o
	./test.o Voice001.wav

clean:
	rm main.o
