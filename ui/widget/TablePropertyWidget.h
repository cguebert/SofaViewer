#pragma once

#include <core/Property.h>

#include <QVariant>
#include <QAbstractTableModel>

// This is how the table model will access the values, without knowing their type
class BaseTableValueAccessor
{
public:
	virtual int rowCount() = 0;
	virtual int columnCount() = 0;
	virtual bool fixed() = 0;
	virtual QVariant data(int row, int column) = 0;
	virtual void setData(int row, int column, QVariant value) = 0;
};

template <class T>
QVariant toVariant(const T& value)
{ return QVariant(value); }

template <class T>
T fromVariant(const QVariant& data)
{ return data.value<T>(); }

// Specialization for each supported types
template <class T>
class TableValueAccessor : public BaseTableValueAccessor
{
public:
	using base_value = typename T::value_type;
	TableValueAccessor(Property::PropertyPtr property)
		: m_property(property)
		, m_columnCount(property->columnCount())
	{ m_value = std::dynamic_pointer_cast<PropertyValue<T>>(property->value());	}

	int rowCount() override 	{ return m_value->value().size() / m_columnCount; }
	int columnCount() override	{ return m_columnCount; }
	bool fixed() override		{ return false; }
	QVariant data(int row, int column) override
	{ return toVariant(m_value->value()[row * m_columnCount + column]); }
	void setData(int row, int column, QVariant value) override
	{ m_value->value()[row * m_columnCount + column] = fromVariant<base_value>(value); }

protected:
	Property::PropertyPtr m_property;
	std::shared_ptr<PropertyValue<T>> m_value;
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
