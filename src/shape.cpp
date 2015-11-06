
#include "../include/shape.h"

using namespace glm;

SHAPEAPI void shape_init()
{
	
}
SHAPEAPI void shape_release()
{
	
}

SHAPEAPI void shape_free(shapeobj* shape)
{
	if (shape != 0 && !shape->empty())
	{
		free(shape->buffer);
		*shape = shapeobj();
	}
}

SHAPEAPI void shape_read(shapeobj* shape, const shape_type type, FILE* file)
{
	if (shape == 0 || file == 0)
	{
		return;
	}

#if 0
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
#else
	fseek(file, 0, SEEK_SET);
	int size = 0;
	while (fgetc(file) != EOF)
	{
		size++;
		if (feof(file) != 0)
		{
			break;
		}
	}

#endif
	char* raw = new char[size + 1];
	memset(raw, 0, size + 1);
	fseek(file, 0, SEEK_SET);
	fread(raw, 1, size, file);
	shape_read(shape, type, raw);
	delete[] raw;
}
SHAPEAPI void shape_read(shapeobj* shape, const shape_type type, const char* buffer)
{
	if (shape != 0 && type != SHAPE_TYPE_UNKNOWN && buffer != 0)
	{
		switch (type)
		{
		case SHAPE_TYPE_WAVEFRONT:
			shape_read_wavefront(shape, buffer);
			break;
		default:
			break;
		}
	}
}
SHAPEAPI void shape_write(shapeobj* shape, const shape_type type, FILE* file)
{
	if (shape != 0 && !shape->empty() && type != SHAPE_TYPE_UNKNOWN && file != 0)
	{
		switch (type)
		{
		case SHAPE_TYPE_WAVEFRONT:
			shape_write_wavefront(shape, file);
			break;
		default:
			break;
		}
	}
}

