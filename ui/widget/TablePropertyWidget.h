#pragma once

#include <core/Property.h>

#include <QVariant>
#include <QAbstractTableModel>
#include <QObject>

class BaseTableValueAccessor;

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
