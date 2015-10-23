#pragma once

#include <core/core.h>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace meta
{

struct CORE_API MetaProperty
{
	using SPtr = std::shared_ptr<MetaProperty>;

	virtual ~MetaProperty() {}
};

struct CORE_API Widget : public MetaProperty
{
	const std::string& type()
	{ return m_type; }

protected:
	Widget(const std::string& type)
		: m_type(type) {}

	std::string m_type;
};

struct CORE_API Validator : public MetaProperty
{
	// Must have a template function setting a validator functor
	// template <class T> void init(std::function<bool(T&)>& func) {...}
};

//****************************************************************************//

class CORE_API BaseMetaContainer
{
public:
	virtual ~BaseMetaContainer() {}

	template <class T>
	void add(T&& prop)
	{
		using prop_type = std::remove_cv_t<std::remove_reference_t<T>>;
		MetaProperty::SPtr ptr = std::make_shared<prop_type>(std::forward<T>(prop));

		const bool isValidator = std::is_base_of<Validator, prop_type>::value;
		static_assert(isValidator == false, "Validators can not be added through BaseMetaContainer");

		m_properties.push_back(ptr);
	}

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

protected:
	std::vector<MetaProperty::SPtr> m_properties;
};

template <class prop_type, class value_type, bool isValidator>
struct PropertyInit
{
	static void init(prop_type& prop, value_type& value) {}
};

template <class prop_type, class value_type>
struct PropertyInit<prop_type, value_type, true>
{
	static void init(prop_type& prop, value_type& value)
	{ prop.init(value); }
};


template <class value_type>
class MetaContainer : public BaseMetaContainer
{
public:
	using validateFunc = std::function<bool(value_type&)>;

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

private:
	template <class T, class... Args>
	void doAdd(T&& prop, Args&&... args)
	{
		doAdd(std::forward<T>(prop));
		doAdd(std::forward<Args>(args)...);
	}

	template <class T>
	void doAdd(T&& prop)
	{
		using prop_type = std::remove_cv_t<std::remove_reference_t<T>>;
		MetaProperty::SPtr ptr = std::make_shared<prop_type>(std::forward<T>(prop));
		m_properties.push_back(ptr);

		const bool isValidator = std::is_base_of<Validator, prop_type>::value;
		if (isValidator)
		{
			prop_type& propRef = dynamic_cast<prop_type&>(*ptr.get());
			validateFunc func;
			PropertyInit<prop_type, validateFunc, isValidator>::init(propRef, func);
			if (func)
				m_validateFunctions.push_back(func);
		}
	}

	std::vector<validateFunc> m_validateFunctions;
};

//****************************************************************************//

struct CORE_API Range : public Validator
{
	Range(float min, float max) : min(min), max(max) {}

	template <class T>
	void init(std::function<bool(T&)>& func)
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
	void init(std::function<bool(std::vector<T>&)>& func)
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

//****************************************************************************//

struct CORE_API Checkbox : public Widget
{
	Checkbox() : Widget("checkbox") {}
};

struct CORE_API Enum : public Widget
{
	Enum(const std::vector<std::string>& values) : Widget("enum"), values(values) {}

	std::vector<std::string> values;
};

} // namespace meta