SHAPEAPI void shape_read_wavefront(shapeobj* shape, const char* buffer)
{
	if (shape == 0 || buffer == 0)
	{
		return;
	}
	
	int vertices = 0;
	int texcoords = 0;
	int normals = 0;
	int faces = 0;
	for (const char* read = buffer; read != '\0'; read = strchr(read, '\n'))
	{
		char first = *read;
		if (first == '\n')
		{
			read++;
			first = *read;
		}

		if (first == 'v')
		{
			read++;
			char second = *read;
			if (second == 'n')
			{
				normals++;
			}
			else if (second == 't')
			{
				texcoords++;
			}
			else
			{
				vertices++;
			}
		}
		else if (first == 'f')
		{
			faces++;
		}
	}

	if (faces < 1 || vertices < 1 || texcoords < 1 || normals < 1)
	{
		return;
	}

	int elements = max(vertices, texcoords);
	*shape = shapeobj(faces, elements);
	shape->size = (faces * sizeof(shapeobj::indexType) * 3) + (elements * sizeof(shapeobj::valueType) * 15);
	shape->buffer = malloc(shape->size);
	shape->type = SHAPE_TYPE_WAVEFRONT;
	memset(shape->buffer, 0, shape->size);
	shape->vertices = shape_component<shapeobj::vertexType>((shapeobj::vertexType*)shape->buffer, elements, 0);
	shape->texcoords = shape_component<shapeobj::texcoordType>(
		(shapeobj::texcoordType*)(((shapeobj::valueType*)shape->buffer) + (elements * 4)),
		elements,
		elements * 4 * sizeof(shapeobj::valueType));
	shape->normals = shape_component<shapeobj::normalType>(
		(shapeobj::normalType*)(((shapeobj::valueType*)shape->buffer) + (elements * 6)),
		elements,
		elements * 6 * sizeof(shapeobj::valueType));
	shape->tangents = shape_component<shapeobj::tangentType>(
		(shapeobj::tangentType*)(((shapeobj::valueType*)shape->buffer) + (elements * 9)),
		elements,
		elements * 9 * sizeof(shapeobj::valueType));
	shape->binormals = shape_component<shapeobj::binormalType>(
		(shapeobj::binormalType*)(((shapeobj::valueType*)shape->buffer) + (elements * 12)),
		elements,
		elements * 12 * sizeof(shapeobj::valueType));
	shape->faceIndices = shape_component<shapeobj::faceType>(
		(shapeobj::faceType*)(((shapeobj::valueType*)shape->buffer) + (elements * 15)),
		faces,
		elements * 15 * sizeof(shapeobj::valueType));
	shape->indices = shape_component<shapeobj::indexType>(
		(shapeobj::indexType*)shape->faceIndices.buffer,
		faces * 3,
		shape->faceIndices.offset);

	std::vector<vec3> rawVertices(vertices);
	std::vector<vec2> rawTexcoords(texcoords);
	std::vector<vec3> rawNormals(normals);
	bool orderByVertices = texcoords <= vertices;
	int i = 0;
	for (const char* read = buffer; read != '\0'; read = strchr(read, '\n'))
	{
		char first = *read;
		if (first == '\n')
		{
			read++;
			first = *read;
		}

		if (*read == 'v')
		{
			char second = *(read + 1);
			if (second == 'n')
			{
				vec3 normal;
				sscanf(read, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
				rawNormals.push_back(normal);
			}
			else if (second == 't')
			{
				vec2 tex;
				sscanf(read, "vt %f %f", &tex.x, &tex.y);
				rawTexcoords.push_back(tex);
			}
			else
			{
				vec3 vert;
				sscanf(read, "v %f %f %f", &vert.x, &vert.y, &vert.z);
				rawVertices.push_back(vert);
			}
		}
		else if (first == 'f')
		{
			ivec3 a, b, c;
			sscanf(read, "f %d/%d/%d %d/%d/%d %d/%d/%d", &a.x, &a.y, &a.z, &b.x, &b.y, &b.z, &c.x, &c.y, &c.z);
			a -= 1;
			b -= 1;
			c -= 1;

			int indexA = orderByVertices ? a.x : a.y;
			int indexB = orderByVertices ? b.x : b.y;
			int indexC = orderByVertices ? c.x : c.y;
			
			shape->vertices[indexA] = vec4(rawVertices[a.x], 1.0f);
			shape->vertices[indexB] = vec4(rawVertices[b.x], 1.0f);
			shape->vertices[indexC] = vec4(rawVertices[c.x], 1.0f);
			shape->texcoords[indexA] =rawTexcoords[a.y];
			shape->texcoords[indexB] = rawTexcoords[b.y];
			shape->texcoords[indexC] = rawTexcoords[c.y];
			shape->normals[indexA] = rawNormals[a.z];
			shape->normals[indexB] = rawNormals[b.z];
			shape->normals[indexC] = rawNormals[c.z];
			shape->faceIndices[i] = ivec3(indexA, indexB, indexC);
			i++;
		}
	}
	
	std::vector<vec3> faceTangents(elements);
	for (int i = 0; i < faces; i++)
	{
		// http://www.terathon.com/code/tangent.html
		ivec3 face = shape->faceIndices[i];
		vec4 va = shape->vertices[face.x];
		vec4 vb = shape->vertices[face.y];
		vec4 vc = shape->vertices[face.z];
		vec2 ta = shape->texcoords[face.x];
		vec2 tb = shape->texcoords[face.y];
		vec2 tc = shape->texcoords[face.z];
		
		float x1 = vb.x - va.x;
		float x2 = vc.x - va.x;
		float y1 = vb.y - va.y;
		float y2 = vc.y - va.y;
		float z1 = vb.z - va.z;
		float z2 = vc.z - va.z;
		float s1 = tb.x - ta.x;
		float s2 = tc.x - ta.x;
		float t1 = tb.y - ta.y;
		float t2 = tc.y - ta.y;
		
		float det = 1.0f / ((s1 * t2) - (s2 * t1));
		if (isinf(det))
		{
			det = 0.0f;
		}
		
		vec3 tangent(((t2 * x1) - (t1 * x2)) * det, ((t2 * y1) - (t1 * y2)) * det, ((t2 * z1) - (t1 * z2)) * det);
		faceTangents[face.x] += tangent;
		faceTangents[face.y] += tangent;
		faceTangents[face.z] += tangent;
	}
	
	for (int i = 0; i < vertices; i++)
	{
		vec3 normal = shape->normals[i];
		vec3 tangent = glm::normalize(faceTangents[i]);
		vec3 binormal = cross(normal, tangent);
		shape->tangents[i] = tangent;
		shape->binormals[i] = binormal;
	}
	
	rawNormals.clear();
	rawTexcoords.clear();
	rawVertices.clear();
	faceTangents.clear();
}

SHAPEAPI void shape_write_wavefront(shapeobj* shape, FILE* file)
{
	if (shape == 0 || shape->empty() || file == 0)
	{
		return;
	}
	
	fprintf(file, "\n");
	for (int i = 0; i < shape->vertices.elements; i++)
	{
		vec4 v = shape->vertices[i];
		fprintf(file, "v %f %f %f\n", v.x, v.y, v.z);
	}
	
	fprintf(file, "\n");
	for (int i = 0; i < shape->normals.elements; i++)
	{
		vec3 n = shape->normals[i];
		fprintf(file, "vn %f %f %f\n", n.x, n.y, n.z);
	}
	
	fprintf(file, "\n");
	for (int i = 0; i < shape->texcoords.elements; i++)
	{
		vec2 t = shape->texcoords[i];
		fprintf(file, "vt %f %f\n", t.x, t.y);
	}
	
#if 0
	fprintf(file, "\n");
	for (int i = 0; i < shape->tangents.elements; i++)
	{
		vec3 t = shape->tangents[i];
		fprintf(file, "tan %f %f %f\n", t.x, t.y, t.z);
	}

	fprintf(file, "\n");
	for (int i = 0; i < shape->binormals.elements; i++)
	{
		vec3 b = shape->binormals[i];
		fprintf(file, "bi %f %f %f\n", b.x, b.y, b.z);
	}
#endif

	fprintf(file, "\n");
	for (int i = 0; i < shape->faceIndices.elements; i++)
	{
		ivec3 a = shape->faceIndices[i];
		fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
			a.x + 1, a.x + 1, a.x + 1,
			a.y + 1, a.y + 1, a.y + 1,
			a.z + 1, a.z + 1, a.z + 1);
	}
}

