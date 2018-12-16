all: clean tst2

INCL=
LIB_DIRS=
LIBS=-lm -ldl -lpthread -lrt -lstdc++ -lserdes++ -lserdes -lrdkafka++ -lrdkafka -lavrocpp -lavro -larrow

tst2:
	echo $(INCL)
	g++ tst2.cpp -fPIC --std=c++17 $(INCL) $(LIB_DIRS) $(LIBS) -o tst2

#c++ --std=c++17 $(INCL) tst2.cpp -c -o tst2.o

clean:
	rm -f tst tst2
	rm -f a.out 
	rm -f *~
	rm -f *.o
	rm -f "#*#"

