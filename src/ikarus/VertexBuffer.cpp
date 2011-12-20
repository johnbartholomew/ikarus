#include "Global.h"
#include "VertexBuffer.h"

namespace
{
	inline unsigned int TypeSize(GLenum type)
	{
		switch (type)
		{
		case GL_BYTE:
			return sizeof(GLbyte);
		case GL_UNSIGNED_BYTE:
			return sizeof(GLubyte);
		case GL_SHORT:
			return sizeof(GLshort);
		case GL_UNSIGNED_SHORT:
			return sizeof(GLushort);
		case GL_INT:
			return sizeof(GLint);
		case GL_UNSIGNED_INT:
			return sizeof(GLuint);
		case GL_FLOAT:
			return sizeof(GLfloat);
		case GL_DOUBLE:
			return sizeof(GLdouble);
		default:
			assert(0 && "Invalid type code.");
			return 1;
		}
	}

	static VertexAttribute *CloneFormat(const VertexFormat format)
	{
		int attrib_count = 0;
		for (const VertexAttribute *attrib = format; attrib->binding != 0; ++attrib)
			++attrib_count;
		VertexAttribute *fmt = new VertexAttribute[attrib_count + 1];
		memcpy(fmt, format, sizeof(VertexAttribute) * attrib_count);
		fmt[attrib_count].binding = 0;
		fmt[attrib_count].count = 0;
		fmt[attrib_count].type = 0;
		return fmt;
	}

} // anonymous namespace

// --- VertexBuffer -----------------------------------------------------------

GLint VertexBuffer::sMaxTextureUnits = -1;
GLint VertexBuffer::sMaxVertexAttribs = -1;

VertexBuffer::VertexBuffer()
: mHandle(0), mNumVertices(0), mUsage(0)
{
	if (sMaxTextureUnits == -1 || sMaxVertexAttribs == -1)
	{
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &sMaxTextureUnits);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS_ARB, &sMaxVertexAttribs);
	}
}

VertexBuffer::VertexBuffer(unsigned int num_verts, const VertexFormat format, GLenum usage, bool use_vbo)
: mHandle(0), mNumVertices(0), mUsage(0)
{
	init(num_verts, format, usage, use_vbo);
}

VertexBuffer::~VertexBuffer()
{
	reset();
}

void VertexBuffer::init(unsigned int num_verts, const VertexFormat fmt, GLenum usage, bool use_vbo)
{
	reset();

	mFormat.reset(CloneFormat(fmt));
	unsigned int vert_size = CalcFormatStride(mFormat.get());
	mNumVertices = num_verts;
	mUsage = usage;

#ifndef DISABLE_VBOS
	if (GLEW_ARB_vertex_buffer_object && use_vbo)
	{
		glGenBuffersARB(1, &mHandle);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, mHandle);
	}
	else
	{
#endif
		mVertices.reset(new unsigned char[mNumVertices * vert_size]);
#ifndef DISABLE_VBOS
	}
#endif
}

void VertexBuffer::reset()
{
#ifndef DISABLE_VBOS
	if (GLEW_ARB_vertex_buffer_object && (mHandle != 0))
	{
		glDeleteBuffersARB(1, &mHandle);
		mHandle = 0;
	}
	else
	{
#endif
		mVertices.reset();
#ifndef DISABLE_VBOS
	}
#endif
	mFormat.reset();
	mUsage = 0;
	mNumVertices = 0;
}

void *VertexBuffer::lock()
{
#ifndef DISABLE_VBOS
	if (GLEW_ARB_vertex_buffer_object && (mHandle != 0))
	{
		unsigned int vert_size = CalcFormatStride(mFormat.get());

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, mHandle);
		// glBufferDataARB() with a null pointer allocates space for the buffer
		// if called after the VBO already has data, it should say "discard the data you have and allocate more",
		// but without affecting any existing rendering commands using that data,
		// so you get a fresh buffer and avoid stalls waiting for renders to complete
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, vert_size * mNumVertices, NULL, mUsage);
		return (unsigned char*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	}
	else
	{
#endif
		return reinterpret_cast<void*>(mVertices.get());
#ifndef DISABLE_VBOS
	}
#endif
}

void VertexBuffer::unlock()
{
#ifndef DISABLE_VBOS
	if (GLEW_ARB_vertex_buffer_object && (mHandle != 0))
		glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
#endif
}

void VertexBuffer::bind()
{
	unsigned int stride = CalcFormatStride(mFormat.get());
	unsigned char *offset;

#ifndef DISABLE_VBOS
	if (GLEW_ARB_vertex_buffer_object && (mHandle != 0))
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, mHandle);
		offset = 0;
	}
	else
	{
#endif
		offset = mVertices.get();
#ifndef DISABLE_VBOS
	}
