#pragma once

#include <core/core.h>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace meta
{

class CORE_API MetaProperty
{
public:
	using SPtr = std::shared_ptr<MetaProperty>;

	virtual ~MetaProperty() {}
};

class CORE_API Widget : public MetaProperty
{
public:
	Widget(const std::string& type)
		: m_type(type) {}

	const std::string& type()
	{ return m_type; }

protected:
	std::string m_type;
};

class CORE_API Validator : public MetaProperty
{
public:
	bool validate() const // Returns true if the value has been changed
	{
		if (m_validateFunc) 
			return m_validateFunc();
		return false;
	}

protected:
	std::function<bool()> m_validateFunc;
};

//****************************************************************************//

class CORE_API BaseMetaContainer
{
public:
	virtual ~BaseMetaContainer() {}

	template <class T> T* get()
	{
		for (auto& prop : m_properties)
		{
			T* ptr = dynamic_cast<T*>(prop.get());
			if (ptr)
				return ptr;
		}

		return nullptr;
	}

	bool validate() const
	{
		bool changed = false;
		for (auto& prop : m_properties)
		{
			auto validator = dynamic_cast<Validator*>(prop.get());
			if (validator)
				changed |= validator->validate();
		}
		return changed;
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
	{
		prop.init(value);
	}
};


template <class value_type>
class MetaContainer : public BaseMetaContainer
{
public:
	template <class... Args>
	void add(value_type& value, Args&&... args)
	{
		doAdd(value, std::forward<Args>(args)...);
	}

private:
	template <class T, class... Args>
	void doAdd(value_type& value, T&& prop, Args&&... args)
	{
		doAdd(value, std::forward<T>(prop));
		doAdd(value, std::forward<Args>(args)...);
	}

	template <class T>
	void doAdd(value_type& value, T&& prop)
	{
		using prop_type = std::remove_cv_t<std::remove_reference_t<T>>;
		MetaProperty::SPtr ptr = std::make_shared<prop_type>(std::forward<T>(prop));

		const bool isValidator = std::is_base_of<Validator, prop_type>::value;
		prop_type& propRef = dynamic_cast<prop_type&>(*ptr.get());
		PropertyInit<prop_type, value_type, isValidator>::init(propRef, value);

		m_properties.push_back(ptr);
	}
};

//****************************************************************************//

class CORE_API Range : public Validator
{
public:
	Range(float min, float max) : m_min(min), m_max(max) {}

	template <class T>
	void init(T& val)
	{
		m_validateFunc = [&val, this]()
		{
			T tmin = static_cast<T>(m_min), tmax = static_cast<T>(m_max);
			if (val < tmin) { val = tmin; return true; }
			if (val > tmax) { val = tmax; return true; }

			return false;
		};
	}

	template <class T>
	void init(std::vector<T>& val)
	{
		m_validateFunc = [&val, this]()
		{
			bool changed = false;
			for (auto& v : val)
			{
				T tmin = static_cast<T>(m_min), tmax = static_cast<T>(m_max);
				if (v < tmin) { v = tmin; changed = true; }
				if (v > tmax) { v = tmax; changed = true; }
			}

			return changed;
		};
	}

private:
	float m_min, m_max;
};

} // namespace meta
