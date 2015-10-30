
bin/libshape: src/shape.cpp
	@mkdir -p bin
	@mkdir -p obj
	@echo "Compiling"
	g++ -o $@ $^

.PHONY: clean
clean:
	@echo "Cleaning"
	@rm -rf obj/*
	@rm -rf bin/*
