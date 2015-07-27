#pragma once

#include <QDialog>

#include <memory>

class Document;
class ObjectProperties;
class Property;

class PropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	PropertiesDialog(std::shared_ptr<ObjectProperties> properties, QWidget* parent = nullptr);

protected:
	std::shared_ptr<ObjectProperties> m_properties;
	std::vector<std::pair<Property&, QWidget*>> m_propertyWidgets;
};
