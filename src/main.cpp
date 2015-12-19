
#define _CRT_SECURE_NO_WARNINGS
#include "../include/shape.h"

int main(int argv, char** argc)
{
	printf("Running libshape\n");
	shape_init();
	for (int i = 1; i < argv; i++)
	{
		printf("Reading: %s\n", argc[i]);
		shapeobj_t shape;
		FILE* file = fopen(argc[i], "r");
		shape_read(&shape, shape_file_extension(argc[i]), file);
		printf("Mesh type: %s\n", shape_type_extension(shape.type));
		printf("Faces: %d\n", shape.faceIndices.elements);
		printf("Vertices: %d\n", shape.vertices.elements);
		printf("Texcoords: %d\n", shape.texcoords.elements);
		printf("Normals: %d\n", shape.normals.elements);
		printf("Tangents: %d\n", shape.tangents.elements);
		printf("Binormals: %d\n", shape.binormals.elements);
		for (int32_t i = 0; i < shape.vertices.elements; i++)
		{
			printf("   v: %4.4f, %4.4f, %4.4f\n", shape.vertices[i].x, shape.vertices[i].y, shape.vertices[i].z);
		}
		
		for (int32_t i = 0; i < shape.texcoords.elements; i++)
		{
			printf("  vt: %4.4f, %4.4f\n", shape.texcoords[i].x, shape.texcoords[i].y);
		}
		
		for (int32_t i = 0; i < shape.normals.elements; i++)
		{
			printf("  vn: %4.4f, %4.4f, %4.4f\n", shape.normals[i].x, shape.normals[i].y, shape.normals[i].z);
		}
		
		for (int32_t i = 0; i < shape.tangents.elements; i++)
		{
			printf("  tn: %4.4f, %4.4f, %4.4f\n", shape.tangents[i].x, shape.tangents[i].y, shape.tangents[i].z);
		}
		
		for (int32_t i = 0; i < shape.binormals.elements; i++)
		{
			printf("  bn: %4.4f, %4.4f, %4.4f\n", shape.binormals[i].x, shape.binormals[i].y, shape.binormals[i].z);
		}
		
		for (int32_t i = 0; i < shape.indices.elements; i++)
		{
			printf("  f[%4d]: %d, %d, %d\n", i, shape.faceIndices[i].x, shape.faceIndices[i].y, shape.faceIndices[i].z);
		}
		
		fclose(file);
		file = fopen("test.obj", "w");
		shape_write_wavefront(&shape, file);
		fclose(file);
		file = fopen("test.dae", "w");
		shape_write_collada(&shape, file);
		fclose(file);
		shape_free(&shape);
	}
	
	shape_release();
	return 0;
}