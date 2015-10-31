
CC = g++

bin/shape: bin/libshape.so
	$(CC) -Wall -o $@ obj/main.o -lshape

.PHONY: bin/libshape.so
bin/libshape.so: obj/shape.o obj/main.o
	@mkdir -p bin
	$(CC) -shared -o $@ $<

.PHONY: obj/shape.o
obj/shape.o: src/shape.cpp
	@mkdir -p obj
	$(CC) -c -o $@ $< -Wall -Werror -fpic

.PHONY: obj/main.o
obj/main.o: src/main.cpp
	@mkdir -p obj
	$(CC) -c -o $@ $< -Wall -Werror -fpic

.PHONY: install
install:
	@ln -f include/*.* /usr/local/include
	@ln -f bin/libshape.so /usr/local/lib

.PHONY: clean
clean:
	@rm -rf bin/*
	@rm -rf obj/*