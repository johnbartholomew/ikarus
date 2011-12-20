#ifndef FONT_H
#define FONT_H

class Texture;

/// Represents an in-memory font.
/// Expects font files generated by AngelCode's BMFont utility.
/// This class only stores the font metrics and maintains ownership of the font's texture/glyph data:
/// A Text object or TextRenderer object can be used to render text using a Font.
class Font : public RefCounted
{
public:
	struct CharInfo
	{
		CharInfo(): draw(false), advance(0.0f) {}
		bool draw;
		vec2f texTopLeft;
		vec2f texBottomRight;
		vec2f posTopLeft;
		vec2f posBottomRight;
		float advance;
	};

	Font();
	~Font();

	/// Loads the font metrics and texture.
	/// @attention Expects the font's texture file to be in the same directory as the font metrics file.
	/// @param fname The path to the font metrics/definition file generated by BMFont.
	void loadFromFile(const char *fname);

	/// @return The Texture object that holds the font's bitmap/glyph data, or null if the font hasn't been loaded yet.
	const Texture *getTexture() const;

	/// @return The height of a line of text in this font, or an undefined value if the font hasn't been loaded yet.
	float getLineHeight() const
	{ return mLineHeight; }

	/// @return The face name of the font (eg, "Arial" or "Courier New") or the empty string if the font hasn't been loaded yet.
	const std::string &getFaceName() const
	{ return mFaceName; }

	/// @return Metrics/glyph position for a particular character in the font.
	const CharInfo &getCharInfo(char c) const
	{ return mCharMetrics.at(c); }

	/// @return the kerning offset for a pair of characters in this font.
	float getKerningOffset(char a, char b) const
	{
		std::map<CharPair, float>::const_iterator it = mKerningPairs.find(CharPair(a, b));
		if (it == mKerningPairs.end())
			return 0.0f;
		else
			return it->second;
	}
private:
	void loadInfoBlock(std::istream &ss, unsigned int blockSize, int version);
	void loadCommonBlock(std::istream &ss, unsigned int blockSize, int version);
	void loadPagesBlock(std::istream &ss, unsigned int blockSize, int version, const std::string &baseDir);
	void loadCharsBlock(std::istream &ss, unsigned int blockSize, int version);
	void loadKerningBlock(std::istream &ss, unsigned int blockSize, int version);

	struct CharPair
	{
		char first;
		char second;

		CharPair(char a, char b): first(a), second(b) {}
		bool operator < (const CharPair &b) const
		{ return ((first << 8) | (second & 0xf)) < ((b.first << 8) | (second & 0xf)); }
	};

	std::vector<CharInfo> mCharMetrics;
	std::map<CharPair, float> mKerningPairs;
	ScopedPtr<Texture> mTexture;
	float mLineHeight;
	float mBase;
	float mTexWidth;
	float mTexHeight;
	std::string mFaceName;
};

class VertexBuffer;
/// A TextRenderer maintains the necessary OpenGL state and objects (read: a vertex buffer) to render text using an arbitrary font.
/// A new vertex buffer is created whenever the current one doesn't have space to hold all the character vertices for the piece of text being rendered.
/// If you're trying to render an unusually large quantity of text in one call, consider using a separate TextRenderer
/// rather than the one you use for all the normal text snippets, otherwise you'll be keeping an unnecessarily large vertex buffer around.
class TextRenderer
{
public:
	TextRenderer();
	~TextRenderer();

	void drawText(const Font *font, const std::string &text, bool use_kerning = true);
	vec2f measureText(const Font *font, const std::string &text, bool use_kerning = true);
	int getStringIndexAt(float x, const Font *font, const std::string &text, bool use_kerning = true);

private:
	void init(const Font *font, const std::string &text, bool use_kerning);

	std::string mText;
	const Font *mFont;
	bool mUseKerning;
	float mWidth;
	float mHeight;
	unsigned int mNumVerts;

	ScopedPtr<VertexBuffer> mVerts;
};

/// Represents a piece of text in a given font.  If there's a piece of text that you want to render repeatedly, consider creating a Text object so that you don't have
/// to keep processing the text and generating the same vertex data over and over again.
class Text
{
public:
	Text();
	Text(RCPtr<Font> fnt, const std::string &str, bool use_kerning = true);
	~Text();

	void init(RCPtr<Font> fnt, const std::string &str, bool use_kerning = true);

	void print();
	float width() const
	{ return mWidth; }
	float height() const
	{ return mHeight; }
private:
	float mWidth;
	float mHeight;
	unsigned int mNumVerts; // not the same as m_Verts->GetNumVertices(), because of invisible space characters
	RCPtr<Font> mFont;
	ScopedPtr<VertexBuffer> mVerts;
};

#endif
