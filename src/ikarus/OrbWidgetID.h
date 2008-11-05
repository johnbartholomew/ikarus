#ifndef ORB_WIDGET_ID_H
#define ORB_WIDGET_ID_H


// a WidgetID is an immutable unique identifier for a widget
struct WidgetID
{
public:
	WidgetID()
		: name("__null"), data(0), idx(0) { makeHash(); }

	WidgetID(const WidgetID &wid)
		: name(wid.name), data(wid.data), idx(wid.idx), hash(wid.hash) {}

	// WidgetID has a constructors for each combination of identifying information
	// however, most of the time you won't construct a WidgetID yourself (that'll be left to the gui widget function)

	explicit WidgetID(const std::string &name)
		: name(name), data(0), idx(0) { makeHash(); }
	explicit WidgetID(const char *name)
		: name(name), data(0), idx(0) { makeHash(); }

	explicit WidgetID(int idx)
		: name("__idxonly"), data(0), idx(idx) { makeHash(); }

	template<typename T>
	explicit WidgetID(T *data)
		: name("__dataonly"), data(static_cast<void*>(data)), idx(0) { makeHash(); }

	explicit WidgetID(const std::string &name, int idx)
		: name(name), data(0), idx(idx) { makeHash(); }
	explicit WidgetID(const char *name, int idx)
		: name(name), data(0), idx(idx) { makeHash(); }

	template<typename T>
	explicit WidgetID(const std::string &name, T *data)
		: name(name), data(static_cast<void*>(data)), idx(0) { makeHash(); }
	template<typename T>
	explicit WidgetID(const char *name, T *data)
		: name(name), data(static_cast<void*>(data)), idx(0) { makeHash(); }

	template<typename T>
	explicit WidgetID(const std::string &name, T *data, int idx)
		: name(name), data(static_cast<void*>(data)), idx(0) { makeHash(); }
	template<typename T>
	explicit WidgetID(const char *name, T *data, int idx)
		: name(name), data(static_cast<void*>(data)), idx(0) { makeHash(); }

	WidgetID &operator=(const WidgetID &wid)
	{
		name = wid.name;
		data = wid.data;
		idx = wid.idx;
		hash = wid.hash;
	}

	bool operator==(const WidgetID &wid) const
	{
		if (hash != wid.hash) return false; // early-out
		if (idx != wid.idx) return false;
		if (data != wid.data) return false;
		if (name != wid.name) return false;
	}

	bool operator!=(const WidgetID &wid) const
	{ return !(*this == wid); }

	unsigned int getHash() const
	{ return hash; }

private:
	void makeHash();

	std::string name;
	const void *data;
	int idx;

	unsigned int hash;
};


#endif
