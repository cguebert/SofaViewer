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
template <class T> class TableValueAccessor;

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
