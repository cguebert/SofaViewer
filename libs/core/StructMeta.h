#pragma once

#include <core/MetaProperties.h>
#include <core/StringConversion.h>

namespace meta
{

class CORE_API BaseStructItemData
{
public:
	BaseStructItemData(size_t size, size_t offset, std::type_index type)
		: m_size(size), m_offset(offset), m_type(type) {}
	virtual ~BaseStructItemData();

	size_t offset() const { return m_offset; }
	size_t size() const { return m_size; }
	std::type_index type() const { return m_type; }

	virtual std::string serialize(const void* pVal) const = 0;
	virtual void deserialize(void* pVal, const std::string& text) const = 0;

private:
	size_t m_offset, m_size;
	std::type_index m_type;
};

//****************************************************************************//

template <class T>
class StructItemData : public BaseStructItemData
{
public:
	StructItemData(size_t size, size_t offset)
		: BaseStructItemData(size, offset, std::type_index(typeid(T))) {}

	std::string serialize(const void* pVal) const override
	{ return conversion::toString(getVal(pVal)); }

	void deserialize(void* pVal, const std::string& text) const override
	{ conversion::fromString(getVal(pVal), text); }

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
std::shared_ptr<BaseStructItemData> createStructItem(size_t offset) 
{ return std::make_shared<StructItemData<T>>(sizeof(T), offset); }

#define STRUCT_ITEM(s, m) createStructItem<decltype(s::m)>(offsetof(s, m))

struct CORE_API StructItem
{
	StructItem(const std::string& name, std::shared_ptr<BaseStructItemData> data)
		: name(name), data(data) {}

	std::string name;
	std::shared_ptr<BaseStructItemData> data;
};

//****************************************************************************//

namespace details
{

std::string replaceAll(const std::string& val, const std::string& from, const std::string& to)
{
	if (from.empty())
		return val;

	std::string ret = val;
	auto fromSize = from.size(), toSize = to.size();
	size_t pos = 0;
	while ((pos = val.find(from, pos)) != std::string::npos)
	{
		ret.replace(pos, fromSize, to);
		pos += toSize;
	}

	return ret;
}

std::string encode(const std::string& val)
{
	auto v = replaceAll(val, ",", "\\,");
	v = replaceAll(v, "}", "\\}");
	return v;
}

std::string decode(const std::string& val)
{
	auto v = replaceAll(val, "\\,", ",");
	v = replaceAll(v, "\\}", "}");
	return v;
}

template <class T>
std::string serialize(const T& val, const std::vector<StructItem>& items)
{
	std::string out = "{";
	for (int i = 0, nbItems = items.size(); i < nbItems; ++i)
	{
		if (i) out += ',';
		out += encode(items[i].data->serialize(&val));
	}
	out += '}';
	return out;
}

template <class T>
std::string serialize(const std::vector<T>& val, const std::vector<StructItem>& items)
{
	std::string out;
	for (int i = 0, nbV = val.size(); i < nbV; ++i)
	{
		if (i) out += ",";
		out += serialize(val[i], items);
	}
	return out;
}

void advance(const std::string& text, const std::string& token, size_t& pos)
{
	pos = text.find(token, pos);
	if (pos != std::string::npos)
		++pos;
}

std::string getValueToken(const std::string& text, size_t& pos)
{
	size_t start = pos, len = text.size();
	while (pos < len)
	{
		if ((text[pos] == ',' || text[pos] == '}') && text[pos - 1] != '\\')
			break;
		++pos;
	}

	size_t tokLen = pos - start;
	++pos;
	return text.substr(start, tokLen);
}

std::string getObjectToken(const std::string& text, size_t& pos)
{
	size_t start = pos, len = text.size();
	while (pos < len)
	{
		if (text[pos] == '}' && text[pos - 1] != '\\')
			break;
		++pos;
	}

	++pos;
	size_t tokLen = pos - start;
	return text.substr(start, tokLen);
}

template <class T>
bool deserialize(T& val, const std::string& text, const std::vector<StructItem>& items)
{
	size_t pos = 0;
	advance(text, "{", pos);
	if (pos == std::string::npos)
		return false;

	for (auto& item : items)
	{
		auto token = decode(getValueToken(text, pos));
		item.data->deserialize(&val, token);
	}

	if (text[pos-1] != '}')
		return false;
	return true;
}

template <class T>
void deserialize(std::vector<T>& val, const std::string& text, const std::vector<StructItem>& items)
{
	val.clear();
	size_t pos = 0, len = text.size();
	while (pos < len)
	{
		std::string token = getObjectToken(text, pos);
		T singleValue = T();
		if (deserialize(singleValue, token, items))
			val.push_back(singleValue);
		else
			break;
	}
}

} // namespace details

//****************************************************************************//

struct CORE_API Struct : public Widget, public Serializator
{
	Struct(const std::vector<StructItem>& items) : Widget("struct"), items(items) {}
	const std::vector<StructItem> items;

	template <class T> void setSerialize(std::function<std::string(const T&)>& func) 
	{
		func = [this](const T& val) -> std::string
		{
			return details::serialize(val, items);
		};
	}

	template <class T> void setDeserialize(std::function<void(T&, const std::string& text)>& func) 
	{
		func = [this](T& val, const std::string& text)
		{
			details::deserialize(val, text, items);
		};
	}
};

} // namespace meta
