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

/// <summary>
/// Enumeration for types of shape files.
/// </summary>
typedef enum shape_type
{
	SHAPE_TYPE_UNKNOWN,
	SHAPE_TYPE_WAVEFRONT,
	SHAPE_TYPE_COLLADA,
	SHAPE_TYPE_FBX
} shape_type;

/// <summary>
/// Contains methods and properties for buffer of shape elements.
/// </summary>
template <typename T> struct SHAPEAPI shape_component
{
	
	inline shape_component() :
		buffer(0),
		elements(0),
		offset(0),
		stride(sizeof(T)),
		size(0) {}
	/// <param name="buffer">Buffer of elements.</param>
	/// <param name="elements">Number of elements in the buffer.</param>
	/// <param name="offset">Offset that from the beginning of the shape buffer to the given buffer of elements.</param>
	inline shape_component(const T* buffer, const size_t elements, const size_t offset) :
		buffer(buffer),
		elements(elements),
		offset(offset),
		stride(sizeof(T)),
		size(elements * sizeof(T)) {}
	/// <param name="b">Other component instance.</param>
	inline shape_component(const shape_component<T>& b) :
		buffer(b.buffer),
		elements(b.elements),
		offset(b.offset),
		stride(sizeof(T)),
		size(elements * sizeof(T)) {}
	inline ~shape_component() {}
	
	/// <summary>
	/// Gets a value indicating whether of not the two components are equal.
	/// </summary>
	/// <param name="b">Other component instance.</param>
	inline void operator=(const shape_component<T>& b)
	{
		memcpy(this, &b, sizeof(shape_component<T>));
	}
	/// <summary>
	/// Gets a pointer to the buffer.
	/// </summary>
	inline T* operator+(const size_t index)
	{
		return (T*)this->buffer + index;
	}
	/// <summary>
	/// Gets a pointer to the buffer moved by the given index.
	/// </summary>
	/// <param name="index">Index to subtract to the buffer.</param>
	inline T* operator-(const size_t index)
	{
		return (T*)this->buffer - index;
	}
	/// <summary>
	/// Gets an element from the buffer.
	/// </summary>
	/// <param name="index">Index to select from the buffer by.</param>
	inline T& operator[](const size_t index)
	{
		return (T&)this->buffer[index % this->elements];
	}
	
	/// <summary>
	/// Pointer to the component's buffer.
	/// </summary>
	T const * buffer;
	/// <summary>
	/// Number of elements in the component.
	/// </summary>
	const size_t elements;
	/// <summary>
	/// The offset from the beginning of the shape's buffer to this component's buffer.
	/// </summary>
	const size_t offset;
	/// <summary>
	/// The size of each element in this component.
	/// </summary>
	const size_t stride;
	/// <summary>
	/// The size of all element's in this component.
	/// </summary>
	const size_t size;
	
};

/// <summary>
/// Contains matrices used for transforming a shape.
/// </summary>
struct SHAPEAPI shape_transformation
{

	inline shape_transformation() :
		trans(1.0f),
		space(1.0f) {}
	/// <param name="translation">3-dimensional vector representing translation.</param>
	/// <param name="scale">3-dimensional vector representing scale.</param>
	/// <param name="rotation">3-dimensional vector representing rotation by radians.</param>
	inline shape_transformation(const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& rotation) :
		trans(glm::scale(scale) * glm::translate(translation)),
		space(
			glm::rotate(rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))) {}
	/// <param name="translation">3-dimensional vector representing translation.</param>
	/// <param name="scale">3-dimensional vector representing scale.</param>
	/// <param name="space">3 by 3 matrix representing rotational space.</param>
	inline shape_transformation(const glm::vec3& translation, const glm::vec3& scale, const glm::mat3& space) :
		trans(glm::translate(translation)),
		space(glm::mat4(space) * glm::scale(scale)) {}
	inline ~shape_transformation() {}
	
	/// <summary>
	/// 4 by 4 matrix representing a shape translation.
	/// </summary>
	glm::mat4 trans;
	/// <summary>
	/// 4 by 4 matrix representing a shape view space.
	/// </summary>
	glm::mat4 space;

};

/// <summary>
/// Contains methods and properties for an instance of shape.
/// </summary>
struct SHAPEAPI shapeobj_t
{
	
	typedef int16_t indexType;
	typedef float valueType;
	
