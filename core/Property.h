#pragma once

#include <memory>
#include <string>
#include <vector>

class BasePropertyValue;

class Property
{
public:
	const std::string& name() const;
	const std::string& help() const;
	const std::string& group() const;
	bool readOnly() const;
	int type() const;
	std::shared_ptr<BasePropertyValue> value() const;

//protected:
	bool m_readOnly = false;
	std::string m_stringValue; // TEMP
	std::string m_name, m_help, m_group;
	int m_valueType = 0;
	std::shared_ptr<BasePropertyValue> m_value;
};

class BasePropertyValue
{
public:
	virtual const void* ptr() const = 0;
	virtual void* ptr() = 0;
};

template <class T>
class PropertyValue : public BasePropertyValue
{
public:
	PropertyValue() {}
	PropertyValue(const T& val) : m_value(val) {}
	PropertyValue(T&& val) : m_value(std::move(val)) {}
	const void* ptr() const override { return &m_value; }
	void* ptr() override { return &m_value; }

	const T& value() const { return m_value; }
	void setValue(const T& value) { m_value = value; }
	void setValue(T&& value) { m_value = std::move(value); }

protected:
	T m_value;
};

inline const std::string& Property::name() const
{ return m_name; }

inline const std::string& Property::help() const
{ return m_help; }

inline const std::string& Property::group() const
{ return m_group; }

inline bool Property::readOnly() const
{ return m_readOnly; }

inline int Property::type() const
{ return m_valueType; }

inline std::shared_ptr<BasePropertyValue> Property::value() const
{ return m_value; }
