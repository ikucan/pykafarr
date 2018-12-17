all: clean read_test write_test

INCL=-I ../../src/cpp
LIB_DIRS=
LIBS=-lm -ldl -lpthread -lrt -lstdc++ -lserdes++ -lserdes -lrdkafka++ -lrdkafka -lavrocpp -lavro -larrow

read_test:
	echo $(INCL)
	g++ read_test.cpp -fPIC --std=c++17 $(INCL) $(LIB_DIRS) $(LIBS) -o read_test

write_test:
	echo $(INCL)
	g++ write_test.cpp -fPIC --std=c++17 $(INCL) $(LIB_DIRS) $(LIBS) -o write_test

clean:
	rm -f read_test write_test
	rm -f a.out 
	rm -f *~
	rm -f *.o
	rm -f "#*#"