SHAPEAPI shape_type shape_file_extension(const char* filename)
{
	size_t length = strlen(filename);
	if (length > 3)
	{
		const char* ext0 = filename + (length - 3);
		const char* ext1 = filename + (length - 2);
		const char* ext2 = filename + (length - 1);
		if (*ext0 == 'o' && *ext1 == 'b' && *ext2 == 'j') { return SHAPE_TYPE_WAVEFRONT; }
		else if (*ext0 == 'd' && *ext1 == 'a' && *ext2 == 'e') { return SHAPE_TYPE_COLLADA; }
		else if (*ext0 == 'f' && *ext1 == 'b' && *ext2 == 'x') { return SHAPE_TYPE_FBX; }
	}
	
	return SHAPE_TYPE_UNKNOWN;
}
SHAPEAPI const char* shape_type_extension(const shape_type type)
{
	switch (type)
	{
	case SHAPE_TYPE_WAVEFRONT: return "obj";
	case SHAPE_TYPE_COLLADA: return "dae";
	case SHAPE_TYPE_FBX: return "fbx";
	default: return 0;
	}
}

SHAPEAPI void shape_normalize(shapeobj* shape)
{
	float unitSize = 0.0f;
	for (int i = 0; i < shape->vertices.elements; i++)
	{
		vec4 v = shape->vertices[i];
		if (v.x > unitSize)
		{
			unitSize = v.x;
		}
		
		if (v.y > unitSize)
		{
			unitSize = v.y;
		}
		
		if (v.z > unitSize)
		{
			unitSize = v.z;
		}
	}
	
	for (int i = 0; i < shape->vertices.elements; i++)
	{
		shape->vertices[i] /= unitSize;
		shape->normals[i] = glm::normalize(shape->normals[i]);
		shape->tangents[i] = glm::normalize(shape->tangents[i]);
		shape->binormals[i] = glm::normalize(shape->binormals[i]);
	}
}

SHAPEAPI void shape_transform(shapeobj* shape, const shape_transformation& modelview)
{
	mat4 m = modelview.trans * modelview.space;
	for (int i = 0; i < shape->vertices.elements; i++)
	{
		vec4* v = &(shape->vertices[i]);
		vec3* n = &(shape->normals[i]);
		vec3* f = &(shape->tangents[i]);
		vec3* b = &(shape->binormals[i]);
		*v = m * *v;
		*n = shapeobj::normalType(modelview.space * vec4(*n, 1.0));
		*f = shapeobj::tangentType(modelview.space * vec4(*f, 1.0));
		*b = shapeobj::binormalType(modelview.space * vec4(*b, 1.0));
	}
}
