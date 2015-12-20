#pragma once

#include <core/PropertiesUtils.h>

#include <cassert>

namespace meta
{

class CORE_API BaseStructItem
{
public:
	using SPtr = std::shared_ptr<BaseStructItem>;
	BaseStructItem(const std::string& name, size_t size, size_t offset, std::type_index type)
		: m_name(name), m_size(size), m_offset(offset), m_type(type) {}
	virtual ~BaseStructItem();

	const std::string& name() const { return m_name; }
	size_t offset() const { return m_offset; }
	size_t size() const { return m_size; }
	std::type_index type() const { return m_type; }

	virtual std::string serialize(const void* pVal) const = 0;
	virtual void deserialize(void* pVal, const std::string& text) const = 0;

	virtual Property::SPtr getSubProperty(void* pVal) const = 0;
	virtual const BaseMetaContainer& metaContainer() const = 0;

	virtual bool isEqual(const void* pVal1, const void* pVal2) const = 0;

private:
	std::string m_name;
	size_t m_offset, m_size;
	std::type_index m_type;
};

//****************************************************************************//

template <class T>
class StructItem : public BaseStructItem
{
public:
	template <class... Args>
	StructItem(const std::string& name, size_t size, size_t offset, Args&&... args)
		: BaseStructItem(name, size, offset, std::type_index(typeid(T))) 
	{
		m_metaContainer.add(std::forward<Args>(args)...);
	}

	std::string serialize(const void* pVal) const override
	{ return conversion::toString(getVal(pVal)); }

	void deserialize(void* pVal, const std::string& text) const override
	{ conversion::fromString(getVal(pVal), text); }

	virtual Property::SPtr getSubProperty(void* pVal) const override
	{
		auto& val = getVal(pVal);
		auto prop = property::createRefProperty(name(), val);
		auto& metaContainer = prop->value()->baseMetaContainer();
		for (const auto& meta : m_metaContainer.properties())
			metaContainer.addExisting(meta);
		return prop;
	}

	template <class... Args>
	void setMeta(Args&&... args)
	{
		m_metaContainer.add(std::forward<Args>(args)...);
	}

	const BaseMetaContainer& metaContainer() const override
	{
		return m_metaContainer;
	}

	bool isEqual(const void* pVal1, const void* pVal2) const override
	{
		const auto& val1 = getVal(pVal1);
		const auto& val2 = getVal(pVal2);
		return val1 == val2;
	}

private:
	void* offsetedPtr(void* pVal) const 
	{ return static_cast<uint8_t*>(pVal) + offset(); }

	const void* offsetedPtr(const void* pVal) const 
	{ return static_cast<const uint8_t*>(pVal) + offset(); }

	T& getVal(void* pVal) const 
	{ return *static_cast<T*>(offsetedPtr(pVal)); }

	const T& getVal(const void* pVal) const
	{ return *static_cast<const T*>(offsetedPtr(pVal)); }

