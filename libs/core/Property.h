#pragma once

#include <core/core.h>
#include <core/MetaProperties.h>
#include <core/StringConversion.h>

#include <memory>
#include <string>
#include <typeindex>

class BasePropertyValue;
template <class T> class PropertyValue;

class CORE_API Property
{
public:
	using SPtr = std::shared_ptr<Property>;
	using ValuePtr = std::shared_ptr<BasePropertyValue>;
	template <class T> using ValueTPtr = std::shared_ptr<PropertyValue<T>>;
	
	Property();
	Property(const std::string& name, ValuePtr value);
	Property(const std::string& name,
			 bool readOnly = false,
			 const std::string& help = "",
			 const std::string& group = "");

	const std::string& name() const;		/// Name of the property, as shown in the UI
	void setName(const std::string& name);
	const std::string& help() const;		/// Description for tooltips and status bar tips
	void setHelp(const std::string& help);
	const std::string& group() const;		/// Regroup properties of the same group in tabs
	void setGroup(const std::string& group);

	bool readOnly() const; /// If true, show the property but disable the edition
	void setReadOnly(bool readOnly);

	/// Tells the UI when to update the property when a widget has been modified
	enum class SaveTrigger { 
		action, /// Wait for the dialog to be closed (properties outside of dialogs will never be updated automatically)
		asap /// Update the property each time the widget is modified
	};
	SaveTrigger saveTrigger() const;
	void setSaveTrigger(SaveTrigger trigger);

	std::type_index type() const; /// Represents the type of the value
	
	ValuePtr value() const;	/// Different types of containers exists for a property value, by default it contains nothing
	template <class T> ValueTPtr<T> value() const
	{ return std::dynamic_pointer_cast<PropertyValue<T>>(m_value); }
	void setValue(ValuePtr value);

	template <class T> T* getMeta() const /// Obtain the metaproperty of the specified type, returns null if not present
	{
		if (!m_value) return nullptr;
		return m_value->baseMetaContainer().get<T>();
	}

protected:
	std::string m_name, m_help, m_group;
	bool m_readOnly = false;
	std::type_index m_type;
	std::shared_ptr<BasePropertyValue> m_value;
	SaveTrigger m_saveTrigger = SaveTrigger::action;
};

//****************************************************************************//

// Stores the value of a Property
class BasePropertyValue
{
public:
	using SPtr = std::shared_ptr<BasePropertyValue>;

	virtual ~BasePropertyValue() {}
	virtual std::type_index type() const = 0;
	virtual bool validate() = 0; // Applies meta::Validators, returns true if the value has been modified
	virtual meta::BaseMetaContainer& baseMetaContainer() = 0;
	virtual std::string toString() const = 0;
	virtual void fromString(const std::string& text) = 0;
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

	std::type_index type() const override
	{ return std::type_index(typeid(T)); }

	bool validate() override
	{ return m_metaProperties.validate(value()); }

	bool validate(T& val)
	{ return m_metaProperties.validate(val); }

	meta::BaseMetaContainer& baseMetaContainer() override
	{ return m_metaProperties; }

	meta::MetaContainer<T>& metaContainer()
	{ return m_metaProperties; }

	std::string toString() const
	{ return conversion::toString(value()); }

	void fromString(const std::string& text)
	{ conversion::fromString(value(), text); }

protected:
	meta::MetaContainer<T> m_metaProperties;
};

//****************************************************************************//

template <class T>
class PropertyCopyValue : public PropertyValue<T>
{
public:
	PropertyCopyValue(const T& val) : m_value(val) {}
	PropertyCopyValue(T&& val) : m_value(std::move(val)) {}

	const T& value() const override { return m_value; }
	T& value() override { return m_value; }
	void setValue(const T& value) override { m_value = value; }
	void setValue(T&& value) override { m_value = std::move(value); }

protected:
	T m_value;
};

template <class T>
class PropertyRefValue : public PropertyValue<T>
{
public:
	PropertyRefValue(T& val) : m_value(val) {}

	const T& value() const override { return m_value; }
	T& value() override { return m_value; }
	void setValue(const T& value) override { m_value = value; }
	void setValue(T&& value) override { m_value = std::move(value); }

protected:
	T& m_value;
};

//****************************************************************************//

// Used in the UI, as a way to copy both way between a Property and a value of a compatible type
class BaseValueWrapper
{
public:
	using SPtr = std::shared_ptr<BaseValueWrapper>;

	BaseValueWrapper(Property::SPtr property)
		: m_property(property) {}
	virtual ~BaseValueWrapper() {}

	virtual void writeToValue() = 0; // Property -> value
	virtual void readFromValue() = 0; // Value -> property

	Property::SPtr property() const { return m_property; }

protected:
	Property::SPtr m_property;
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

inline Property::SaveTrigger Property::saveTrigger() const
{ return m_saveTrigger; }

inline void Property::setSaveTrigger(SaveTrigger trigger)
{ m_saveTrigger = trigger; }

inline std::type_index Property::type() const
{ return m_type; }

inline std::shared_ptr<BasePropertyValue> Property::value() const
{ return m_value; }
