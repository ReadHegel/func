all: 
	clang++ -Wall -Wextra -std=c++23 -O2 harrness.cpp

test: 
	clang++ -Wall -Wextra -std=c++23 -O2 funclist_test_1.cpp
