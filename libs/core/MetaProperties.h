#pragma once

#include <core/core.h>

#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <type_traits>
#include <vector>

namespace meta
{

struct CORE_API MetaProperty
{
	using SPtr = std::shared_ptr<MetaProperty>;

	virtual ~MetaProperty() {}
};

struct CORE_API Widget : virtual public MetaProperty
{
	const std::string& type()
	{ return m_type; }

protected:
	Widget(const std::string& type)
		: m_type(type) {}

private:
	std::string m_type;
};

struct CORE_API Validator : virtual public MetaProperty
{
	// Must have a template function setting a validator functor
	// This functor must return true if the value passed is changed
	// template <class T> void setValidate(std::function<bool(T&)>& func) {...}
};

struct CORE_API Serializator : virtual public MetaProperty
{
	// Must have a template function setting a serializator functor and a deserializator functor
	// template <class T> void setSerializeFunctions(
	//		std::function<std::string(const value_type&)>& func,
	//		std::function<void(value_type&, const std::string&)>& func
	//	)
};

//****************************************************************************//

class CORE_API BaseMetaContainer
{
public:
	using Properties = std::vector<MetaProperty::SPtr>;

	virtual ~BaseMetaContainer() {}

	template <class T>
	void add(T&& prop)
	{
		using prop_type = std::decay_t<T>;
		MetaProperty::SPtr ptr = std::make_shared<prop_type>(std::forward<T>(prop));

		const bool isValidator = std::is_base_of<Validator, prop_type>::value;
		static_assert(isValidator == false, "Validators can not be added through BaseMetaContainer");

		const bool isSerializator = std::is_base_of<Serializator, prop_type>::value;
		static_assert(isSerializator == false, "Serializators can not be added through BaseMetaContainer");

		m_properties.push_back(ptr);
	}

	// No treatment is done to this property when inserting. Use it only to pass data to the widgets
	void addExisting(MetaProperty::SPtr prop)
	{ m_properties.push_back(prop); }

	template <class T> T* get() const
	{
		for (auto& prop : m_properties)
		{
			T* ptr = dynamic_cast<T*>(prop.get());
			if (ptr)
				return ptr;
		}

		return nullptr;
	}

	const Properties& properties() const
	{ return m_properties; }

protected:
	Properties m_properties;
};

//****************************************************************************//

template <class prop_type, class func_type, bool isValidator>
struct GetValidateFunc
{ static void get(prop_type& prop, func_type& func) {} };

template <class prop_type, class func_type>
struct GetValidateFunc<prop_type, func_type, true>
{ static void get(prop_type& prop, func_type& func) { prop.setValidate(func); } };

template <class prop_type, class func_type1, class func_type2, bool isSerializator>
struct GetSerializeFunctions
{ static void get(prop_type& prop, func_type1& func1,  func_type2& func2) {} };

template <class prop_type,  class func_type1, class func_type2>
struct GetSerializeFunctions<prop_type, func_type1, func_type2, true>
{ static void get(prop_type& prop, func_type1& func1,  func_type2& func2) { prop.setSerializeFunctions(func1, func2); } };

//****************************************************************************//

template <class value_type>
class MetaContainer : public BaseMetaContainer
{
public:
	using ValidateFunc = std::function<bool(value_type&)>;
	using SerializeFunc = std::function<std::string(const value_type&)>;
	using DeserializeFunc = std::function<void(value_type&, const std::string&)>;

	template <class... Args>
	void add(Args&&... args)
	{
		doAdd(std::forward<Args>(args)...);
	}

	bool validate(value_type& value) const
	{
		bool changed = false;
		for (auto& func : m_validateFunctions)
			changed |= func(value);
		return changed;
	}

	std::string serialize(const value_type& value) const
	{
		if (m_serializeFunction)
			return m_serializeFunction(value);
		return "";
	}

	void deserialize(value_type& value, const std::string& text) const
	{
		if (m_deserializeFunction)
			m_deserializeFunction(value, text);
	}

private:
	template <class T, class... Args>
	void doAdd(T&& prop, Args&&... args)
	{
		doAdd(std::forward<T>(prop));
		doAdd(std::forward<Args>(args)...);
	}

	void doAdd() {} // Empty parameter pack

	template <class T>
	void doAdd(T&& prop)
	{
		using prop_type = std::decay_t<T>;
		MetaProperty::SPtr ptr = std::make_shared<prop_type>(std::forward<T>(prop));
		m_properties.push_back(ptr);

		const bool isValidator = std::is_base_of<Validator, prop_type>::value;
		if (isValidator)
		{
			prop_type& propRef = dynamic_cast<prop_type&>(*ptr.get());
			ValidateFunc func;
			GetValidateFunc<prop_type, ValidateFunc, isValidator>::get(propRef, func); // Even if the code is not executed, it is still created, so I have to go though another level of indirection
			if (func) 
				m_validateFunctions.push_back(func);
		}

		const bool isSerializator = std::is_base_of<Serializator, prop_type>::value;
		if (isSerializator)
		{
			prop_type& propRef = dynamic_cast<prop_type&>(*ptr.get());
			SerializeFunc serFunc;
			DeserializeFunc desFunc;

			GetSerializeFunctions<prop_type, SerializeFunc, DeserializeFunc, isSerializator>::get(propRef, serFunc, desFunc);
			if (serFunc) m_serializeFunction = serFunc;
			if (desFunc) m_deserializeFunction = desFunc;
		}
	}

	std::vector<ValidateFunc> m_validateFunctions;
	SerializeFunc m_serializeFunction;
	DeserializeFunc m_deserializeFunction;
};

//****************************************************************************//

// Ensure that a value stays inside a specified range
struct CORE_API Range : public Validator
{
	Range(float min, float max) : min(min), max(max) {}

	template <class T>
	void setValidate(std::function<bool(T&)>& func)
	{
		func = [this](T& val)
		{
			T tmin = static_cast<T>(min), tmax = static_cast<T>(max);
			if (val < tmin) { val = tmin; return true; }
			if (val > tmax) { val = tmax; return true; }

			return false;
		};
	}

	template <class T>
	void setValidate(std::function<bool(std::vector<T>&)>& func)
	{
		func = [this](std::vector<T>& val)
		{
			bool changed = false;
			for (auto& v : val)
			{
				T tmin = static_cast<T>(min), tmax = static_cast<T>(max);
				if (v < tmin) { v = tmin; changed = true; }
				if (v > tmax) { v = tmax; changed = true; }
			}

			return changed;
		};
	}

	float min, max;
};

// TODO: No validation is done with the step yet
struct CORE_API RangeWithStep : public Range
{
	RangeWithStep(float min, float max, float step) : Range(min, max), step(step) {}

	float step;
};

//****************************************************************************//

struct CORE_API Checkbox : public Widget // For int
{
	Checkbox() : Widget("checkbox") {}
};

struct CORE_API Enum : public Widget // For int & string
{
	Enum(const std::vector<std::string>& values) : Widget("enum"), values(values) {}

	std::vector<std::string> values;
};

struct CORE_API Directory : public Widget // For string
{
	Directory() : Widget("directory") {}
};

struct CORE_API File : public Widget // For string
{
	File() : Widget("file") {}
	File(const std::string& filter) : Widget("file"), filter(filter) {}

	std::string filter;
};

struct CORE_API Slider : public Widget, public RangeWithStep // For numerical values
{
	Slider(float min, float max, float step) : Widget("slider"), RangeWithStep(min, max, step) {}
};

struct CORE_API Color : public Widget // For vec3 and vec4
{
	Color() : Widget("color") {}
};

} // namespace meta
