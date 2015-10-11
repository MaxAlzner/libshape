
#include "../include/shape.h"

using namespace glm;

SHAPEAPI void shape::read(FILE* file)
{
	if (file == 0)
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
	this->read(raw);
	delete[] raw;
}
SHAPEAPI void shape::read(const char* buffer)
{
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
	this->clear();
	this->_size = (elements * 15 * sizeof(float)) + (faces * 3 * sizeof(int));
	this->_buffer = malloc(this->_size);
	memset(this->_buffer, 0, this->_size);
	this->vertices = component<vec4>((vec4*)this->_buffer, elements, 0);
	this->texcoords = component<vec2>(
		(vec2*)(((float*)this->_buffer) + (elements * 4)),
		elements,
		elements * 4 * sizeof(float));
	this->normals = component<vec3>(
		(vec3*)(((float*)this->_buffer) + (elements * 6)),
		elements,
		elements * 6 * sizeof(float));
	this->tangents = component<vec3>(
		(vec3*)(((float*)this->_buffer) + (elements * 9)),
		elements,
		elements * 9 * sizeof(float));
	this->binormals = component<vec3>(
		(vec3*)(((float*)this->_buffer) + (elements * 12)),
		elements,
		elements * 12 * sizeof(float));
	this->faceIndices = component<ivec3>(
		(ivec3*)(((float*)this->_buffer) + (elements * 15)),
		faces,
		elements * 15 * sizeof(float));
	this->indices = component<int>((int*)this->faceIndices.buffer, faces * 3, this->faceIndices.offset);

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
				sscanf_s(read, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
				rawNormals.push_back(normal);
			}
			else if (second == 't')
			{
				vec2 tex;
				sscanf_s(read, "vt %f %f", &tex.x, &tex.y);
				rawTexcoords.push_back(tex);
			}
			else
			{
				vec3 vert;
				sscanf_s(read, "v %f %f %f", &vert.x, &vert.y, &vert.z);
				rawVertices.push_back(vert);
			}
		}
		else if (first == 'f')
		{
			ivec3 a, b, c;
			sscanf_s(read, "f %d/%d/%d %d/%d/%d %d/%d/%d", &a.x, &a.y, &a.z, &b.x, &b.y, &b.z, &c.x, &c.y, &c.z);
			a -= 1;
			b -= 1;
			c -= 1;

			int indexA = orderByVertices ? a.x : a.y;
			int indexB = orderByVertices ? b.x : b.y;
			int indexC = orderByVertices ? c.x : c.y;

			this->putVertex(indexA, vec4(rawVertices[a.x], 1.0f));
			this->putVertex(indexB, vec4(rawVertices[b.x], 1.0f));
			this->putVertex(indexC, vec4(rawVertices[c.x], 1.0f));
			this->putTexcoord(indexA, rawTexcoords[a.y]);
			this->putTexcoord(indexB, rawTexcoords[b.y]);
			this->putTexcoord(indexC, rawTexcoords[c.y]);
			this->putNormal(indexA, rawNormals[a.z]);
			this->putNormal(indexB, rawNormals[b.z]);
			this->putNormal(indexC, rawNormals[c.z]);
			this->putFace(i, ivec3(indexA, indexB, indexC));
			i++;
		}
	}

	std::vector<vec3> faceTangents(elements);
	for (int i = 0; i < faces; i++)
	{
		// http://www.terathon.com/code/tangent.html
		ivec3 face = this->faceIndices[i];
		vec4 va = this->vertices[face.x];
		vec4 vb = this->vertices[face.y];
		vec4 vc = this->vertices[face.z];
		vec2 ta = this->texcoords[face.x];
		vec2 tb = this->texcoords[face.y];
		vec2 tc = this->texcoords[face.z];

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
		vec3 normal = this->normals[i];
		vec3 tangent = glm::normalize(faceTangents[i]);
		vec3 binormal = cross(normal, tangent);
		this->putTangent(i, tangent);
		this->putBinormal(i, binormal);
	}

	rawNormals.clear();
	rawTexcoords.clear();
	rawVertices.clear();
	faceTangents.clear();
}

