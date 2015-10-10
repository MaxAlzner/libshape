#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>

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

	template <typename T> struct SHAPEAPI transformation
	{

		inline transformation() :
			translation((T)0),
			scale((T)1),
			rotation((T)1),
			space((T)1) {}
		inline transformation(const glm::tvec3<T>& translation, const glm::tvec3<T>& scale, const glm::tvec3<T>& rotation) :
			translation(translation),
			scale(scale),
			rotation(glm::rotate(glm::mat4(), rotation)),
			space(this->rotation) {}
		inline transformation(const glm::tvec3<T>& translation, const glm::tvec3<T>& scale, const glm::tmat3x3<T>& space) :
			translation(translation),
			scale(scale),
			rotation(space),
			space(space) {}
		inline transformation(const glm::tvec3<T>& translation, const glm::tvec3<T>& scale, const glm::tmat4x4<T>& space) :
			translation(translation),
			scale(scale),
			rotation(space),
			space(space) {}
		inline ~transformation() {}

		glm::tvec3<T> translation;
		glm::tvec3<T> scale;
		glm::tmat4x4<T> rotation;
		glm::tmat3x3<T> space;

	};

	class SHAPEAPI bone
	{
	public:

		inline bone() : _parent(0) {}
		inline ~bone() {}

		const bone* parent() const;

		void operator=(const glm::mat4& m);

	protected:

		bone* _parent;
		glm::mat4 _local;

	};

	inline shape() : _buffer(0), _size(0) {}
	inline ~shape() {}

	void release();

	void read(FILE* file);
	void write(FILE* file);

	void clear();

	void normalize();

	void putFace(const int index, const glm::ivec3& face);
	void putVertex(const int index, const glm::vec4& v);
	void putTexcoord(const int index, const glm::vec2& uv);
	void putNormal(const int index, const glm::vec3& n);
	void putTangent(const int index, const glm::vec3& t);
	void putBinormal(const int index, const glm::vec3& b);
	void putWeights(const int index, const glm::ivec4& w);
	void putBoneIndices(const int index, const glm::ivec4& bi);
	glm::ivec3& getFace(const int index);
	glm::vec4& getVertex(const int index);
	glm::vec2& getTexcoord(const int index);
	glm::vec3& getNormal(const int index);
	glm::vec3& getTangent(const int index);
	glm::vec3& getBinormal(const int index);
	glm::ivec4& getWeights(const int index);
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
	component<glm::ivec4> weights;
	component<glm::ivec4> boneIndices;

	component<glm::mat4> bones;

protected:

	void* _buffer;
	unsigned int _size;

};