#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

// #define DISABLE_VBOS

struct VertexAttribute
{
	enum VertexAttributeBinding
	{
		BindNone            = 0x000,
		
		BindVertex          = 0x001,
		BindColour,
		BindSecondaryColour,
		BindNormal,
		BindFogCoord,

		BindTexCoord0       = 0x010,
		BindTexCoord1,
		BindTexCoord2,
		BindTexCoord3,
		BindTexCoord4,
		BindTexCoord5,
		BindTexCoord6,
		BindTexCoord7,
		BindTexCoord8,
		BindTexCoordFLAG    = 0x010,
		
		BindGeneric0        = 0x100,
		BindGeneric1,
		BindGeneric2,
		BindGeneric3,
		BindGeneric4,
		BindGeneric5,
		BindGeneric6,
		BindGeneric7,
		BindGeneric8,
		BindGeneric9,
		BindGeneric10,
		BindGeneric11,
		BindGeneric12,
		BindGeneric13,
		BindGeneric14,
		BindGeneric15,
		BindGenericFLAG     = 0x100,
	};

	int binding;
	GLint count;
	GLenum type;
};
typedef VertexAttribute VertexFormat[];

class VertexBuffer
{
	friend class VertexBufferLock;
public:
	explicit VertexBuffer();
	explicit VertexBuffer(unsigned int num_verts, const VertexFormat format, GLenum usage = GL_STATIC_DRAW_ARB, bool use_vbo = true);
	~VertexBuffer();

	void init(unsigned int num_verts, const VertexFormat format, GLenum usage = GL_STATIC_DRAW_ARB, bool use_vbo = true);
	void reset();

	void bind();
	void draw(unsigned int primitive_type, unsigned int vertex_count, unsigned int start_vertex);

	void *lock();
	void unlock();

	unsigned int getNumVertices()
	{ return mNumVertices; }

	static unsigned int CalcFormatStride(const VertexFormat format);
private:
	GLuint mHandle;
	unsigned int mNumVertices;
	GLenum mUsage;
	ScopedArray<VertexAttribute> mFormat;
	ScopedArray<unsigned char> mVertices;

	// static capabilities info
	// very slow to query this info, so it's just queried once and stored
	static GLint sMaxTextureUnits;
	static GLint sMaxVertexAttribs;
};

class VertexBufferLock
{
public:
	explicit VertexBufferLock(VertexBuffer &vbuf): mBuffer(vbuf), mPtr(vbuf.lock()) {}
	~VertexBufferLock() { if (mPtr) mBuffer.unlock(); }

	void reset()
	{ mBuffer.unlock(); mPtr = 0; }

	template <typename T>
	T *get() const
	{ return reinterpret_cast<T*>(mPtr); }
private:
	VertexBuffer &mBuffer;
	void *mPtr;
};

class IndexBuffer
{
public:
	explicit IndexBuffer();
	explicit IndexBuffer(unsigned int num_indices, unsigned int type = GL_UNSIGNED_SHORT, GLenum usage = GL_STATIC_DRAW_ARB);
	~IndexBuffer();

	void init(unsigned int num_indices, unsigned int type = GL_UNSIGNED_SHORT, GLenum usage = GL_STATIC_DRAW_ARB);
	void reset();

	void *lock();
	void unlock();

	void bind();
	void draw(unsigned int primitive_type, unsigned int index_count, unsigned int start_index);

	unsigned int getNumIndices()
	{ return mNumIndices; }
private:
	GLuint mHandle;
	unsigned int mNumIndices;
	unsigned int mType;
	ScopedArray<unsigned char> mIndices;
};

class IndexBufferLock
{
public:
	explicit IndexBufferLock(IndexBuffer &ibuf): mBuffer(ibuf), mPtr(ibuf.lock()) {}
	~IndexBufferLock() { if (mPtr) mBuffer.unlock(); }

	void reset()
	{ mBuffer.unlock(); mPtr = 0; }

	template <typename T>
	T *get() const
	{ return reinterpret_cast<T*>(mPtr); }
private:
	IndexBuffer &mBuffer;
	void *mPtr;
};

#endif
