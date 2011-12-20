#include "Global.h"
#include "Texture.h"

#include <SOIL.h>

namespace
{
	int formatChannels(Texture::TextureFormat format)
	{
		switch (format)
		{
			case Texture::FormatAuto: return SOIL_LOAD_AUTO;
			case Texture::FormatAlpha: return 1;
			case Texture::FormatLuminance: return 1;
			case Texture::FormatLuminanceAlpha: return 2;
			case Texture::FormatRGB: return 3;
			case Texture::FormatRGBA: return 4;
			default: assert(0); return SOIL_LOAD_AUTO;
		}
	}

	// from http://graphics.stanford.edu/~seander/bithacks.html
	int nextPowerOfTwo(int n)
	{
		--n;
		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;
		++n;
		return n;
	}
}

// ----- Texture --------------------------------------------------------------

Texture::Texture(): mID(0)
{
}

Texture::~Texture()
{
	if (mID)
		glDeleteTextures(1, &mID);
}

void Texture::loadFromFile(const char *fname, bool generate_mip_maps, Texture::TextureFormat format)
{
	assert(fname);
	assert(! mID);

	int w, h, c;
	unsigned char *data = SOIL_load_image(fname, &w, &h, &c, formatChannels(format));

	assert((format == FormatAuto) || (c == formatChannels(format)));

	try
	{
		mSize = vec2i(w, h);
		mID = SOIL_create_OGL_texture(data, w, h, formatChannels(format), SOIL_CREATE_NEW_ID, generate_mip_maps ? SOIL_FLAG_MIPMAPS : 0);
	}
	catch (...)
	{
		SOIL_free_image_data(data);
		throw;
	}

	assert(mID);
}
