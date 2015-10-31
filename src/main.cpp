
#include "../include/shape.h"

int main(int argv, char** argc)
{
	printf("Running libshape\n");
	for (int i = 1; i < argv; i++)
	{
		printf("Reading: %s\n", argc[i]);
		FILE* file = fopen(argc[i], "r");
		shape* shape = new ::shape;
		shape->read(file);
		printf("Faces: %d\n", shape->faceIndices.elements);
		printf("Vertices: %d\n", shape->vertices.elements);
		printf("Texcoords: %d\n", shape->texcoords.elements);
		printf("Normals: %d\n", shape->normals.elements);
		printf("Tangents: %d\n", shape->tangents.elements);
		printf("Binormals: %d\n", shape->binormals.elements);
		fclose(file);
		shape->clear();
		delete shape;
	}

	return 0;
}