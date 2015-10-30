#pragma once

#define GLM_FORCE_RADIANS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define SHAPEAPI 

class SHAPEAPI shape
{
public:

	template <typename T> struct SHAPEAPI component
	{

		inline component() :
			buffer(0),
			elements(0),
			offset(0),
			stride(sizeof(T)),
			size(0) {}
		inline component(const T* buffer, const int elements, const int offset) :
			buffer(buffer),
			elements(elements),
			offset(offset),
			stride(sizeof(T)),
			size(elements * sizeof(T)) {}
		inline component(const component<T>& b) :
			buffer(b.buffer),
			elements(b.elements),
			offset(b.offset),
			stride(sizeof(T)),
			size(elements * sizeof(T)) {}
		inline ~component() {}

		inline void operator=(const component<T>& b)
		{
			memcpy(this, &b, sizeof(component<T>));
		}
		inline T* operator+(const int index)
		{
			return (T*)this->buffer + index;
		}
		inline T* operator-(const int index)
		{
			return (T*)this->buffer - index;
		}
		inline T& operator[](const int index)
		{
			return (T&)this->buffer[index % this->elements];
		}

		const T* buffer;
		const int elements;
		const int offset;
		const int stride;
		const int size;

	};

#if 0
	template <typename T> struct SHAPEAPI transformation
	{

		inline transformation() :
			trans((T)1),
			space((T)1) {}
		inline transformation(const glm::tvec3<T>& translation, const glm::tvec3<T>& scale, const glm::tvec3<T>& rotation) :
			trans(glm::translate(translation)),
			space(glm::rotate(rotation.x, glm::tvec3<T>((T)1, (T)0, (T)0)) * glm::rotate(rotation.y, glm::tvec3<T>((T)0, (T)1, (T)0)) * glm::rotate(rotation.z, glm::tvec3<T>((T)0, (T)0, (T)1)) * glm::scale(scale)) {}
		inline transformation(const glm::tvec3<T>& translation, const glm::tvec3<T>& scale, const glm::tmat3x3<T>& space) :
			trans(glm::translate(translation)),
			space(glm::tmat4x4<T>(space) * glm::scale(scale)) {}
		inline ~transformation() {}

		glm::tmat4x4<T> trans;
		glm::tmat4x4<T> space;

	};
#endif

	inline shape() : _buffer(0), _size(0) {}
	inline ~shape() {}

	void read(FILE* file);
	void read(const char* buffer);
	void write(FILE* file);

	void clear();

	void normalize();
#if 0
	void transform(transformation<float>& modelview);
#endif

	void putFace(const int index, const glm::ivec3& face);
	void putVertex(const int index, const glm::vec4& v);
	void putTexcoord(const int index, const glm::vec2& uv);
	void putNormal(const int index, const glm::vec3& n);
	void putTangent(const int index, const glm::vec3& t);
	void putBinormal(const int index, const glm::vec3& b);
	void putBoneIndices(const int index, const glm::ivec4& bi);
	glm::ivec3& getFace(const int index);
	glm::vec4& getVertex(const int index);
	glm::vec2& getTexcoord(const int index);
	glm::vec3& getNormal(const int index);
	glm::vec3& getTangent(const int index);
	glm::vec3& getBinormal(const int index);
	glm::ivec4& getBoneIndices(const int index);

	bool empty() const;

	const int elements() const;
	const int components() const;
	const void* buffer() const;

	component<int> indices;
	component<glm::ivec3> faceIndices;

	component<glm::vec4> vertices;
	component<glm::vec2> texcoords;
	component<glm::vec3> normals;
	component<glm::vec3> binormals;
	component<glm::vec3> tangents;
	component<glm::ivec4> boneIndices;

	//component<glm::mat4> bones;

protected:

	void* _buffer;
	unsigned int _size;

};