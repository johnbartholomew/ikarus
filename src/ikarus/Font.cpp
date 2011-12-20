#include "Global.h"

#include "Font.h"
#include "Texture.h"
#include "VertexBuffer.h"

#include "FileUtil.h"

namespace // anonymous namespace
{
	const unsigned int kInitialRendererVertexCount = 4*128;

	enum FontFlags
	{
		kFontBold    = 0x10,
		kFontItalic  = 0x20,
		kFontUnicode = 0x40,
		kFontSmooth  = 0x80
	};

	enum CommonBlockFlags
	{
		kBlockPacked  = 0x01,
		kBlockEncoded = 0x02
	};

	enum BlockTypeTags
	{
		kBlockTypeInfo = 1,
		kBlockTypeCommon = 2,
		kBlockTypePages = 3,
		kBlockTypeChars = 4,
		kBlockTypeKerning = 5
	};

#pragma pack(push)
#pragma pack(1)
	struct InfoBlock
	{
		unsigned short fontSize;
		unsigned char flags; // bold, italic, unicode & smooth
		unsigned char charSet;
		unsigned short stretchH;
		unsigned char aa;
		unsigned char paddingUp;
		unsigned char paddingRight;
		unsigned char paddingDown;
		unsigned char paddingLeft;
		unsigned char spacingHoriz;
		unsigned char spacingVert;
		unsigned char outline; // Added with version 2
		char fontName[1];
	};

	struct CommonBlock
	{
		unsigned short lineHeight;
		unsigned short base;
		unsigned short scaleW;
		unsigned short scaleH;
		unsigned short pages;
		unsigned char flags; // packed & encoded (encoded was added with version 2 and removed with version 3)
		unsigned char alphaChnl;
		unsigned char redChnl;
		unsigned char greenChnl;
		unsigned char blueChnl;
	};

	struct PagesBlock
	{
		char pageNames[1];
	};

	struct CharsBlock
	{
		struct CharInfo
		{
			unsigned int id;
			unsigned short x;
			unsigned short y;
			unsigned short width;
			unsigned short height;
			signed short xoffset;
			signed short yoffset;
			signed short xadvance;
			unsigned char page;
			unsigned char chnl;
		} chars[1];
	};

	struct KerningPairsBlock
	{
		struct KerningPair
		{
			unsigned int first;
			unsigned int second;
			signed short amount;
		} kerningPairs[1];
	};
#pragma pack(pop)

	const VertexFormat kFontVertexFormat =
	{
		{VertexAttribute::BindTexCoord0, 2, GL_FLOAT},
		{VertexAttribute::BindVertex,    2, GL_FLOAT},
		{0}
	};

} // end anonymous namespace

Font::Font()
{
	mTexture.reset(new Texture);
	mCharMetrics.reserve(256);
	for (int i = 0; i < 256; ++i)
	{
		CharInfo ci;
		ci.draw = false;
		ci.advance = 0.0f;
		mCharMetrics.push_back(ci);
	}
}

Font::~Font()
{
}

void Font::loadFromFile(const char *fname)
{
	std::ifstream ss(fname, std::ios::in | std::ios::binary);

	std::string dir(fname);
	size_t pos = dir.find_last_of("/\\");
	if (pos != std::string::npos)
		dir = dir.substr(0, pos + 1);
	else
		dir = "";

	// check for a valid header
	char magic[3];
	ReadRaw(ss, magic);
	if (memcmp(magic, "BMF", sizeof(magic)) != 0)
		throw std::runtime_error("Invalid font file (no magic code)");

	unsigned char version;
	ReadRaw(ss, version);
	if (version > 3)
		throw std::runtime_error("Cannot load font file (unsupported file version)");

	while (ss.good())
	{
		unsigned char blockType;
		ReadRaw(ss, blockType);
		if (! ss.good()) break;

		unsigned int blockSize;
		ReadRaw(ss, blockSize);
		if (! ss.good())
			throw std::runtime_error("Cannot load font file (file is truncated; ends after a block type tag)");

		// before version 3, the blockSize included the size field itself
		if (version < 3)
			blockSize -= 4;

		switch (blockType)
		{
		case kBlockTypeInfo:
			loadInfoBlock(ss, blockSize, version);
			break;
		case kBlockTypeCommon:
			loadCommonBlock(ss, blockSize, version);
			break;
		case kBlockTypePages:
			loadPagesBlock(ss, blockSize, version, dir);
			break;
		case kBlockTypeChars:
			loadCharsBlock(ss, blockSize, version);
			break;
		case kBlockTypeKerning:
			loadKerningBlock(ss, blockSize, version);
			break;
		default:
			throw std::runtime_error("Cannot load font file (file contains a block of unknown type)");
			break;
		}
	}
}

