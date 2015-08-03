#pragma once

#include <core/Property.h>

#include <QWidget>
#include <QAbstractTableModel>

#include <memory>

// One of this will be created for each Property.
// It is an intermediary between the dialog and the widgets used to show and edit the Property
// This class is a QObject so that we can use signals and slots
class BasePropertyWidget : public QWidget
{
	Q_OBJECT
public:
	BasePropertyWidget(std::shared_ptr<Property> property, QWidget* parent = nullptr);
	virtual ~BasePropertyWidget() {}

	// The implementation of this method holds the widget creation and the signal / slot connections.
	virtual QWidget* createWidgets() = 0;

protected:
	std::shared_ptr<Property> m_property;
};

// Specializations for supported types
template <class T>
class PropertyWidget : public BasePropertyWidget
{
public:
	typedef T value_type;

	PropertyWidget(std::shared_ptr<Property> property, QWidget* parent = nullptr)
		: BasePropertyWidget(property, parent)
		, m_propertyValue(std::dynamic_pointer_cast<PropertyValue<T>>(property->value()))
	{ }

	QWidget* createWidgets() override;

protected:
	std::shared_ptr<PropertyValue<T>> m_propertyValue;
};

// This is how the table model will access the values, without knowing their type
class BaseTableValueAccessor
{
public:
	virtual int rowCount() = 0;
	virtual int columnCount() = 0;
	virtual QVariant data(int row, int column) = 0;
};

// Specialization for each supported types
template <class T>
class TableValueAccessor : public BaseTableValueAccessor
{
public:
	TableValueAccessor(std::vector<T>& data, int columnCount)
		: m_data(data), m_columnCount(columnCount)
	{ }

	int rowCount() override 	{ return m_data.size() / m_columnCount; }
	int columnCount() override	{ return m_columnCount; }
	QVariant data(int row, int column) override
	{
		return QVariant(m_data[row * m_columnCount + column]);
	}

protected:
	std::vector<T>& m_data;
	int m_columnCount;
};

// Table model for property widgets for lists of values
class TablePropertyModel : public QAbstractTableModel
{
public:
	TablePropertyModel(QObject* parent, std::shared_ptr<BaseTableValueAccessor> accessor);

	int rowCount(const QModelIndex& parent) const override;
	int columnCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& parent, int role) const override;

protected:
	std::shared_ptr<BaseTableValueAccessor> m_accessor;
};
