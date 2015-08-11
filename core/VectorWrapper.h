#pragma once

#include <core/Property.h>

#include <vector>

/// This class helps create better property widgets
/// for vectors of values that behave as arrays
template <class T>
class VectorWrapper
{
public:
	using value_type = T;
	using wrapper_type = VectorWrapper<value_type>;
	VectorWrapper() {}
	VectorWrapper(const value_type& value) : m_value(value) {}
	VectorWrapper(value_type&& value) : m_value(std::move(value)) {}

	operator value_type() const { return m_value; }
	void operator=(const value_type& val) { m_value = val; }

	bool operator==(const wrapper_type& rhs)
	{ return m_value == rhs.m_value; }
	bool operator!=(const wrapper_type& rhs)
	{ return m_value != rhs.m_value; }

	const value_type& value() const
	{ return m_value; }
	value_type& value()
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
