#include <ui/widget/PropertyWidgetFactory.h>
#include <ui/widget/PropertyWidget.h>

#include <ui/widget/StructPropertyWidget.h>

namespace
{

const BasePropertyWidgetCreator* getStructPropertyWidgetCreator()
{
	static PropertyWidgetCreator<StructPropertyWidget> structPropertyWidgetCreator;
	return &structPropertyWidgetCreator;
}

}

PropertyWidgetFactory& PropertyWidgetFactory::instance()
{
	static PropertyWidgetFactory instance;
	return instance;
}

std::unique_ptr<BasePropertyWidget> PropertyWidgetFactory::create(std::shared_ptr<Property> property, QWidget* parent) const
{
	if(!property)
		return nullptr;
	const std::type_index type = property->type();

	std::string widget;
	auto meta = property->getMeta<meta::Widget>();
	if (meta)
		widget = meta->type();
	const auto* ctr = creator(type, widget);
	if(ctr)
		return ctr->create(property, parent);
	return nullptr;
}

const BasePropertyWidgetCreator* PropertyWidgetFactory::creator(const std::type_index type, const std::string& widgetName) const
{
	if (widgetName == "struct")
		return getStructPropertyWidgetCreator();

	auto mapIt = registry.find(type);
	if(mapIt == registry.end())
		return nullptr;

	const auto& map = mapIt->second;

	if(map.size() == 1)
		return map.begin()->second->creator.get();

	PropertyWidgetEntryMap::const_iterator it;
	if (widgetName.empty())
		it = map.find("default");
	else
	{
		it = map.find(widgetName);
		if (it == map.end())	// If the custom widget doesn't exist, first look for a generic one
			it = map.find("generic");

		if (it == map.end())	// Then use the default one
			it = map.find("default");
	}

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