	typedef glm::tvec3<indexType> faceType;
	typedef glm::tvec4<valueType> vertexType;
	typedef glm::tvec2<valueType> texcoordType;
	typedef glm::tvec3<valueType> normalType;
	typedef glm::tvec3<valueType> tangentType;
	typedef glm::tvec3<valueType> binormalType;
	
	inline shapeobj_t() :
		faces(0),
		elements(0),
		components(0),
		size(0),
		buffer(0) {}
	/// <param name="faces">Number of faces in the shape.</param>
	/// <param name="elements">Number of elements in the shape.</param>
	inline shapeobj_t(const size_t faces, const size_t elements) :
		faces(faces),
		elements(elements),
		components(5),
		size(0),
		buffer(0) {}
	/// <param name="other">Other shape instance.</param>
	inline shapeobj_t(const shapeobj_t& other) :
		faces(other.faces),
		elements(other.elements),
		components(other.components),
		size(other.size),
		buffer(other.buffer) {}
	inline ~shapeobj_t() {}
	
	/// <summary>
	/// Gets a value indicating whether or not this instance is empty.
	/// </summary>
	bool empty() const
	{
		return this->buffer == 0 && this->size < 1;
	}
	
	/// <summary>
	/// Number of triangle faces in the shape.
	/// </summary>
	size_t faces;
	/// <summary>
	/// Number of elements in the shape.
	/// </summary>
	size_t elements;
	/// <summary>
	/// Number of components in the shape.
	/// </summary>
	size_t components;
	/// <summary>
	/// Size of the buffer in bytes.
	/// </summary>
	size_t size;
	/// <summary>
	/// Buffer containing the indices and the elements.
	/// </summary>
	void* buffer;
	/// <summary>
	/// Type of the shape.
	/// </summary>
	shape_type type;
	
	/// <summary>
	/// Component for the shape's indices, every grouping of 3 will be a face.
	/// </summary>
	shape_component<indexType> indices;
	/// <summary>
	/// Component for the shape's face indices.
	/// </summary>
	shape_component<faceType> faceIndices;
	
	/// <summary>
	/// Component for the shape's vertices.
	/// </summary>
	shape_component<vertexType> vertices;
	/// <summary>
	/// Component for the shape's texcoords.
	/// </summary>
	shape_component<texcoordType> texcoords;
	/// <summary>
	/// Component for the shape's normals.
	/// </summary>
	shape_component<normalType> normals;
	/// <summary>
	/// Component for the shape's binormals.
	/// </summary>
	shape_component<binormalType> binormals;
	/// <summary>
	/// Component for the shape's tangents.
	/// </summary>
	shape_component<tangentType> tangents;
	
};

#if 0
/// <summary>
/// Contains methods and properties for an instance of shape_point.
/// </summary>
struct SHAPEAPI shape_point
{
	
	inline shape_point() :
		vertex(0),
		texcoord(0),
		normal(0),
		tangent(0),
		binormal(0) {}
	/// <param name="vertex">Pointer to a 3-dimensional vector representing a point's vertex.</param>
	/// <param name="texcoord">Pointer to a 3-dimensional vector representing a point's texcoord.</param>
	/// <param name="normal">Pointer to a 3-dimensional vector representing a point's normal.</param>
	/// <param name="tangent">Pointer to a 3-dimensional vector representing a point's tangent.</param>
	/// <param name="binormal">Pointer to a 3-dimensional vector representing a point's binormal.</param>
	inline shape_point(glm::vec4* vertex, glm::vec2* texcoord, glm::vec3* normal, glm::vec3* tangent, glm::vec3* binormal) :
		vertex(vertex), texcoord(texcoord), normal(normal), tangent(tangent), binormal(binormal) {}
	inline ~shape_point() {}
	
	glm::vec4 const * vertex;
	glm::vec2 const * texcoord;
	glm::vec3 const * normal;
	glm::vec3 const * tangent;
	glm::vec3 const * binormal;
	
};

/// <summary>
/// Contains methods and properties for an instance of shape_face.
/// </summary>
struct SHAPEAPI shape_face
{
	
