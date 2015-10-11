
#include "../include/shape.h"

int main(int argv, char** argc)
{
	for (int i = 1; i < argv; i++)
	{
		FILE* file = 0;
		fopen_s(&file, argc[i], "r");
		shape* shape = new ::shape;
		shape->read(file);
		shape->release();
		delete shape;
		fclose(file);
	}

	return 0;
}