	MetaContainer<T> m_metaContainer;
};

//****************************************************************************//

template <class T, class... Args>
std::shared_ptr<StructItem<T>> createStructItem(const std::string& name, size_t offset, Args&&... args) 
{ return std::make_shared<StructItem<T>>(name, sizeof(T), offset, std::forward<Args>(args)...); }

#define STRUCT_ITEM(n, s, m) createStructItem<decltype(s::m)>(n, offsetof(s, m))
#define STRUCT_ITEM_1META(n, s, m, meta) createStructItem<decltype(s::m)>(n, offsetof(s, m), meta)
#define STRUCT_ITEM_2META(n, s, m, meta0, meta1) createStructItem<decltype(s::m)>(n, offsetof(s, m), meta0, meta1)

//****************************************************************************//

namespace utils
{

std::string CORE_API replaceAll(const std::string& val, const std::string& from, const std::string& to);
std::string CORE_API encode(const std::string& val);
std::string CORE_API decode(const std::string& val);
std::string CORE_API getValueToken(const std::string& text, size_t& pos);
std::string CORE_API getObjectToken(const std::string& text, size_t& pos);

} // namespace utils

namespace details
{

template <class T>
std::string serialize(const T& val, const std::vector<BaseStructItem::SPtr>& items)
{
	std::string out = "{";
	for (int i = 0, nbItems = items.size(); i < nbItems; ++i)
	{
		if (i) out += ',';
		out += utils::encode(items[i]->serialize(&val));
	}
	out += '}';
	return out;
}

template <class T>
std::string serialize(const std::vector<T>& val, const std::vector<BaseStructItem::SPtr>& items)
{
	std::string out;
	for (int i = 0, nbV = val.size(); i < nbV; ++i)
	{
		if (i) out += ",";
		out += serialize(val[i], items);
	}
	return out;
}

template <class T>
bool deserialize(T& val, const std::string& text, const std::vector<BaseStructItem::SPtr>& items)
{
	size_t pos = text.find('{');
	if (pos == std::string::npos)
		return false;
	++pos;

	for (auto& item : items)
	{
		auto token = utils::decode(utils::getValueToken(text, pos));
		item->deserialize(&val, token);
	}

	if (text[pos-1] != '}')
		return false;
	return true;
}

template <class T>
void deserialize(std::vector<T>& val, const std::string& text, const std::vector<BaseStructItem::SPtr>& items)
{
	val.clear();
	size_t pos = 0, len = text.size();
	while (pos < len)
	{
		std::string token = utils::getObjectToken(text, pos);
		T singleValue = T();
		if (deserialize(singleValue, token, items))
			val.push_back(singleValue);
		else
			break;
	}
}

} // namespace details

namespace
{

// Used to differentiate for singleValues, std::vector and VectorWrapper
template <class T> 
struct ListTraits
{
	using value_type = T;
	using base_type = T;

	static int size(const value_type& value) { return 1; }
	static void resize(value_type& value, int nb) { }
	static bool fixed(const value_type& value) { return true; }
	static base_type& value(value_type& value, int) { return value; }
	static const base_type& value(const value_type& value, int) { return value; }
};

template <class T>
struct ListTraits<std::vector<T>>
{
	using value_type = std::vector<T>;
	using base_type = T;

	static int size(const value_type& value) { return value.size(); }
	static void resize(value_type& value, int nb) { value.resize(nb); }
	static bool fixed(const value_type& value) { return false; }
	static base_type& value(value_type& value, int index) { return value[index]; }
	static const base_type& value(const value_type& value, int index) { return value[index]; }
};

template <class T>
struct ListTraits<VectorWrapper<T>>
{
	using value_type = VectorWrapper<T>;
	using base_type = typename value_type::base_type;

	static int size(const value_type& wrapper) { return wrapper.value().size(); }
	static void resize(value_type& wrapper, int nb) { wrapper.value().resize(nb); }
	static bool fixed(const value_type& wrapper) { return wrapper.fixedSize(); }
	static base_type& value(value_type& wrapper, int index) { return wrapper.value()[index]; }
	static const base_type& value(const value_type& wrapper, int index) { return wrapper.value()[index]; }
};

} // unnamed namespace

//****************************************************************************//

struct CORE_API Struct : public Widget, public Serializator
{
	Struct(const std::vector<BaseStructItem::SPtr>& items) : Widget("struct"), items(items) {}
	const std::vector<BaseStructItem::SPtr> items;

	template <class T>
	static T& getValue(BasePropertyValue::SPtr value)
	{
		return std::dynamic_pointer_cast<PropertyValue<T>>(value)->value();
	}

