#pragma once

#include <type_traits>

/// This class helps create better property widgets
/// for vectors of values that behave as arrays
template <class T>
class VectorWrapper
{
public:
	using vector_type = std::decay_t<T>;
	using value_type = typename T::value_type;
	using wrapper_type = VectorWrapper<vector_type>;
	VectorWrapper() {}
	VectorWrapper(const vector_type& value) : m_value(value) {}
	VectorWrapper(vector_type&& value) : m_value(std::move(value)) {}

	operator vector_type() const { return m_value; }
	void operator=(const vector_type& val) { m_value = val; }

	const vector_type& value() const
	{ return m_value; }
	vector_type& value()
	{ return m_value; }

	bool operator==(const wrapper_type& rhs)
	{ return m_value == rhs.m_value; }
	bool operator!=(const wrapper_type& rhs)
	{ return m_value != rhs.m_value; }

	const value_type& get(int row, int column) const
	{ return m_value[row * m_columnCount + column]; }
	void set(int row, int column, const value_type& val)
	{ m_value[row * m_columnCount + column] = val; }

	int columnCount() const // Length of an array
	{ return m_columnCount; }
	void setColumnCount(int count)
	{ m_columnCount = count; }

	int rowCount() const
	{ return m_value.size() / m_columnCount; }
	void setRowCount(int count)
	{ m_value.resize(count * m_columnCount); }

	bool fixedSize() const // Can this be resized or not
	{ return m_fixedSize; }
	void setFixedSize(bool f)
	{ m_fixedSize = f; }

protected:
	T m_value;
	int m_columnCount = 1;
	bool m_fixedSize = false;
};

template <class T> 
T& getContents(VectorWrapper<T>& wrapper)
{ return wrapper.value(); }

template <class T> 
const  T& getContents(const VectorWrapper<T>& wrapper)
{ return wrapper.value(); }

template <class T>
struct contents;

template <class T>
struct contents<VectorWrapper<T>>
{ using type = T; };
