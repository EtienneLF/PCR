all:	 TestMessage TestRedirection TestLectureEcriture terminal test validation

message.o: message.c message.h
	gcc -Wall -c message.c

alea.o: alea.h alea.c
	gcc -Wall -c alea.c

terminal.o: terminal.c
	gcc -Wall -c terminal.c

validation.o: validation.c 
	gcc -Wall -c validation.c

TestMessage: message.o alea.o TestMessage.c
	gcc -Wall TestMessage.c message.o alea.o -o  TestMessage

TestRedirection: TestRedirection.c
	gcc -Wall TestRedirection.c -o  TestRedirection

lectureEcriture.o: lectureEcriture.c lectureEcriture.h
	gcc -c -Wall lectureEcriture.c

TestLectureEcriture: lectureEcriture.o TestLectureEcriture.c
	gcc lectureEcriture.o TestLectureEcriture.c -o TestLectureEcriture

terminal: message.o alea.o terminal.o alea.o lectureEcriture.o
	gcc terminal.c message.o alea.o lectureEcriture.o -o  terminal

validation: validation.o
	gcc validation.o -o validation

test: message.o alea.o lectureEcriture.o test.c
	gcc -Wall test.c message.o alea.o lectureEcriture.o -o test

clean:	
	rm -f *.o *~ 

cleanall: clean
	rm TestRedirection TestMessage TestLectureEcriture terminal test
