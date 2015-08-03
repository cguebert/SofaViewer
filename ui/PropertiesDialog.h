#pragma once

#include <QDialog>

#include <memory>

class ObjectProperties;
class Property;
class BasePropertyWidget;
class QTabWidget;

class PropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	PropertiesDialog(std::shared_ptr<ObjectProperties> properties, QWidget* parent = nullptr);

protected:
	using PropertyPair = std::pair<std::shared_ptr<Property>, std::shared_ptr<BasePropertyWidget>>;
	using PropertyPairList = std::vector<PropertyPair>;
	using PropertyPairListIter = PropertyPairList::const_iterator;

	void addTab(QTabWidget* tabWidget, QString name, PropertyPairListIter begin, PropertyPairListIter end);

	std::shared_ptr<ObjectProperties> m_properties;
	PropertyPairList m_propertyWidgets;
};
