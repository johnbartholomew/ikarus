#ifndef REF_VECTOR_H
#define REF_VECTOR_H

#include <vector>
#include <cassert>
#include <memory>

namespace detail
{
	template <typename T>
	class const_iterator
	{
	public:
		typedef const_iterator<T> this_type;
		typedef T value_type;
		typedef const T &const_reference;

		const_reference operator*() const
		{
			return *(*it);
		}

		this_type &operator++()
		{
			++it;
			return (*this);
		}

		this_type &operator++(int)
		{
			this_type x(*this);
			++it;
			return x;
		}

		this_type &operator--()
		{
			--it;
			return (*this);
		}

		this_type &operator--(int)
		{
			this_type x(*this);
			--it;
			return x;
		}

		bool operator==(const const_iterator &other)
		{ return (it == other.it); }

		bool operator!=(const const_iterator &other)
		{ return (it != other.it); }

		bool operator<(const const_iterator &other)
		{ return (it < other.it); }

		bool operator>(const const_iterator &other)
		{ return (it > other.it); }

		bool operator<=(const const_iterator &other)
		{ return (it <= other.it); }

		bool operator>=(const const_iterator &other)
		{ return (it >= other.it); }
	protected:
		typename ::std::vector<T * const>::const_iterator it;
	};

	template <typename T>
	class iterator : public const_iterator<T>
	{
	public:
		typedef iterator<T> this_type;
		typedef T &reference;

		reference operator*() const
		{ return *(*(this->it)); }
	};
}

template <typename T>
class refvector
{
private:
	typedef typename std::vector<T*> vector_type;
public:
	typedef typename vector_type::size_type size_type;
	typedef T value_type;
	typedef T &reference;
	typedef const T &const_reference;
	typedef detail::const_iterator<T> const_iterator;
	typedef detail::iterator<T> iterator;

	refvector() {}
	~refvector()
	{
		clear();
	}

	void clear()
	{
		typename vector_type::iterator it = mItems.begin();
		while (it != mItems.end())
		{
			delete (*it);
			++it;
		}
		mItems.clear();
	}

	bool empty() const
	{ return mItems.empty(); }

	size_type size() const
	{ return mItems.size(); }

	const_iterator begin() const
	{ return const_iterator(mItems.begin()); }

	iterator begin()
	{ return iterator(mItems.begin()); }

	const_iterator end() const
	{ return const_iterator(mItems.end()); }

	iterator end()
	{ return iterator(mItems.end()); }

	template <typename Y>
	iterator insert(iterator pos, Y * const y)
	{
		assert(y != 0);
		iterator(mItems.insert(pos, y));
	}

	iterator erase(iterator it)
	{
		delete (&(*it));
		return iterator(mItems.erase(it));
	}

	const_reference front() const
	{
		return *(mItems.front());
	}

	reference front()
	{
		return *(mItems.front());
	}

	const_reference back() const
	{
		return *(mItems.back());
	}

	reference back()
	{
		return *(mItems.back());
	}

	template <typename Y>
	void push_back(std::auto_ptr<Y> &y)
	{
		assert(y.get() != 0);
		mItems.push_back(y.release());
	}

	template <typename Y>
	void push_back(Y * const y)
	{
		assert(y != 0);
		mItems.push_back(y);
	}

	void pop_back()
	{
		delete (mItems.back());
		mItems.pop_back();
	}

	const_reference operator[](size_type idx) const
	{ return *(mItems[idx]); }

	reference operator[](size_type idx)
	{ return *(mItems[idx]); }

	template <typename Y>
	void reset_at(size_type idx, Y * const y)
	{
		assert(y != 0);
		T *tmp = mItems[idx];
		mItems[idx] = y;
		delete tmp;
	}

	void reserve(size_type n)
	{ mItems.reserve(n); }
private:
	// non-copyable
	refvector(const refvector<T> &other);
	// non-assignable
	refvector<T> &operator=(const refvector<T> &other);

	vector_type mItems;
};

#endif
