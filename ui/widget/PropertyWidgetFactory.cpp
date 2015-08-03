#include <ui/widget/PropertyWidgetFactory.h>

#include <core/Property.h>

PropertyWidgetFactory& PropertyWidgetFactory::instance()
{
	static PropertyWidgetFactory instance;
	return instance;
}

BasePropertyWidget* PropertyWidgetFactory::create(std::shared_ptr<Property> property, QWidget* parent) const
{
	if(!property)
		return nullptr;
	const std::type_index type = property->type();
	const std::string widgetName = ""; //property->widget();

	const auto* ctr = creator(type, widgetName);
	if(ctr)
		return ctr->create(property, parent);
	return nullptr;
}

const BasePropertyWidgetCreator* PropertyWidgetFactory::creator(const std::type_index type, const std::string& widgetName) const
{
	auto mapIt = registry.find(type);
	if(mapIt == registry.end())
		return nullptr;

	const auto& map = mapIt->second;

	// Special case : for lists and animations, we use the same PropertyWidget which will create other ones later
	if(map.size() == 1)
		return map.begin()->second->creator.get();

	auto it = map.find(widgetName);
	if(it == map.end())	// If the custom widget doesn't exist, first look for a generic one
		it = map.find("generic");

	if(it == map.end())	// Then use the default one
		it = map.find("default");

	// If a default one doesn't exist, we don't know which one to use
	if(it != map.end())
		return it->second->creator.get();

	return nullptr;
}

const PropertyWidgetFactory::PropertyWidgetEntry* PropertyWidgetFactory::entry(const std::type_index type, const std::string& widgetName) const
{
	const auto itType = registry.find(type);
	if(itType == registry.end())
		return nullptr;

	const auto& widgets = itType->second;
	const auto itWidget = widgets.find(widgetName);
	if(itWidget == widgets.end())
		return nullptr;

	return itWidget->second.get();
}

std::vector<std::string> PropertyWidgetFactory::widgetNames(const std::type_index type) const
{
	std::vector<std::string> result;
	auto it = registry.find(type);
	if(it == registry.end())
		return result;

	for(const auto& widget : it->second)
		result.push_back(widget.second->widgetName);
	return result;
}

void PropertyWidgetFactory::registerWidget(const std::type_index type, const std::string& widgetName, PropertyWidgetCreatorPtr creator)
{
	auto entry = std::make_shared<PropertyWidgetEntry>();
	entry->type = type;
	entry->widgetName = widgetName;
	entry->creator = creator;

	registry[type][widgetName] = entry;
}