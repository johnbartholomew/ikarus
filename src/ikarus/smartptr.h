#ifndef SMART_PTR_H
#define SMART_PTR_H

/* There are occasionally situations where I don't want to use boost,
 * but I also don't want to live without smart pointers (they're easily
 * the most useful part of boost), so here are some simple re-implementations */

template <typename T>
struct ScopedPtr
{
public:
	ScopedPtr(): p(0) {}
	template <typename Y>
	explicit ScopedPtr(Y *p): p(p) {}
	~ScopedPtr() { delete p; }

	void reset()
	{ ScopedPtr<T>().swap(*this); }

	template <typename Y>
	void reset(Y *np)
	{ ScopedPtr<T>(np).swap(*this); }

	T *get() const
	{ return p; }

	T *operator->() const
	{ return p; }

	T &operator*() const
	{ return *p; }

	operator bool() const
	{ return (p != 0); }

	bool operator!() const
	{ return (p == 0); }
private:
	// we're supposed to be the only owner, so make this non-copyable and non-assignable
	// you can still assign to a raw/weak pointer
	ScopedPtr(const ScopedPtr<T> &from);
	ScopedPtr<T> &operator=(const ScopedPtr<T> &from);

	void swap(ScopedPtr<T> &b) throw()
	{
		T *tmp = p;
		p = b.p;
		b.p = tmp;
	}

	T *p;
};

template <typename T>
struct ScopedArray
{
public:
	ScopedArray(): p(0) {}
	template <typename Y>
	explicit ScopedArray(Y *p): p(p) {}
	~ScopedArray() { delete[] p; }	
	
	void reset()
	{ ScopedArray<T>().swap(*this); }

	template <typename Y>
	void reset(Y *np)
	{ ScopedArray<T>(np).swap(*this); }

	T &operator[](int i)
	{ return p[i]; }
	const T &operator[](int i) const
	{ return p[i]; }

	T *get()
	{ return p; }

	const T *get() const
	{ return p; }

	operator bool() const
	{ return (p != 0); }

	bool operator!() const
	{ return (p == 0); }
private:
	ScopedArray(const ScopedArray<T> &);
	ScopedArray<T> &operator=(const ScopedArray<T> &);

	void swap(ScopedArray<T> &b) throw()
	{
		T *tmp = p;
		p = b.p;
		b.p = tmp;
	}

	T *p;
};

class RefCounted
{
public:
	RefCounted(): rc(0) {}
	virtual ~RefCounted() {}

	void AddRef()
	{ ++rc; }
	void Release()
	{ if (!--rc) delete this; }
private:
	unsigned int rc;
};

struct dynamic_cast_tag {};
struct static_cast_tag {};

template <class T>
class RCPtr
{
public:
	RCPtr(): p(0) {}

	template <typename Y>
	explicit RCPtr(Y *p_): p(p_) { if (p) p->AddRef(); }

	template <typename Y>
	explicit RCPtr(const RCPtr<Y> &p_): p(p_.p) { if (p) p->AddRef(); }

	~RCPtr()
	{ if (p) p->Release(); }

	template <typename Y>
	RCPtr<T> &operator = (const RCPtr<Y> &p_)
	{ RCPtr<T>(p_).swap(*this); return *this; }

	void reset()
	{ RCPtr<T>().swap(*this); }

	template <typename Y>
	void reset(Y *p_)
	{ RCPtr<T>(p_).swap(*this); }

	T *get() const { return p; }
	T *operator -> () const { return p; }
	T &operator * () const { return *p; }

	operator bool() const
	{ return (p != 0); }

	bool operator!() const
	{ return (p == 0); }
private:
	T *p;
	void swap(RCPtr<T> &b) throw()
	{
		T *tmp = p;
		p = b.p;
		b.p = tmp;
	}
};

#endif
