#ifndef OPENGL_CONTEXT_H
#define OPENGL_CONTEXT_H

// some typedefs from windows.h so we don't have to bring in that huge header
typedef struct HWND__ *HWND;
typedef struct HDC__ *HDC;
typedef struct HGLRC__ *HGLRC;

class RenderTarget;

class OpenGLContext
{
public:
	explicit OpenGLContext();
	~OpenGLContext();

	void open(RenderTarget *target);
	void open(HDC baseDevice);
	void close();

	void bind();
	void bind(RenderTarget *target);
	void swapBuffers() const;

	static void Share(OpenGLContext &a, OpenGLContext &b);
private:
	RenderTarget *mTarget;
	HGLRC mContext;
};

class RenderTarget
{
public:
	explicit RenderTarget();
	virtual ~RenderTarget();

	virtual HDC getDevice() const = 0;
};

class WindowRenderTarget : public RenderTarget
{
public:
	explicit WindowRenderTarget();
	virtual ~WindowRenderTarget();

	void open(HWND window, int depthBufferBitDepth = 16);
	void close();

	virtual HDC getDevice() const;
private:
	HWND mWindow;
	HDC mDevice;
};

#endif
