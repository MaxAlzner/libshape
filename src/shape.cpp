
#define _CRT_SECURE_NO_WARNINGS
#include "../include/shape.h"

using namespace glm;
using namespace rapidxml;

SHAPEAPI void shape_init()
{
	
}
SHAPEAPI void shape_release()
{
	
}

SHAPEAPI void shape_free(shapeobj_t* shape)
{
	if (shape != 0 && !shape->empty())
	{
		free(shape->buffer);
		*shape = shapeobj_t();
	}
}

SHAPEAPI void shape_read(shapeobj_t* shape, const shape_type type, FILE* file)
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
SHAPEAPI void shape_read(shapeobj_t* shape, const shape_type type, const char* buffer)
{
	if (shape != 0 && type != SHAPE_TYPE_UNKNOWN && buffer != 0)
	{
		switch (type)
		{
		case SHAPE_TYPE_WAVEFRONT:
			shape_read_wavefront(shape, buffer);
			break;
		case SHAPE_TYPE_COLLADA:
			shape_read_collada(shape, buffer);
			break;
		default:
			break;
		}
	}
}
SHAPEAPI void shape_write(shapeobj_t* shape, const shape_type type, FILE* file)
{
	if (shape != 0 && !shape->empty() && type != SHAPE_TYPE_UNKNOWN && file != 0)
	{
		switch (type)
		{
		case SHAPE_TYPE_WAVEFRONT:
			shape_write_wavefront(shape, file);
			break;
		case SHAPE_TYPE_COLLADA:
			shape_write_collada(shape, file);
			break;
		default:
			break;
		}
	}
}

SHAPEAPI void shape_read_wavefront(shapeobj_t* shape, const char* buffer)
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
	*shape = shapeobj_t(faces, elements);
	shape->size = (faces * sizeof(shapeobj_t::indexType) * 3) + (elements * sizeof(shapeobj_t::valueType) * 15);
	shape->buffer = malloc(shape->size);
	shape->type = SHAPE_TYPE_WAVEFRONT;
	memset(shape->buffer, 0, shape->size);
	shape->vertices = shape_component<shapeobj_t::vertexType>((shapeobj_t::vertexType*)shape->buffer, elements, 0);
	shape->texcoords = shape_component<shapeobj_t::texcoordType>(
		(shapeobj_t::texcoordType*)(((shapeobj_t::valueType*)shape->buffer) + (elements * 4)),
		elements,
		elements * 4 * sizeof(shapeobj_t::valueType));
	shape->normals = shape_component<shapeobj_t::normalType>(
		(shapeobj_t::normalType*)(((shapeobj_t::valueType*)shape->buffer) + (elements * 6)),
		elements,
		elements * 6 * sizeof(shapeobj_t::valueType));
	shape->tangents = shape_component<shapeobj_t::tangentType>(
		(shapeobj_t::tangentType*)(((shapeobj_t::valueType*)shape->buffer) + (elements * 9)),
		elements,
		elements * 9 * sizeof(shapeobj_t::valueType));
	shape->binormals = shape_component<shapeobj_t::binormalType>(
		(shapeobj_t::binormalType*)(((shapeobj_t::valueType*)shape->buffer) + (elements * 12)),
		elements,
		elements * 12 * sizeof(shapeobj_t::valueType));
	shape->faceIndices = shape_component<shapeobj_t::faceType>(
		(shapeobj_t::faceType*)(((shapeobj_t::valueType*)shape->buffer) + (elements * 15)),
		faces,
		elements * 15 * sizeof(shapeobj_t::valueType));
	shape->indices = shape_component<shapeobj_t::indexType>(
		(shapeobj_t::indexType*)shape->faceIndices.buffer,
		faces * 3,
		shape->faceIndices.offset);

	std::vector<vec3> rawVertices;
	std::vector<vec2> rawTexcoords;
	std::vector<vec3> rawNormals;
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
	
	rawNormals.clear();
	rawTexcoords.clear();
	rawVertices.clear();
	shape_split_tangents(shape);
}

SHAPEAPI void shape_write_wavefront(shapeobj_t* shape, FILE* file)
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

