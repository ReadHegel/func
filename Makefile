all: harrness test test_clear

harrness: funclist.h 
	clang++ -Wall -Wextra -std=c++23 -O2 harrness.cpp -o harrness

test: funclist_test_1.cpp funclist.h
	clang++ -Wall -Wextra -std=c++23 -O2 funclist_test_1.cpp -o test

test_clear: funclist_test_clear.cpp funclist.h
	clang++ -Wall -Wextra -std=c++23 -O2 funclist_test_clear.cpp -o test_clear

run: test test2
	./test
	./test_clear

clean:
	rm -f test test_clear harrness
