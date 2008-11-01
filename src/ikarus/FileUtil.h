#ifndef FILE_UTIL_H
#define FILE_UTIL_H

/// Extracts the extension of a filename, returns a lowercase version of everything from the last '.' onwards (including the dot).
/// nb: This function transforms the returned string to lowercase, so you can then use a simple test to check for particular file types based on the extension (without having to deal with case-insensitive comparisons).
/// @code
/// GetFileExt("hello.png") == ".png"
/// @endcode
inline std::string GetFileExt(const std::string &fname)
{
	std::string ext = fname.substr(fname.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), &::tolower);
	return ext;
}

/// Extracts the directory of a filename, including trailing slash
/// Can be used to get a base directory name from a filename to create relative filenames
inline std::string GetFileDirectory(const std::string &fname)
{	
	std::string dir(fname);
	// normalise slashes to forward slashes
	for (int i = 0; i < (int)dir.size(); ++i)
		if (dir[i] == '\\')
			dir[i] = '/';
	return dir.substr(0, dir.find_last_of('/') + 1);
}

/// Reads a value of any type from a std::istream as raw data.
/// nb: Be careful when using this - it should only be used to read simple types or for bits of quick-hack code that will be fixed later.
/// Improper usage may cause problems due to padding in data structures, or endianness issues.
template <typename T>
void ReadRaw(std::istream &stream, T &obj)
{
	const std::streamsize N = sizeof(T);
	stream.read(reinterpret_cast<char*>(&obj), N);
}

/// Reads in a static array of data of any type from a std::istream.
/// Ok for things like reading in constant-length strings, but as with the other ReadRaw, care must be taken.
/// @code
/// char magic_code[3]; // magic file code should be "PNG"
/// ReadRaw(a_stream, magic_code);
/// if (memcmp(magic_code, "PNG", sizeof(magic_code)) != 0)
///     throw Error();
/// @endcode
template <typename T, size_t n>
void ReadRaw(std::istream &stream, T (&obj)[n])
{
	stream.read(reinterpret_cast<char*>(&obj[0]), sizeof(obj[0]) * n);
}

/// Reads in an arbitrary sized integer type from a stream, interpreting the data in little-endian byte order.
template <typename T>
void ReadIntLE(std::istream &stream, T &n)
{
	n = 0;
	unsigned char x;
	for (int i = 0; i < sizeof(T); ++i)
	{
		stream.read(reinterpret_cast<char*>(&x), 1);
		n = n | (static_cast<T>(x) << (i*8));
	}
}

/// Reads in an arbitrary sized integer type from a stream, interpreting the data in big-endian byte order.
template <typename T>
inline void ReadIntBE(std::istream &stream, T &n)
{
	n = 0;
	unsigned char x;
	for (int i = 0; i < sizeof(T); ++i)
	{
		n <<= 8;
		stream.read(reinterpret_cast<char*>(&x), 1);
		n |= x;
	}
}

/// Reads in an entire file and returns the file's contents as a std::string.
/// @attention The file is read in @em binary mode, not text mode, so newlines will not be normalized.
/// (the reason for reading in binary mode is due to the specification of std::stream::tellg() which is only guaranteed to
/// give a position as a byte-count into the file when the file is in binary mode: it's stupid but there it is).
inline std::string ReadFileAsString(const char *fname)
{
	std::ifstream fl(fname, std::ios::in | std::ios::binary);
	if (! fl.good())
	{
		std::ostringstream ss;
		ss << "Could not open file '" << fname << "'.";
		throw std::runtime_error(ss.str());
	}
	fl.seekg(0, std::ios::end);
	std::streamsize len = fl.tellg();
	fl.seekg(0, std::ios::beg);

	std::string data(len, '\0');
	fl.read(&data[0], len);
	fl.close();

	return data;
}

#endif
