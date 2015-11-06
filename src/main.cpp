
#include "../include/shape.h"

int main(int argv, char** argc)
{
	printf("Running libshape\n");
	shape_init();
	for (int i = 1; i < argv; i++)
	{
		printf("Reading: %s\n", argc[i]);
		FILE* file = fopen(argc[i], "r");
		shapeobj shape;
		shape_read(&shape, shape_file_extension(argc[i]), file);
		printf("Mesh type: %s\n", shape_type_extension(shape.type));
		printf("Faces: %d\n", shape.faceIndices.elements);
		printf("Vertices: %d\n", shape.vertices.elements);
		printf("Texcoords: %d\n", shape.texcoords.elements);
		printf("Normals: %d\n", shape.normals.elements);
		printf("Tangents: %d\n", shape.tangents.elements);
		printf("Binormals: %d\n", shape.binormals.elements);
		shape_free(&shape);
		fclose(file);
	}
	
	shape_release();
	return 0;
}