void Font::loadInfoBlock(std::istream &ss, unsigned int blockSize, int version)
{
	InfoBlock block;
	
	ReadRaw(ss, block.fontSize);
	ReadRaw(ss, block.flags);
	ReadRaw(ss, block.charSet);
	ReadRaw(ss, block.stretchH);
	ReadRaw(ss, block.aa);
	ReadRaw(ss, block.paddingUp);
	ReadRaw(ss, block.paddingRight);
	ReadRaw(ss, block.paddingDown);
	ReadRaw(ss, block.paddingLeft);
	ReadRaw(ss, block.spacingHoriz);
	ReadRaw(ss, block.spacingVert);
	if (version >= 2)
		ReadRaw(ss, block.outline);

	block.flags &= kFontBold | kFontItalic | kFontUnicode | kFontSmooth;

	// font face name
	unsigned char faceNameLength = blockSize - sizeof(block);
	mFaceName.clear();
	mFaceName.reserve(faceNameLength);

	unsigned int pos = sizeof(block) - 1;
	while (ss.good() && (pos < blockSize))
	{
		char c;
		ReadRaw(ss, c);
		if (c == 0)
			break;
		else
			mFaceName += c;
	}
}

void Font::loadCommonBlock(std::istream &ss, unsigned int blockSize, int version)
{
	CommonBlock block;

	ReadRaw(ss, block.lineHeight);
	ReadRaw(ss, block.base);
	ReadRaw(ss, block.scaleW);
	ReadRaw(ss, block.scaleH);
	ReadRaw(ss, block.pages);
	ReadRaw(ss, block.flags);

	if (version == 1 || version == 3) // encoded was added in version 2 and removed in version 3 (lol?)
		block.flags &= kBlockPacked;
	else if (version == 2)
		block.flags &= kBlockPacked | kBlockEncoded;

	if (version == 3)
	{
		ReadRaw(ss, block.alphaChnl);
		ReadRaw(ss, block.redChnl);
		ReadRaw(ss, block.greenChnl);
		ReadRaw(ss, block.blueChnl);
	}

	mLineHeight = static_cast<float>(block.lineHeight);
	mBase = static_cast<float>(block.base);
}

void Font::loadPagesBlock(std::istream &ss, unsigned int blockSize, int version, const std::string &baseDir)
{
	// load the texture for the first page
	// FIXME: support multiple page fonts??

	PagesBlock block;

	unsigned int fnameLength = blockSize - sizeof(block);
	std::string fname;
	fname.reserve(fnameLength);

	unsigned int pos = 0;
	while (ss.good() && (pos < blockSize))
	{
		char c;
		ReadRaw(ss, c);
		pos += 1;
		if (c == 0)
			break;
		else
			fname += c;
	}

	if (pos != blockSize)
		throw std::runtime_error("Cannot load font file (pages block is truncated or has an incorrect blockSize)");

	mTexture->loadFromFile((baseDir + fname).c_str(), false, Texture::FormatAlpha);
}

void Font::loadCharsBlock(std::istream &ss, unsigned int blockSize, int version)
{
	CharsBlock::CharInfo c;

	vec2i texSize = mTexture->getSize();

	unsigned int pos = 0;
	while (ss.good() && (pos < blockSize))
	{
		if (version < 3)
		{
			unsigned short ids;
			ReadRaw(ss, ids);
			c.id = ids;
		}
		else
			ReadRaw(ss, c.id);
		ReadRaw(ss, c.x);
		ReadRaw(ss, c.y);
		ReadRaw(ss, c.width);
		ReadRaw(ss, c.height);
		ReadRaw(ss, c.xoffset);
		ReadRaw(ss, c.yoffset);
		ReadRaw(ss, c.xadvance);
		ReadRaw(ss, c.page);
		ReadRaw(ss, c.chnl);

		if (version < 3)
			pos += 18;
		else
			pos += 20;

		if (c.id >= 256)
			continue;

		CharInfo &info = mCharMetrics[c.id];

		info.draw = true;
		info.advance = static_cast<float>(c.xadvance + 1);
		info.posTopLeft = vec2f(
			static_cast<float>(c.xoffset),
			static_cast<float>(c.yoffset)
		);
		info.posBottomRight = vec2f(
			static_cast<float>(c.xoffset + c.width),
			static_cast<float>(c.yoffset + c.height)
		);
		info.texTopLeft = vec2f(
			static_cast<float>(c.x) / static_cast<float>(texSize.x),
			static_cast<float>(c.y) / static_cast<float>(texSize.y)
		);
		info.texBottomRight = vec2f(
			static_cast<float>(c.x + c.width ) / static_cast<float>(texSize.x),
			static_cast<float>(c.y + c.height) / static_cast<float>(texSize.y)
		);
	}

	if (pos != blockSize)
		throw std::runtime_error("Cannot load font file (chars block is truncated or has an incorrect blockSize)");

	// hard-code for space and tab
	mCharMetrics[' '].draw = false;
	mCharMetrics['\t'].draw = false;
	mCharMetrics['\t'].advance = 4.0f * mCharMetrics[' '].advance;
}