	inline shape_face() {}
	/// <param name="vertices">Component of vertices.</param>
	/// <param name="texcoords">Component of texcoords.</param>
	/// <param name="normals">Component of normals.</param>
	/// <param name="tangents">Component of tangents.</param>
	/// <param name="binormals">Component of binormals.</param>
	inline shape_face(
		const shape_component<glm::vec4>& vertices,
		const shape_component<glm::vec2>& texcoords,
		const shape_component<glm::vec3>& normals,
		const shape_component<glm::vec3>& tangents,
		const shape_component<glm::vec3>& binormals,
		const glm::ivec3& face) :
		a(shape_point(&vertices[face.x], &texcoords[face.x], &normals[face.x], &tangents[face.x], &binormals[face.x])),
		b(shape_point(&vertices[face.y], &texcoords[face.y], &normals[face.y], &tangents[face.y], &binormals[face.y])),
		c(shape_point(&vertices[face.z], &texcoords[face.z], &normals[face.z], &tangents[face.z], &binormals[face.z])) {}
	inline ~shape_face() {}
	
	shape_point a, b, c;
	
};
#endif

/// <summary>
/// Initializes the api for reading and writing shapes.
/// </summary>
extern SHAPEAPI void shape_init();
/// <summary>
/// Releases all held assets for created shapes.
/// </summary>
extern SHAPEAPI void shape_release();

/// <summary>
/// Frees the resources used by a given shape.
/// </summary>
/// <param name="shape">Pointer to a shape.</param>
extern SHAPEAPI void shape_free(shapeobj_t* shape);

/// <summary>
/// Reads a shape object based on the given type.
/// </summary>
/// <param name="shape">Pointer to a shape.</param>
/// <param name="type">Type of the given shape file.</param>
/// <param name="file">Pointer to a standard file.<param>
extern SHAPEAPI void shape_read(shapeobj_t* shape, const shape_type type, FILE* file);
/// <summary>
/// Reads a shape object from a character buffer.
/// </summary>
/// <param name="shape">Pointer to a shape.</param>
/// <param name="type">Type of the given shape file.</param>
/// <param name="buffer">Buffer of characters representing a shape file.</param>
extern SHAPEAPI void shape_read(shapeobj_t* shape, const shape_type type, const char* buffer);
/// <summary>
/// Writes a shape object based on the given type.
/// </summary>
/// <param name="shape">Pointer to a shape.</param>
/// <param name="type">Type of the given shape file.</param>
/// <param name="file">Pointer to a standard file.<param>
extern SHAPEAPI void shape_write(shapeobj_t* shape, const shape_type type, FILE* file);

/// <summary>
/// Reads a wavefront shape object from a buffer of characters.
/// </summary>
/// <param name="shape">Pointer to a shape.</param>
/// <param name="buffer">Buffer of characters representing a shape file.</param>
extern SHAPEAPI void shape_read_wavefront(shapeobj_t* shape, const char* buffer);
/// <summary>
/// Writes a wavefront shape object to a standard file.
/// </summary>
/// <param name="shape">Pointer to a shape.</param>
/// <param name="file">Pointer to a standard file object.<param>
extern SHAPEAPI void shape_write_wavefront(shapeobj_t* shape, FILE* file);

/// <summary>
/// Reads a collada shape object from a buffer of characters.
/// </summary>
/// <param name="shape">Pointer to a shape.</param>
/// <param name="buffer">Buffer of characters representing a shape file.</param>
extern SHAPEAPI void shape_read_collada(shapeobj_t* shape, const char* buffer);
/// <summary>
/// Writes a collada shape object to a standard file.
/// </summary>
/// <param name="shape">Pointer to a shape.</param>
/// <param name="file">Pointer to a standard file.<param>
extern SHAPEAPI void shape_write_collada(shapeobj_t* shape, FILE* file);

/// <summary>
/// Calculates tangents and binormals for a shape object.
/// </summary>
/// <param name="shape">Pointer to a shape.</param>
extern SHAPEAPI void shape_split_tangents(shapeobj_t* shape);

/// <summary>
/// Gets the shape type of a given filename.
/// </summary>
/// <param name="filename">String with the filename and path of a file.</param>
extern SHAPEAPI shape_type shape_file_extension(const char* filename);
/// <summary>
/// Gets the file extension of a given shape type.
/// </summary>
/// <param name="type">Type of a shape file.</param>
extern SHAPEAPI const char* shape_type_extension(const shape_type type);

/// <summary>
/// Normalizes the surface vectors (normal, tangent, binormal) of a shape object.
/// </summary>
/// <param name="shape">Pointer to a shape.</param>
extern SHAPEAPI void shape_normalize(shapeobj_t* shape);

/// <summary>
/// Transform a shape object based on the the given transformation.
/// </summary>
/// <param name="shape">Pointer to a shape.</param>
/// <param name="modelview">Shape transform data.</param>
extern SHAPEAPI void shape_transform(shapeobj_t* shape, const shape_transformation& modelview);