#pragma once

#include <map>
#include <memory>
#include <typeindex>
#include <vector>

class QWidget;
class BasePropertyWidget;
class Property;

class BasePropertyWidgetCreator
{
public:
	virtual ~BasePropertyWidgetCreator() {}
	virtual std::unique_ptr<BasePropertyWidget> create(std::shared_ptr<Property> property, QWidget* parent) const = 0;
};

class PropertyWidgetFactory
{
private:
	PropertyWidgetFactory() {}

public:
	typedef std::shared_ptr<BasePropertyWidgetCreator> PropertyWidgetCreatorPtr;

	class PropertyWidgetEntry
	{
	public:
		using SPtr = std::shared_ptr<PropertyWidgetEntry>;

		PropertyWidgetEntry() : type(typeid(int)) {}

		std::type_index type;
		std::string widgetName;
		PropertyWidgetCreatorPtr creator;
	};

	static PropertyWidgetFactory& instance();
	const PropertyWidgetEntry* entry(std::type_index type, const std::string& widgetName) const;
	const BasePropertyWidgetCreator* creator(std::type_index type, const std::string& widgetName) const;
	std::vector<std::string> widgetNames(std::type_index type) const;

	std::unique_ptr<BasePropertyWidget> create(std::shared_ptr<Property> property, QWidget* parent) const;

protected:
	using PropertyWidgetEntryMap = std::map<std::string, PropertyWidgetEntry::SPtr>;
	typedef std::map<std::type_index, PropertyWidgetEntryMap> RegistryMap;
	RegistryMap registry;

	template<class T> friend class RegisterWidget;
	void registerWidget(std::type_index index, const std::string& widgetName, PropertyWidgetCreatorPtr creator);
};

template<class T>
class PropertyWidgetCreator : public BasePropertyWidgetCreator
{
public:
	virtual std::unique_ptr<BasePropertyWidget> create(std::shared_ptr<Property> property, QWidget* parent) const
	{ return std::make_unique<T>(property, parent); }
};

template <class T>
class RegisterWidget
{
public:
	explicit RegisterWidget(const std::string& widgetName)
	{
		std::type_index type = std::type_index(typeid(T::value_type));
		auto creator = std::make_shared<PropertyWidgetCreator<T>>();
		auto& factory = PropertyWidgetFactory::instance();
		factory.registerWidget(type, widgetName, creator);
	}

private:
	RegisterWidget();
};