	template <class T> 
	void setSerializeFunctions(std::function<std::string(const T&)>& funcSerialize,
		std::function<void(T&, const std::string& text)>& funcDeserialize) 
	{
		funcSerialize = [this](const T& val) -> std::string
		{ return details::serialize(val, items); };

		funcDeserialize = [this](T& val, const std::string& text)
		{ details::deserialize(val, text, items); };

		m_isFixedSizeFunc = [](BasePropertyValue::SPtr propVal) -> bool {
			const auto& value = getValue<T>(propVal);
			return ListTraits<T>::fixed(value);
		};

		m_getSizeFunc = [](BasePropertyValue::SPtr propVal) -> int {
			const auto& value = getValue<T>(propVal);
			return ListTraits<T>::size(value);
		};

		m_resizeSizeFunc = [](BasePropertyValue::SPtr propVal, int size) {
			auto& value = getValue<T>(propVal);
			return ListTraits<T>::resize(value, size);
		};

		m_getSubPropertyFunc = [this](BasePropertyValue::SPtr propVal, int index, int structItem) {
			auto& value = getValue<T>(propVal);
			auto& subVal = ListTraits<T>::value(value, index);
			assert(structItem >= 0 && structItem < static_cast<int>(items.size()));
			return items[structItem]->getSubProperty(&subVal);
		};

		m_clonePropertyValueFunc = [](Property::SPtr property) -> BasePropertyValue::SPtr {
			const auto& value = property->value<T>()->value();
			return property::createCopyValue(value);
		};

		m_isModifiedFunc = [this](BasePropertyValue::SPtr propVal1, BasePropertyValue::SPtr propVal2) -> bool{
			const auto& value1 = getValue<T>(propVal1);
			const auto& value2 = getValue<T>(propVal2);
			const int size1 = ListTraits<T>::size(value1);
			const int size2 = ListTraits<T>::size(value2);
			if (size1 != size2)
				return true;
			
			// We compare only the data that is exposed via meta::Struct
			int nbItems = items.size();
			for (int i = 0; i < size1; ++i)
			{
				const auto& subVal1 = ListTraits<T>::value(value1, i);
				const auto& subVal2 = ListTraits<T>::value(value2, i);
				for (int j = 0; j < nbItems; ++j)
				{
					if (!items[j]->isEqual(&subVal1, &subVal2))
						return true;
				}
			}

			return false;
		};

		m_setValueFunc = [](BasePropertyValue::SPtr to, BasePropertyValue::SPtr from) {
			auto& value1 = getValue<T>(to);
			const auto& value2 = getValue<T>(from);
			value1 = value2;
		};

		m_validateFunc = [](BasePropertyValue::SPtr propVal) {
			auto propValT = std::dynamic_pointer_cast<PropertyValue<T>>(propVal);
			auto& value = propValT->value();
			return propValT->validate(value);
		};
	}

	bool isFixedSize(BasePropertyValue::SPtr value) const
	{
		if (m_isFixedSizeFunc)
			return m_isFixedSizeFunc(value);
		return true;
	}

	int getSize(BasePropertyValue::SPtr value) const
	{
		if (m_getSizeFunc)
			return m_getSizeFunc(value);
		return 0;
	}

	void resize(BasePropertyValue::SPtr value, int size) const
	{
		if (m_resizeSizeFunc)
			m_resizeSizeFunc(value, size);
	}

	Property::SPtr getSubProperty(BasePropertyValue::SPtr value, int index, int structItem)
	{
		if (m_getSubPropertyFunc)
			return m_getSubPropertyFunc(value, index, structItem);
		return nullptr;
	}

	BasePropertyValue::SPtr cloneProperty(Property::SPtr property)
	{
		if (m_clonePropertyValueFunc)
			return m_clonePropertyValueFunc(property);
		return nullptr;
	}

	bool isModified(BasePropertyValue::SPtr val1, BasePropertyValue::SPtr val2)
	{
		if (m_isModifiedFunc)
			return m_isModifiedFunc(val1, val2);
		return false;
	}

	void setValue(BasePropertyValue::SPtr to, BasePropertyValue::SPtr from)
	{
		if (m_setValueFunc)
			m_setValueFunc(to, from);
	}

	bool validate(BasePropertyValue::SPtr propVal)
	{
		if (m_validateFunc)
			return m_validateFunc(propVal);
		return false;
	}

private:
	std::function<bool(BasePropertyValue::SPtr)> m_isFixedSizeFunc;
	std::function<int(BasePropertyValue::SPtr)> m_getSizeFunc;
	std::function<void(BasePropertyValue::SPtr, int)> m_resizeSizeFunc;
	std::function<Property::SPtr(BasePropertyValue::SPtr, int, int)> m_getSubPropertyFunc;
	std::function<BasePropertyValue::SPtr(Property::SPtr)> m_clonePropertyValueFunc;
	std::function<bool(BasePropertyValue::SPtr, BasePropertyValue::SPtr)> m_isModifiedFunc;
	std::function<void(BasePropertyValue::SPtr, BasePropertyValue::SPtr)> m_setValueFunc;
	std::function<bool(BasePropertyValue::SPtr)> m_validateFunc;
};

} // namespace meta