void Font::loadKerningBlock(std::istream &ss, unsigned int blockSize, int version)
{
	KerningPairsBlock::KerningPair info;

	unsigned int pos = 0;
	while (ss.good() && (pos < blockSize))
	{
		if (version < 3)
		{
			unsigned short ids;
			ReadRaw(ss, ids);
			info.first = ids;
			ReadRaw(ss, ids);
			info.second = ids;
		}
		else
		{
			ReadRaw(ss, info.first);
			ReadRaw(ss, info.second);
		}
		
		ReadRaw(ss, info.amount);

		if (version < 3)
			pos += 6;
		else
			pos += 10;

		if (info.first >= 256 || info.second >= 256)
			continue;

		mKerningPairs.insert(std::make_pair(
			CharPair(static_cast<char>(info.first), static_cast<char>(info.second)),
			static_cast<float>(info.amount)
		));
	}

	if (pos != blockSize)
		throw std::runtime_error("Cannot load font file (kerning block is truncated or has an incorrect blockSize)");
}

const Texture *Font::getTexture() const
{
	return mTexture.get();
}

// ----- TextRenderer ---------------------------------------------------------

TextRenderer::TextRenderer()
:	mFont(0), mUseKerning(true), mWidth(0.0f), mHeight(1.0f), mNumVerts(0)
{
}

TextRenderer::~TextRenderer()
{
}

void TextRenderer::drawText(const Font *font, const std::string &text, bool use_kerning)
{
	if (text.empty())
		return;

	init(font, text, use_kerning);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mFont->getTexture()->bind();
	mVerts->bind();
	mVerts->draw(GL_QUADS, mNumVerts, 0);
}

vec2f TextRenderer::measureText(const Font *font, const std::string &text, bool use_kerning)
{
	if (text.empty())
		return vec2f(0.0, 0.0);
	init(font, text, use_kerning);
	return vec2f(mWidth, mHeight);
}

int TextRenderer::getStringIndexAt(float test_x, const Font *font, const std::string &text, bool use_kerning)
{
	float x = 0.0f, next_x;

	char prev_char = 0;
	for (size_t i = 0; i < text.size(); ++i)
	{
		char c = text[i];

		if (c == '\r' || c == '\n')
		{
			// newline breaks up kerning pairs
			prev_char = 0;
			continue;
		}
		
		// Get the normal character information
		const Font::CharInfo &ci = mFont->getCharInfo(c);

		// Get the kerning pair offset, if there is one
		float kerning_offset = 0.0f;

		if (mUseKerning && (prev_char != 0))
			kerning_offset = mFont->getKerningOffset(prev_char, c);
		
		prev_char = c;

		// apply kerning
		x += kerning_offset;
		next_x = x + ci.advance;

		float w = next_x - x;
		float a = x + w/2.0f;
		if ((test_x >= x) && (test_x < a))
			return static_cast<int>(i);
		else if ((test_x >= a) && (test_x <= next_x))
			return static_cast<int>(i + 1);

		x = next_x;
	}

	return -1;
}

