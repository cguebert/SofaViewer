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
	StructItem(const std::string& name, size_t size, size_t offset)
		: BaseStructItem(name, size, offset, std::type_index(typeid(T))) {}

	std::string serialize(const void* pVal) const override
	{ return conversion::toString(getVal(pVal)); }

	void deserialize(void* pVal, const std::string& text) const override
	{ conversion::fromString(getVal(pVal), text); }

	virtual Property::SPtr getSubProperty(void* pVal) const override
	{
		auto& val = getVal(pVal);
		return property::createRefProperty(name(), val);
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

template <class T>
std::shared_ptr<BaseStructItem> createStructItem(const std::string& name, size_t offset) 
{ return std::make_shared<StructItem<T>>(name, sizeof(T), offset); }

#define STRUCT_ITEM(n, s, m) createStructItem<decltype(s::m)>(n, offsetof(s, m))

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
};

} // unnamed namespace

//****************************************************************************//

struct CORE_API Struct : public Widget, public Serializator
{
	Struct(const std::vector<BaseStructItem::SPtr>& items) : Widget("struct"), items(items) {}
	const std::vector<BaseStructItem::SPtr> items;

	template <class T> 
	void setSerializeFunctions(std::function<std::string(const T&)>& funcSerialize,
		std::function<void(T&, const std::string& text)>& funcDeserialize) 
	{
		funcSerialize = [this](const T& val) -> std::string
		{ return details::serialize(val, items); };

		funcDeserialize = [this](T& val, const std::string& text)
		{ details::deserialize(val, text, items); };

		m_isFixedSizeFunc = [](Property::SPtr property) -> bool {
			const auto& value = property->value<T>()->value();
			return ListTraits<T>::fixed(value);
		};

		m_getSizeFunc = [](Property::SPtr property) -> int {
			const auto& value = property->value<T>()->value();
			return ListTraits<T>::size(value);
		};

		m_resizeSizeFunc = [](Property::SPtr property, int size) {
			auto& value = property->value<T>()->value();
			return ListTraits<T>::resize(value, size);
		};

		m_getSubPropertyFunc = [this](Property::SPtr property, int index, int structItem) {
			auto& value = property->value<T>()->value();
			auto& subVal = ListTraits<T>::value(value, index);
			assert(structItem >= 0 && structItem < static_cast<int>(items.size()));
			return items[structItem]->getSubProperty(&subVal);
		};
	}

	bool isFixedSize(Property::SPtr property) const
	{
		if (m_isFixedSizeFunc)
			return m_isFixedSizeFunc(property);
		return true;
	}

	int getSize(Property::SPtr property) const
	{
		if (m_getSizeFunc)
			return m_getSizeFunc(property);
		return 0;
	}

	void resize(Property::SPtr property, int size) const
	{
		if (m_resizeSizeFunc)
			m_resizeSizeFunc(property, size);
	}

	Property::SPtr getSubProperty(Property::SPtr property, int index, int structItem)
	{
		if (m_getSubPropertyFunc)
			return m_getSubPropertyFunc(property, index, structItem);
		return nullptr;
	}

private:
	std::function<bool(Property::SPtr)> m_isFixedSizeFunc;
	std::function<int(Property::SPtr)> m_getSizeFunc;
	std::function<void(Property::SPtr, int)> m_resizeSizeFunc;
	std::function<Property::SPtr(Property::SPtr, int, int)> m_getSubPropertyFunc;
};

} // namespace meta
