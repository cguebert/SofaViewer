#pragma once

#include <memory>
#include <string>
#include <vector>

class BasePropertyValue;

class Property
{
public:
	enum class Type : int32_t // Copy of sfe::Data::DataType
	{
		NotSet = 0,
		Int = 1,
		Float = 2,
		Double = 3,
		String = 4,
		Vector_Int = 17,
		Vector_Float = 18,
		Vector_Double = 19,
		Vector_String = 20
	};

	Property() {}
	Property(const std::string& name,
			 const std::string& help,
			 const std::string& group,
			 bool readOnly,
			 Type storageType,
			 const std::string& valueType);

	const std::string& name() const;
	const std::string& help() const;
	const std::string& group() const;
	bool readOnly() const;
	Type storageType() const;
	const std::string& valueType() const; // Used to choose the widget (for example, storage is int but type was originally bool)

	void setColumnCount(int count);
	int columnCount() const;

	using ValuePtr = std::shared_ptr<BasePropertyValue>;
	void setValue(ValuePtr value);
	ValuePtr value() const;

	std::string m_stringValue; // TEMP

protected:
	std::string m_name, m_help, m_group, m_valueType;
	bool m_readOnly = false;
	Type m_storageType = Type::NotSet;
	int m_columnCount = 1;
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
	T& value() { return m_value; }
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

inline Property::Type Property::storageType() const
{ return m_storageType; }

inline void Property::setColumnCount(int count)
{ m_columnCount = count; }

inline int Property::columnCount() const
{ return m_columnCount; }

inline const std::string& Property::valueType() const
{ return m_valueType; }

inline std::shared_ptr<BasePropertyValue> Property::value() const
{ return m_value; }
