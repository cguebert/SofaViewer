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

	virtual void serialize(std::ostream& s, const void* pVal) = 0;
	virtual void deserialize(std::istream& s, void* pVal) = 0;

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

	void serialize(std::ostream& s, const void* pVal) override
	{

	}

	void deserialize(std::istream& s, void* pVal) override
	{

	}

	MetaContainer<T> m_metaContainer;
};

//****************************************************************************//

template <class T>
std::shared_ptr<BaseStructItemData> createStructItem(size_t offset) 
{
	return std::make_shared<StructItemData<T>>(sizeof(T), offset);
}

#define STRUCT_ITEM(s, m) createStructItem<decltype(s::m)>(offsetof(s, m))

struct CORE_API StructItem
{
	StructItem(const std::string& name, std::shared_ptr<BaseStructItemData> data)
		: name(name), data(data) {}

	std::string name;
	std::shared_ptr<BaseStructItemData> data;
};

//****************************************************************************//

struct CORE_API Struct : public Widget, public Serializator
{
	Struct(const std::vector<StructItem>& items) : Widget("struct"), items(items) {}
	std::vector<StructItem> items;

	template <class T> void init(std::function<std::ostream&(std::ostream&, const T&)>& func) 
	{
		func = [this](std::ostream& s, const T& val) -> std::ostream&
		{
			for (const auto& item : items)
				item.data->serialize(s, &val);
			return s;
		};
	}

	template <class T> void init(std::function<std::istream&(std::istream&, T&)>& func) 
	{
		func = [this](std::istream& s, T& val) -> std::istream&
		{
			for (const auto& item : items)
				item.data->deserialize(s, &val);
			return s;
		};
	}
};

} // namespace meta
