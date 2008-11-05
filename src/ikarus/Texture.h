#ifndef TEXTURE_H
#define TEXTURE_H

/// Represents an OpenGL texture object.
/// If loading from a file, the resulting texture object may be larger than the original image size due to the power-of-two restriction on texture sizes.
/// The Texture class keeps track of the original image size, so it knows what section of the texture object actually contains image data.
/// You should therefore use the getTexCoordRight() and getTexCoordBottom() functions to get the maximum texture U,V coordinates within the texture,
/// rather than just assuming that U and V both range from 0 to 1.
class Texture
{
public:
	// nb: equal to the SOIL loading format enum
	// if this changes, then you must add some mapping from this to the SOIL format specifiers (or change SOIL to match)
	enum TextureFormat
	{
		FormatAuto = 0,
		FormatAlpha = 1,
		FormatLuminance = 2,
		FormatLuminanceAlpha = 3,
		FormatRGB = 4,
		FormatRGBA = 5
	};

	Texture();
	~Texture();

	void bind() const
	{ glBindTexture(GL_TEXTURE_2D, mID); }

	static void Unbind()
	{ glBindTexture(GL_TEXTURE_2D, 0); }

	void loadFromFile(const char *fname, bool generate_mip_maps = false, TextureFormat format = FormatAuto);

	vec2i getSize() const
	{ return mSize; }

	GLuint getOpenGLID() const
	{ return mID; }

	bool isLoaded() const
	{ return (mID != 0); }
private:
	GLuint mID;
	vec2i mSize;
};

#endif
