s: build run

.PHONY: build
build:
	clang++ main.cpp -o main -std=c++23

.PHONY: run
run:
	./main