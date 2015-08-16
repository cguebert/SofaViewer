#pragma once

#include <core/Property.h>

#include <QVariant>
#include <QAbstractTableModel>
#include <QObject>

// This is how the table model will access the values, without knowing their type
class BaseTableValueAccessor
{
public:
	virtual int rowCount() const = 0;
	virtual int columnCount() const = 0;
	virtual bool fixed() const = 0;
	virtual QVariant data(int row, int column) const = 0;
	virtual void setData(int row, int column, QVariant value) = 0;
	virtual void resize(int nb) = 0;
};

class BaseTableWidgetContainer : public QObject
{
	Q_OBJECT
public slots:
	virtual void resizeValue() {}
	virtual void toggleView(bool) {}
};

template <class T>
QVariant toVariant(const T& value)
{ return QVariant(value); }

template <class T>
T fromVariant(const QVariant& data)
{ return data.value<T>(); }

// Specialization for each supported types
template <class T> class TableValueAccessor; // Don't forget to implement value/setValue

// Table model for property widgets for lists of values
class TablePropertyModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	TablePropertyModel(QObject* parent, std::shared_ptr<BaseTableValueAccessor> accessor);

	int rowCount(const QModelIndex& parent) const override;
	int columnCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& parent, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;

	void resizeValue(int nb);

	void beginReset();
	void endReset();

signals:
	void modified();

protected:
	std::shared_ptr<BaseTableValueAccessor> m_accessor;
};
