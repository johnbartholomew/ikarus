#ifndef ORB_WIDGET_ID_H
#define ORB_WIDGET_ID_H


// a WidgetID is an immutable unique identifier for a widget
struct WidgetID
{
public:
	static const WidgetID NullWID;

	WidgetID()
		: name("__null"), data(0), idx(0) { makeHash(); }

	WidgetID(const WidgetID &wid)
		: name(wid.name), data(wid.data), idx(wid.idx), hash(wid.hash) {}

	// WidgetID has a constructors for each combination of identifying information

	// these constructors are deliberately not explicit, because I want to be able to use strings, indexes and pointers directly as IDs

	WidgetID(const std::string &name)
		: name(name), data(0), idx(0) { makeHash(); }
	WidgetID(const char *name)
		: name(name ? name : ""), data(0), idx(0) { makeHash(); }

	WidgetID(int idx)
		: name("__idxonly"), data(0), idx(idx) { makeHash(); }

	template<typename T>
	WidgetID(T *data)
		: name("__dataonly"), data(static_cast<void*>(data)), idx(0) { makeHash(); }

	WidgetID(const std::string &name, int idx)
		: name(name), data(0), idx(idx) { makeHash(); }
	WidgetID(const char *name, int idx)
		: name(name ? name : ""), data(0), idx(idx) { makeHash(); }

	template<typename T>
	WidgetID(const std::string &name, T *data)
		: name(name), data(static_cast<void*>(data)), idx(0) { makeHash(); }
	template<typename T>
	WidgetID(const char *name, T *data)
		: name(name ? name : ""), data(static_cast<void*>(data)), idx(0) { makeHash(); }

	template<typename T>
	WidgetID(const std::string &name, T *data, int idx)
		: name(name), data(static_cast<void*>(data)), idx(0) { makeHash(); }
	template<typename T>
	WidgetID(const char *name, T *data, int idx)
		: name(name ? name : ""), data(static_cast<void*>(data)), idx(0) { makeHash(); }

	WidgetID &operator=(const WidgetID &wid)
	{
		name = wid.name;
		data = wid.data;
		idx = wid.idx;
		hash = wid.hash;
		return *this;
	}

	bool operator==(const WidgetID &wid) const
	{
		if (hash != wid.hash) return false; // early-out
		if (idx != wid.idx) return false;
		if (data != wid.data) return false;
		if (name != wid.name) return false;
		return true;
	}

	bool operator!=(const WidgetID &wid) const
	{ return !(*this == wid); }

	unsigned int getHash() const
	{ return hash; }

	bool isNull() const
	{ return (*this == NullWID); }

private:
	void makeHash();

	std::string name;
	const void *data;
	int idx;

	unsigned int hash;
};


#endif