SHAPEAPI void shape::write(FILE* file)
{
	if (file == 0)
	{
		return;
	}

	fprintf(file, "\n");
	for (int i = 0; i < this->vertices.elements; i++)
	{
		vec4 v = this->vertices[i];
		fprintf(file, "v %f %f %f\n", v.x, v.y, v.z);
	}

	fprintf(file, "\n");
	for (int i = 0; i < this->normals.elements; i++)
	{
		vec3 n = this->normals[i];
		fprintf(file, "vn %f %f %f\n", n.x, n.y, n.z);
	}

	fprintf(file, "\n");
	for (int i = 0; i < this->texcoords.elements; i++)
	{
		vec2 t = this->texcoords[i];
		fprintf(file, "vt %f %f\n", t.x, t.y);
	}

#if 0
	fprintf(file, "\n");
	for (int i = 0; i < this->tangents.elements; i++)
	{
		vec3 t = this->tangents[i];
		fprintf(file, "tan %f %f %f\n", t.x, t.y, t.z);
	}

	fprintf(file, "\n");
	for (int i = 0; i < this->binormals.elements; i++)
	{
		vec3 b = this->binormals[i];
		fprintf(file, "bi %f %f %f\n", b.x, b.y, b.z);
	}
#endif

	fprintf(file, "\n");
	for (int i = 0; i < this->faceIndices.elements; i++)
	{
		ivec3 a = this->faceIndices[i];
		fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
			a.x + 1, a.x + 1, a.x + 1,
			a.y + 1, a.y + 1, a.y + 1,
			a.z + 1, a.z + 1, a.z + 1);
	}
}

SHAPEAPI void shape::clear()
{
	this->indices = component<int>();
	this->faceIndices = component<ivec3>();
	this->vertices = component<vec4>();
	this->texcoords = component<vec2>();
	this->normals = component<vec3>();
	this->tangents = component<vec3>();
	this->binormals = component<vec3>();
	this->weights = component<ivec4>();
	this->boneIndices = component<ivec4>();
	//this->bones = component<mat4>();
	this->_size = 0;
	if (this->_buffer != 0)
	{
		delete[] this->_buffer;
	}
}

SHAPEAPI void shape::normalize()
{
	float unitSize = 0.0f;
	for (int i = 0; i < this->vertices.elements; i++)
	{
		vec4 v = this->vertices[i];
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

	for (int i = 0; i < this->vertices.elements; i++)
	{
		this->vertices[i] /= unitSize;
		this->normals[i] = glm::normalize(this->normals[i]);
		this->tangents[i] = glm::normalize(this->tangents[i]);
		this->binormals[i] = glm::normalize(this->binormals[i]);
	}
}
SHAPEAPI void shape::transform(transformation<float>& modelview)
{
	mat4 m = modelview.trans * modelview.space;
	for (int i = 0; i < this->vertices.elements; i++)
	{
		vec4* v = &(this->vertices[i]);
		*v = m * *v;
	}
}

SHAPEAPI void shape::putFace(const int index, const ivec3& face)
{
	this->faceIndices[index] = face;
}
SHAPEAPI void shape::putVertex(const int index, const vec4& v)
{
	this->vertices[index] = v;
}
SHAPEAPI void shape::putTexcoord(const int index, const vec2& uv)
{
	this->texcoords[index] = uv;
}
SHAPEAPI void shape::putNormal(const int index, const vec3& n)
{
	this->normals[index] = n;
}
SHAPEAPI void shape::putTangent(const int index, const vec3& t)
{
	this->tangents[index] = t;
}
SHAPEAPI void shape::putBinormal(const int index, const vec3& b)
{
	this->binormals[index] = b;
}
SHAPEAPI void shape::putWeights(const int index, const ivec4& w)
{
	this->weights[index] = w;
}
SHAPEAPI void shape::putBoneIndices(const int index, const ivec4& bi)
{
	this->boneIndices[index] = bi;
}
SHAPEAPI ivec3& shape::getFace(const int index)
{
	return this->faceIndices[index];
}
SHAPEAPI vec4& shape::getVertex(const int index)
{
	return this->vertices[index];
}
SHAPEAPI vec2& shape::getTexcoord(const int index)
{
	return this->texcoords[index];
}
SHAPEAPI vec3& shape::getNormal(const int index)
{
	return this->normals[index];
}
SHAPEAPI vec3& shape::getTangent(const int index)
{
	return this->tangents[index];
}
SHAPEAPI vec3& shape::getBinormal(const int index)
{
	return this->binormals[index];
}
SHAPEAPI ivec4& shape::getWeights(const int index)
{
	return this->weights[index];
}
SHAPEAPI ivec4& shape::getBoneIndices(const int index)
{
	return this->boneIndices[index];
}

SHAPEAPI bool shape::empty() const
{
	return this->_buffer == 0 && this->_size == 0;
}

SHAPEAPI const int shape::elements() const
{
	return this->vertices.elements;
}
SHAPEAPI const int shape::components() const
{
	return 5;
}
SHAPEAPI const void* shape::buffer() const
{
	return this->_buffer;
}