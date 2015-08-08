#pragma once

#include <core/Property.h>

#include <vector>

/// This class helps create better property widgets
/// for vectors of values that behave as arrays
template <class T>
class VectorWrapper
{
public:
	using value_type = typename T::value_type;
	using wrapper_type = VectorWrapper<T>;
	VectorWrapper() {}
	VectorWrapper(const T& value) : m_value(value) {}
	VectorWrapper(T&& value) : m_value(std::move(value)) {}

	operator T() const { return m_value; }
	void operator=(const T& val) { m_value = val; }

	bool operator==(const wrapper_type& rhs)
	{ return m_value == rhs.m_value; }
	bool operator!=(const wrapper_type& rhs)
	{ return m_value != rhs.m_value; }

	const T& value() const
	{ return m_value; }
	T& value()
	{ return m_value; }

	int columnCount() const // Length of an array
	{ return m_columnCount; }
	void setColumnCount(int count)
	{ m_columnCount = count; }

	bool fixedSize() const // Can this be resized or not
	{ return m_fixedSize; }
	void setFixedSize(bool f)
	{ m_fixedSize = f; }

protected:
	T m_value;
	int m_columnCount = 1;
	bool m_fixedSize = false;
};