void TextRenderer::init(const Font *font, const std::string &text, bool use_kerning)
{
	if ((font != mFont) || (text != mText) || (use_kerning != mUseKerning))
	{
		unsigned int curVertCount = (!mVerts) ? 0 : mVerts->getNumVertices();
		unsigned int reqVertCount = std::max(kInitialRendererVertexCount, static_cast<unsigned int>(text.size()) * 4);
		if (reqVertCount > curVertCount)
			mVerts.reset(new VertexBuffer(reqVertCount, kFontVertexFormat, GL_STREAM_DRAW_ARB, false));

		mFont = font;
		mUseKerning = use_kerning;
		mText = text;

		VertexBufferLock lock(*mVerts);
		float *v = lock.get<float>();

		float x = 0.0f, y = 0.0f;

		mWidth = 0.0f;
		mHeight = 0.0f;

		mNumVerts = 0;
		char prev_char = 0;
		for (size_t i = 0; i < mText.size(); ++i)
		{
			char c = mText[i];

			if (c == '\r' || c == '\n')
			{
				if (i == mText.size() - 1)
					break;

				if (mText[i + 1] == '\r' || mText[i + 1] == '\n')
					++i;

				// newline breaks up kerning pairs
				prev_char = 0;
				
				mWidth = std::max(mWidth, x);

				x = 0.0f;
				y += mFont->getLineHeight();

				continue;
			}

			// Get the normal character information
			const Font::CharInfo &ci = mFont->getCharInfo(c);

			// Get the kerning pair offset, if there is one
			float kerning_offset = 0.0f;

			if (mUseKerning && (prev_char != 0))
				kerning_offset = mFont->getKerningOffset(prev_char, c);

			prev_char = c;

			x += kerning_offset;

			if (! ci.draw)
			{
				x += ci.advance;
				continue;
			}

			// Add the vertices

			// TopLeft
			*v++ = ci.texTopLeft[0]; // u
			*v++ = ci.texTopLeft[1]; // v
			*v++ = x + ci.posTopLeft[0]; // x
			*v++ = y + ci.posTopLeft[1]; // y

			// BottomLeft
			*v++ = ci.texTopLeft[0]; // u
			*v++ = ci.texBottomRight[1]; // v
			*v++ = x + ci.posTopLeft[0]; // x
			*v++ = y + ci.posBottomRight[1]; // y
			
			// BottomRight
			*v++ = ci.texBottomRight[0]; // u
			*v++ = ci.texBottomRight[1]; // v
			*v++ = x + ci.posBottomRight[0]; // x
			*v++ = y + ci.posBottomRight[1]; // y

			// TopRight
			*v++ = ci.texBottomRight[0]; // u
			*v++ = ci.texTopLeft[1]; // v
			*v++ = x + ci.posBottomRight[0]; // x
			*v++ = y + ci.posTopLeft[1]; // y

			x += ci.advance;

			mNumVerts += 4;
		}

		lock.reset();

		mWidth = std::max(x, mWidth);
		mHeight = y + mFont->getLineHeight();
	}
}

// ----- Text -----------------------------------------------------------------

Text::Text(): mNumVerts(0), mWidth(0.0f), mHeight(0.0f)
{
}

Text::Text(RCPtr<Font> fnt, const std::string &str, bool use_kerning): mNumVerts(0), mWidth(0.0f), mHeight(0.0f)
{
	init(fnt, str, use_kerning);
}

Text::~Text()
{
}

void Text::init(RCPtr<Font> fnt, const std::string &str, bool use_kerning)
{
	assert(fnt.get());
	assert(! str.empty());

	mFont = fnt;

	mVerts.reset(new VertexBuffer((unsigned int)str.size() * 4, kFontVertexFormat, GL_STREAM_DRAW_ARB, false));

	VertexBufferLock lock(*mVerts);
	float *v = lock.get<float>();

	float x = 0.0f, y = 0.0f;

	mWidth = 0.0f;
	mHeight = 0.0f;

	mNumVerts = 0;
	char prev_char = 0;
	for (size_t i = 0; i < str.size(); ++i)
	{
		char c = str[i];

		if (c == '\r' || c == '\n')
		{
			++i;

			if (i < str.size() && str[i] == '\r' || str[i] == '\n')
				++i;
			
			if (i == str.size())
				break;

			// newline breaks up kerning pairs
			prev_char = 0;
			
			mWidth = std::max(mWidth, x);

			x = 0.0f;
			y += mFont->getLineHeight();

			continue;
		}

		// Get the normal character information
		const Font::CharInfo &ci = mFont->getCharInfo(c);

		// Get the kerning pair offset, if there is one
		float kerning_offset = 0.0f;

		if (use_kerning && (prev_char != 0))
			kerning_offset = mFont->getKerningOffset(prev_char, c);

		prev_char = c;

		x += kerning_offset;

		if (! ci.draw)
		{
			x += ci.advance;
			continue;
		}

		// Add the vertices

		// TopLeft
		*v++ = ci.texTopLeft[0]; // u
		*v++ = ci.texTopLeft[1]; // v
		*v++ = x + ci.posTopLeft[0]; // x
		*v++ = y + ci.posTopLeft[1]; // y

		// BottomLeft
		*v++ = ci.texTopLeft[0]; // u
		*v++ = ci.texBottomRight[1]; // v
		*v++ = x + ci.posTopLeft[0]; // x
		*v++ = y + ci.posBottomRight[1]; // y
		
		// BottomRight
		*v++ = ci.texBottomRight[0]; // u
		*v++ = ci.texBottomRight[1]; // v
		*v++ = x + ci.posBottomRight[0]; // x
		*v++ = y + ci.posBottomRight[1]; // y

		// TopRight
		*v++ = ci.texBottomRight[0]; // u
		*v++ = ci.texTopLeft[1]; // v
		*v++ = x + ci.posBottomRight[0]; // x
		*v++ = y + ci.posTopLeft[1]; // y

		x += ci.advance;

		mNumVerts += 4;
	}

	lock.reset();

	mWidth = std::max(x, mWidth);
	mHeight = y + mFont->getLineHeight();
}

void Text::print()
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	mFont->getTexture()->bind();
	mVerts->bind();
	mVerts->draw(GL_QUADS, mNumVerts, 0);
}