#endif

	// disable all attributes
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_SECONDARY_COLOR_ARRAY_EXT);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_FOG_COORDINATE_ARRAY_EXT);
	for (GLint i = GL_TEXTURE0_ARB; i < (GL_TEXTURE0_ARB + sMaxTextureUnits); ++i)
	{
		glClientActiveTextureARB(i);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	for (GLint i = 1; i < sMaxVertexAttribs; ++i)
		glDisableVertexAttribArrayARB(i);

	// bind and enable the attributes for the format
	for (VertexAttribute *attrib = mFormat.get(); attrib->binding != 0; ++attrib)
	{
		switch (attrib->binding)
		{
		case VertexAttribute::BindNone:
			break;
		case VertexAttribute::BindVertex:
			glVertexPointer(attrib->count, attrib->type, stride, offset);
			glEnableClientState(GL_VERTEX_ARRAY);
			break;
		case VertexAttribute::BindColour:
			glColorPointer(attrib->count, attrib->type, stride, offset);
			glEnableClientState(GL_COLOR_ARRAY);
			break;
		case VertexAttribute::BindSecondaryColour:
			glSecondaryColorPointerEXT(attrib->count, attrib->type, stride, offset);
			glEnableClientState(GL_SECONDARY_COLOR_ARRAY_EXT);
			break;
		case VertexAttribute::BindNormal:
			glNormalPointer(attrib->type, stride, offset);
			assert(attrib->count == 3);
			glEnableClientState(GL_NORMAL_ARRAY);
			break;
		case VertexAttribute::BindFogCoord:
			glFogCoordPointerEXT(attrib->type, stride, offset);
			assert(attrib->count == 1);
			glEnableClientState(GL_FOG_COORDINATE_ARRAY_EXT);
			break;
		default: break;
		}

		if ((attrib->binding & VertexAttribute::BindTexCoordFLAG) == VertexAttribute::BindTexCoordFLAG)
		{
			int tex_idx = attrib->binding - VertexAttribute::BindTexCoord0;
			int tex_id = GL_TEXTURE0_ARB + tex_idx;
			glClientActiveTextureARB(tex_id);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(attrib->count, attrib->type, stride, offset);
		}

		if ((attrib->binding & VertexAttribute::BindGenericFLAG) == VertexAttribute::BindGenericFLAG)
		{
			int idx = attrib->binding - VertexAttribute::BindGeneric0;
			glEnableVertexAttribArrayARB(idx);
			glVertexAttribPointerARB(idx, attrib->count, attrib->type, GL_FALSE, stride, offset);
		}

		offset += TypeSize(attrib->type) * attrib->count;
	}
}

void VertexBuffer::draw(unsigned int primitive_type, unsigned int vertex_count, unsigned int start_vertex)
{
	glDrawArrays(primitive_type, start_vertex, vertex_count);
}

unsigned int VertexBuffer::CalcFormatStride(const VertexFormat format)
{
	unsigned int sz = 0;
	for (const VertexAttribute *attrib = format; attrib->binding != 0; ++attrib)
		sz += attrib->count * TypeSize(attrib->type);
	return sz;
}

// --- IndexBuffer ------------------------------------------------------------

IndexBuffer::IndexBuffer()
: mHandle(0), mType(GL_UNSIGNED_SHORT), mNumIndices(0)
{
}

IndexBuffer::IndexBuffer(unsigned int num_indices, unsigned int type_, GLenum usage)
: mHandle(0), mType(GL_UNSIGNED_SHORT), mNumIndices(0)
{
	init(num_indices, type_, usage);
}

IndexBuffer::~IndexBuffer()
{
	reset();
}

void IndexBuffer::init(unsigned int num_indices, unsigned int type_, GLenum usage)
{
	reset();

	mNumIndices = num_indices;
	mType = type_;
	unsigned int index_size = TypeSize(mType);
#ifndef DISABLE_VBOS
	if (GLEW_ARB_vertex_buffer_object)
	{
		glGenBuffersARB(1, &mHandle);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mHandle);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, index_size * mNumIndices, NULL, usage);
	}
	else
	{
#endif
		mIndices.reset(new unsigned char[index_size * mNumIndices]);
#ifndef DISABLE_VBOS
	}
#endif
}

void IndexBuffer::reset()
{
#ifndef DISABLE_VBOS
	if (GLEW_ARB_vertex_buffer_object)
	{
		if (mHandle != 0)
		{
			glDeleteBuffersARB(1, &mHandle);
			mHandle = 0;
		}
	}
	else
	{
#endif
		mIndices.reset();
#ifndef DISABLE_VBOS
	}
#endif
}

void IndexBuffer::bind()
{
#ifndef DISABLE_VBOS
	if (GLEW_ARB_vertex_buffer_object)
	{
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mHandle);
	}
#endif
}

void IndexBuffer::draw(unsigned int primitive_type, unsigned int index_count, unsigned int start_index)
{
	unsigned int index_size = TypeSize(mType);
#ifndef DISABLE_VBOS
	if (GLEW_ARB_vertex_buffer_object)
	{
#pragma warning(disable: 4312) // 'reinterpret_cast' : conversion from 'unsigned int' to 'unsigned int *' of greater size
		glDrawElements(primitive_type, index_count, mType, reinterpret_cast<unsigned int*>(start_index * index_size));
#pragma warning(default: 4312)
	}
	else
	{
#endif
		glDrawElements(primitive_type, index_count, mType, reinterpret_cast<unsigned int*>(start_index * index_size + mIndices.get()));
#ifndef DISABLE_VBOS
	}
#endif
}

void *IndexBuffer::lock()
{
#ifndef DISABLE_VBOS
	if (GLEW_ARB_vertex_buffer_object)
	{
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mHandle);
		return glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	}
	else
	{
#endif
		return reinterpret_cast<void*>(mIndices.get());
#ifndef DISABLE_VBOS
	}
#endif
}

void IndexBuffer::unlock()
{
#ifndef DISABLE_VBOS
	if (GLEW_ARB_vertex_buffer_object)
		glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
#endif
}