SHAPEAPI void shape_read_collada(shapeobj_t* shape, const char* buffer)
{
	if (shape == 0 || buffer == 0)
	{
		return;
	}
	
	std::vector<vec3> vertices;
	std::vector<vec2> texcoords;
	std::vector<vec3> normals;
	std::vector<ivec3> points;
	float scale = 1.0f;
	xml_document<char> document;
	document.parse<0>((char*)buffer);
	xml_node<char>* root = document.first_node();
	if (root != 0)
	{
		xml_node<char>* node = root->first_node();
		while (node != 0)
		{
			if (strcmp(node->name(), "asset") == 0)
			{
				xml_node<char>* unit = node->last_node("unit");
				if (unit != 0)
				{
					xml_attribute<char>* meter = unit->last_attribute("meter");
					scale = meter != 0 ? (float)atof(meter->value()) : scale;
				}
			}
			else if (strcmp(node->name(), "library_geometries") == 0)
			{
				xml_node<char>* geometry = node->first_node();
				xml_node<char>* mesh = geometry != 0 ? geometry->first_node() : 0;
				xml_node<char>* source = mesh != 0 ? mesh->first_node("source") : 0;
				while (source != 0)
				{
					xml_attribute<char>* name = source->last_attribute("name");
					xml_node<char>* values = source->last_node("float_array");
					if (name != 0 && values != 0)
					{
						int stride = 0;
						if (strcmp(name->value(), "position") == 0)
						{
							stride = 3;
						}
						else if (strcmp(name->value(), "normal") == 0)
						{
							stride = 3;
						}
						else if (strcmp(name->value(), "map1") == 0)
						{
							stride = 2;
						}
						
						int i = 0;
						std::vector<shapeobj_t::valueType> stack;
						for (const char* read = values->value(); read != 0; read = strchr(read + 1, ' '))
						{
							stack.push_back((shapeobj_t::valueType)atof(read));
							i++;
							if (i >= stride)
							{
								if (strcmp(name->value(), "position") == 0)
								{
									//printf("  vertex: %f, %f, %f\n", stack[0], stack[1], stack[2]);
									vertices.push_back(vec3(stack[0], stack[1], stack[2]) * scale);
								}
								else if (strcmp(name->value(), "normal") == 0)
								{
									//printf("  normal: %f, %f, %f\n", stack[0], stack[1], stack[2]);
									normals.push_back(vec3(stack[0], stack[1], stack[2]));
								}
								else if (strcmp(name->value(), "map1") == 0)
								{
									//printf("  texcoord: %f, %f\n", stack[0], stack[1]);
									texcoords.push_back(vec2(stack[0], stack[1]));
								}
								
								stack.clear();
								i = 0;
							}
						}
					}
					
					source = source->next_sibling("source");
				}
				
				bool hasVertices = vertices.size() > 0;
				bool hasTexcoords = texcoords.size() > 0;
				bool hasNormals = normals.size() > 0;
				xml_node<char>* triangles = mesh != 0 ? mesh->first_node("triangles") : 0;
				while (triangles != 0)
				{
					xml_node<char>* values = triangles->last_node("p");
					if (values != 0)
					{
						for (const char* read = values->value(); read != 0; )
						{
							ivec3 point(-1, -1, -1);
							if (hasVertices)
							{
								point.x = atoi(read);
								read = strchr(read + 1, ' ');
							}
							
							if (hasTexcoords)
							{
								point.y = atof(read);
								read = strchr(read + 1, ' ');
							}
							
							if (hasNormals)
							{
								point.z = atof(read);
								read = strchr(read + 1, ' ');
							}
							
							//printf("  face: %d, %d, %d\n", point.x, point.y, point.z);
							points.push_back(point);
						}
					}
					
					triangles = triangles->next_sibling("triangles");
				}
			}
			
			node = node->next_sibling();
		}
	}
	
	//printf("    vertices: %d\n", (int)vertices.size());
	//printf("    texcoords: %d\n", (int)texcoords.size());
	//printf("    normals: %d\n", (int)normals.size());
	//printf("    points: %d\n", (int)points.size());
	
	if (vertices.size() < 1 || (points.size() % 3) != 0)
	{
		vertices.clear();
		texcoords.clear();
		normals.clear();
		points.clear();
		return;
	}
	
	int elements = max(vertices.size(), texcoords.size());
	int faces = points.size() / 3;
	*shape = shapeobj_t(faces, elements);
	shape->size = (faces * sizeof(shapeobj_t::indexType) * 3) + (elements * sizeof(shapeobj_t::valueType) * 15);
	shape->buffer = malloc(shape->size);
	shape->type = SHAPE_TYPE_COLLADA;
	memset(shape->buffer, 0, shape->size);
	shape->vertices = shape_component<shapeobj_t::vertexType>((shapeobj_t::vertexType*)shape->buffer, elements, 0);
	shape->texcoords = shape_component<shapeobj_t::texcoordType>(
		(shapeobj_t::texcoordType*)(((shapeobj_t::valueType*)shape->buffer) + (elements * 4)),
		elements,
		elements * 4 * sizeof(shapeobj_t::valueType));
	shape->normals = shape_component<shapeobj_t::normalType>(
		(shapeobj_t::normalType*)(((shapeobj_t::valueType*)shape->buffer) + (elements * 6)),
		elements,
		elements * 6 * sizeof(shapeobj_t::valueType));
	shape->tangents = shape_component<shapeobj_t::tangentType>(
		(shapeobj_t::tangentType*)(((shapeobj_t::valueType*)shape->buffer) + (elements * 9)),
		elements,
		elements * 9 * sizeof(shapeobj_t::valueType));
	shape->binormals = shape_component<shapeobj_t::binormalType>(
		(shapeobj_t::binormalType*)(((shapeobj_t::valueType*)shape->buffer) + (elements * 12)),
		elements,
		elements * 12 * sizeof(shapeobj_t::valueType));
	shape->faceIndices = shape_component<shapeobj_t::faceType>(
		(shapeobj_t::faceType*)(((shapeobj_t::valueType*)shape->buffer) + (elements * 15)),
		faces,
		elements * 15 * sizeof(shapeobj_t::valueType));
	shape->indices = shape_component<shapeobj_t::indexType>(
		(shapeobj_t::indexType*)shape->faceIndices.buffer,
		faces * 3,
		shape->faceIndices.offset);
	bool orderByVertices = texcoords.size() <= vertices.size();
	for (std::vector<ivec3>::iterator i = points.begin(); i != points.end(); i++)
	{
		ivec3 point = *i;
		shapeobj_t::indexType index = orderByVertices ? (shapeobj_t::indexType)point.x : (shapeobj_t::indexType)point.y;
		if (index >= 0)
		{
			shape->vertices[index] = shapeobj_t::vertexType(vertices[point.x], 1.0f);
			if (point.y >= 0 && texcoords.size() > 0)
			{
				shape->texcoords[index] = (shapeobj_t::texcoordType)texcoords[point.y];
			}
			
			if (point.z >= 0 && normals.size() > 0)
			{
				shape->normals[index] = (shapeobj_t::normalType)normals[point.z];
			}
			
			shape->indices[(int)(i - points.begin())] = index;
		}
	}
	
	vertices.clear();
	texcoords.clear();
	normals.clear();
	points.clear();
	shape_split_tangents(shape);
}
SHAPEAPI void shape_write_collada(shapeobj_t* shape, FILE* file)
{
	if (shape == 0 || shape->empty() || file == 0)
	{
		return;
	}
	
	fprintf(file, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	fprintf(file, "<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">\n");
	fprintf(file, "</COLLADA>\n");
}

SHAPEAPI void shape_split_tangents(shapeobj_t* shape)
{
	if (shape == 0 || shape->empty())
	{
		return;
	}
	
	std::vector<vec3> faceTangents(shape->elements);
	for (int32_t i = 0; i < shape->faces; i++)
	{
		// http://www.terathon.com/code/tangent.html
		ivec3 face = shape->faceIndices[i];
		shapeobj_t::vertexType va = shape->vertices[face.x];
		shapeobj_t::vertexType vb = shape->vertices[face.y];
		shapeobj_t::vertexType vc = shape->vertices[face.z];
		shapeobj_t::texcoordType ta = shape->texcoords[face.x];
		shapeobj_t::texcoordType tb = shape->texcoords[face.y];
		shapeobj_t::texcoordType tc = shape->texcoords[face.z];
		shapeobj_t::valueType x1 = vb.x - va.x;
		shapeobj_t::valueType x2 = vc.x - va.x;
		shapeobj_t::valueType y1 = vb.y - va.y;
		shapeobj_t::valueType y2 = vc.y - va.y;
		shapeobj_t::valueType z1 = vb.z - va.z;
		shapeobj_t::valueType z2 = vc.z - va.z;
		shapeobj_t::valueType s1 = tb.x - ta.x;
		shapeobj_t::valueType s2 = tc.x - ta.x;
		shapeobj_t::valueType t1 = tb.y - ta.y;
		shapeobj_t::valueType t2 = tc.y - ta.y;
		shapeobj_t::valueType det = (shapeobj_t::valueType)1 / ((s1 * t2) - (s2 * t1));
		if (isinf(det) || isnan(det))
		{
			det = (shapeobj_t::valueType)0;
		}
		
		vec3 tangent(((t2 * x1) - (t1 * x2)) * det, ((t2 * y1) - (t1 * y2)) * det, ((t2 * z1) - (t1 * z2)) * det);
		faceTangents[face.x] += tangent;
		faceTangents[face.y] += tangent;
		faceTangents[face.z] += tangent;
	}
	
	for (int32_t i = 0; i < shape->elements; i++)
	{
		vec3 normal = shape->normals[i];
		vec3 tangent = faceTangents[i];
		tangent = glm::length(tangent) > 0.0f ? glm::normalize(tangent) : tangent;
		vec3 binormal = glm::cross(normal, tangent);
		shape->tangents[i] = tangent;
		shape->binormals[i] = binormal;
	}
	
	faceTangents.clear();
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

SHAPEAPI void shape_normalize(shapeobj_t* shape)
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

SHAPEAPI void shape_transform(shapeobj_t* shape, const shape_transformation& modelview)
{
	mat4 m = modelview.trans * modelview.space;
	for (int i = 0; i < shape->vertices.elements; i++)
	{
		vec4* v = &(shape->vertices[i]);
		vec3* n = &(shape->normals[i]);
		vec3* f = &(shape->tangents[i]);
		vec3* b = &(shape->binormals[i]);
		*v = m * *v;
		*n = shapeobj_t::normalType(modelview.space * vec4(*n, 1.0));
		*f = shapeobj_t::tangentType(modelview.space * vec4(*f, 1.0));
		*b = shapeobj_t::binormalType(modelview.space * vec4(*b, 1.0));
	}
}
