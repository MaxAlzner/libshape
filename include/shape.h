#pragma once

#define GLM_FORCE_RADIANS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <rapidxml.hpp>

#define SHAPEAPI 

typedef enum shape_type
{
	SHAPE_TYPE_UNKNOWN,
	SHAPE_TYPE_WAVEFRONT,
	SHAPE_TYPE_COLLADA,
	SHAPE_TYPE_FBX
} shape_type;

template <typename T> struct SHAPEAPI shape_component
{
	
	inline shape_component() :
		buffer(0),
		elements(0),
		offset(0),
		stride(sizeof(T)),
		size(0) {}
	inline shape_component(const T* buffer, const int32_t elements, const int32_t offset) :
		buffer(buffer),
		elements(elements),
		offset(offset),
		stride(sizeof(T)),
		size(elements * sizeof(T)) {}
	inline shape_component(const shape_component<T>& b) :
		buffer(b.buffer),
		elements(b.elements),
		offset(b.offset),
		stride(sizeof(T)),
		size(elements * sizeof(T)) {}
	inline ~shape_component() {}
	
	inline void operator=(const shape_component<T>& b)
	{
		memcpy(this, &b, sizeof(shape_component<T>));
	}
	inline T* operator+(const int32_t index)
	{
		return (T*)this->buffer + index;
	}
	inline T* operator-(const int32_t index)
	{
		return (T*)this->buffer - index;
	}
	inline T& operator[](const int32_t index)
	{
		return (T&)this->buffer[index % this->elements];
	}
	
	T const * buffer;
	const int32_t elements;
	const int32_t offset;
	const int32_t stride;
	const int32_t size;
	
};

struct SHAPEAPI shape_transformation
{

	inline shape_transformation() :
		trans(1.0f),
		space(1.0f) {}
	inline shape_transformation(const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& rotation) :
		trans(glm::scale(scale) * glm::translate(translation)),
		space(
			glm::rotate(rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))) {}
	inline shape_transformation(const glm::vec3& translation, const glm::vec3& scale, const glm::mat3& space) :
		trans(glm::translate(translation)),
		space(glm::mat4(space) * glm::scale(scale)) {}
	inline ~shape_transformation() {}

	glm::mat4 trans;
	glm::mat4 space;

};

typedef struct SHAPEAPI shapeobj
{
	
	typedef int16_t indexType;
	typedef float valueType;
	
	typedef glm::tvec3<indexType> faceType;
	typedef glm::tvec4<valueType> vertexType;
	typedef glm::tvec2<valueType> texcoordType;
	typedef glm::tvec3<valueType> normalType;
	typedef glm::tvec3<valueType> tangentType;
	typedef glm::tvec3<valueType> binormalType;
	
	inline shapeobj() :
		faces(0),
		elements(0),
		components(0),
		size(0),
		buffer(0) {}
	inline shapeobj(const int32_t faces, const int32_t elements) :
		faces(faces),
		elements(elements),
		components(5),
		size(0),
		buffer(0) {}
	inline shapeobj(const shapeobj& other) :
		faces(other.faces),
		elements(other.elements),
		components(other.components),
		size(other.size),
		buffer(other.buffer) {}
	inline ~shapeobj() {}
	
	bool empty() const
	{
		return this->buffer == 0 && this->size < 1;
	}
	
	int32_t faces;
	int32_t elements;
	int32_t components;
	uint32_t size;
	void* buffer;
	shape_type type;
	
	shape_component<indexType> indices;
	shape_component<faceType> faceIndices;

	shape_component<vertexType> vertices;
	shape_component<texcoordType> texcoords;
	shape_component<normalType> normals;
	shape_component<binormalType> binormals;
	shape_component<tangentType> tangents;
	
} shapeobj;

#if 0
struct SHAPEAPI shape_point
{
	
	inline shape_point() :
		vertex(0),
		texcoord(0),
		normal(0),
		tangent(0),
		binormal(0) {}
	inline shape_point(glm::vec4* vertex, glm::vec2* texcoord, glm::vec3* normal, glm::vec3* tangent, glm::vec3* binormal) :
		vertex(vertex), texcoord(texcoord), normal(normal), tangent(tangent), binormal(binormal) {}
	inline ~shape_point() {}
	
	glm::vec4 const * vertex;
	glm::vec2 const * texcoord;
	glm::vec3 const * normal;
	glm::vec3 const * tangent;
	glm::vec3 const * binormal;
	
};

struct SHAPEAPI shape_face
{
	
	inline shape_face() {}
	inline shape_face(
		const shape_component& vertices,
		const shape_component& texcoords,
		const shape_component& normals,
		const shape_component& tangents,
		const shape_component& binormals,
		const glm::ivec3& face) :
		a(shape_point(&vertices[face.x], &texcoords[face.x], &normals[face.x], &tangents[face.x], &binormals[face.x])),
		b(shape_point(&vertices[face.y], &texcoords[face.y], &normals[face.y], &tangents[face.y], &binormals[face.y])),
		c(shape_point(&vertices[face.z], &texcoords[face.z], &normals[face.z], &tangents[face.z], &binormals[face.z])) {}
	inline ~shape_face() {}
	
	shape_point a, b, c;
	
};
#endif

extern SHAPEAPI void shape_init();
extern SHAPEAPI void shape_release();

extern SHAPEAPI void shape_free(shapeobj* shape);

extern SHAPEAPI void shape_read(shapeobj* shape, const shape_type type, FILE* file);
extern SHAPEAPI void shape_read(shapeobj* shape, const shape_type type, const char* buffer);
extern SHAPEAPI void shape_write(shapeobj* shape, const shape_type type, FILE* file);

extern SHAPEAPI void shape_read_wavefront(shapeobj* shape, const char* buffer);
extern SHAPEAPI void shape_write_wavefront(shapeobj* shape, FILE* file);

extern SHAPEAPI void shape_read_collada(shapeobj* shape, const char* buffer);
extern SHAPEAPI void shape_write_collada(shapeobj* shape, FILE* file);

extern SHAPEAPI void shape_split_tangents(shapeobj* shape);

extern SHAPEAPI shape_type shape_file_extension(const char* filename);
extern SHAPEAPI const char* shape_type_extension(const shape_type type);

extern SHAPEAPI void shape_normalize(shapeobj* shape);

extern SHAPEAPI void shape_transform(shapeobj* shape, const shape_transformation& modelview);