#pragma once

#include <core/core.h>
#include <memory>
#include <string>
#include <typeindex>

class BasePropertyValue;
template <class T> class PropertyValue;

class CORE_API Property
{
public:
	using ValuePtr = std::shared_ptr<BasePropertyValue>;
	template <class T> using ValueTPtr = std::shared_ptr<PropertyValue<T>>;
	using PropertyPtr = std::shared_ptr<Property>;

	Property();
	Property(const std::string& name, ValuePtr value);
	Property(const std::string& name,
			 const std::string& widget = "default",
			 bool readOnly = false,
			 const std::string& help = "",
			 const std::string& group = "");

	const std::string& name() const;
	void setName(const std::string& name);
	const std::string& help() const;
	void setHelp(const std::string& help);
	const std::string& group() const;
	void setGroup(const std::string& group);

	bool readOnly() const;
	void setReadOnly(bool readOnly);

	std::type_index type() const;
	const std::string& widget() const; // Used to choose the widget (for example, storage is int but type was originally bool)
	void setWidget(const std::string& widget);

	ValuePtr value() const;
	template <class T> ValueTPtr<T> value() const
	{ return std::dynamic_pointer_cast<PropertyValue<T>>(m_value); }
	void setValue(ValuePtr value);

protected:
	std::string m_name, m_help, m_group, m_widget;
	bool m_readOnly = false;
	std::type_index m_type;
	std::shared_ptr<BasePropertyValue> m_value;
};

//****************************************************************************//

class BasePropertyValue
{
public:
	~BasePropertyValue() {}
	virtual std::type_index type() const = 0;
};

template <class T>
class PropertyValue : public BasePropertyValue
{
public:
	PropertyValue() {}

	virtual const T& value() const = 0;
	virtual T& value() = 0;
	virtual void setValue(const T& value) = 0;
	virtual void setValue(T&& value) = 0;

	std::type_index type() const
	{ return std::type_index(typeid(T)); }

protected:
	T m_value;
};

//****************************************************************************//

template <class T>
class PropertyValueCopy : public PropertyValue<T>
{
public:
	PropertyValueCopy(const T& val) : m_value(val) {}
	PropertyValueCopy(T&& val) : m_value(std::move(val)) {}

	const T& value() const override { return m_value; }
	T& value() override { return m_value; }
	void setValue(const T& value) override { m_value = value; }
	void setValue(T&& value) override { m_value = std::move(value); }

protected:
	T m_value;
};

template <class T>
class PropertyValueRef : public PropertyValue<T>
{
public:
	PropertyValueRef(T& val) : m_value(val) {}

	const T& value() const override { return m_value; }
	T& value() override { return m_value; }
	void setValue(const T& value) override { m_value = value; }
	void setValue(T&& value) override { m_value = std::move(value); }

protected:
	T& m_value;
};

//****************************************************************************//

inline const std::string& Property::name() const
{ return m_name; }

inline void Property::setName(const std::string& name)
{ m_name = name; }

inline const std::string& Property::help() const
{ return m_help; }

inline void Property::setHelp(const std::string& help)
{ m_help = help; }

inline const std::string& Property::group() const
{ return m_group; }

inline void Property::setGroup(const std::string& group)
{ m_group = group; }

inline bool Property::readOnly() const
{ return m_readOnly; }

inline void Property::setReadOnly(bool readOnly)
{ m_readOnly = readOnly; }

inline std::type_index Property::type() const
{ return m_type; }

inline const std::string& Property::widget() const
{ return m_widget; }

inline void Property::setWidget(const std::string& widget)
{ m_widget = widget; }

inline std::shared_ptr<BasePropertyValue> Property::value() const
{ return m_value; }
