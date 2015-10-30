
#include "../include/shape.h"

int main(int argv, char** argc)
{
	printf("Running libshape\n");
	for (int i = 1; i < argv; i++)
	{
		printf("arg: %s\n", argc[i]);
		FILE* file = fopen(argc[i], "r");
		shape* shape = new ::shape;
		shape->read(file);
		fclose(file);
		shape->clear();
		delete shape;
	}

	return 0;
}