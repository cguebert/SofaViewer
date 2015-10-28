#pragma once

#include <QDialog>

#include <memory>

class BasePropertyWidget;
class GraphNode;
class ObjectProperties;
class Property;

class QGroupBox;
class QTabWidget;

class PropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	PropertiesDialog(std::shared_ptr<ObjectProperties> objectProperties, GraphNode* node, QWidget* parent = nullptr);
	std::shared_ptr<ObjectProperties> objectProperties() const;
	GraphNode* graphNode() const;

protected:
	void apply();
	void applyAndClose();
	void resetWidgets();
	void stateChanged(BasePropertyWidget*, int);

	struct PropertyStruct
	{
		std::shared_ptr<Property> property;
		std::shared_ptr<BasePropertyWidget> widget;
		QGroupBox* groupBox = nullptr;
		QString title;
	};

	using PropertyList = std::vector<PropertyStruct>;
	using IntListIter = std::vector<int>::const_iterator;

	void addTab(QTabWidget* tabWidget, QString name, IntListIter begin, IntListIter end);
	void writeToProperties();
	void readFromProperties();
	bool doApply(); // Returns false if there is a conflict, and the user cancelled

	std::shared_ptr<ObjectProperties> m_objectProperties;
	PropertyList m_propertyWidgets;
	GraphNode* m_graphNode;
};

inline std::shared_ptr<ObjectProperties> PropertiesDialog::objectProperties() const
{ return m_objectProperties; }

inline GraphNode* PropertiesDialog::graphNode() const
{ return m_graphNode; }