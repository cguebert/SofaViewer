#include <ui/PropertyWidget.h>

#include <core/Property.h>

#include <QtWidgets>

BasePropertyWidget::BasePropertyWidget(std::shared_ptr<BasePropertyValue> property, QWidget* parent)
	: QWidget(parent)
	, m_baseProperty(property)
{
}

/*****************************************************************************/

QWidget* PropertyWidget<int>::createWidgets(bool readOnly)
{
	auto spinBox = new QSpinBox(parentWidget());
	spinBox->setMinimum(INT_MIN);
	spinBox->setMaximum(INT_MAX);
	spinBox->setSingleStep(1);
	spinBox->setEnabled(!readOnly);

	return spinBox;
}

template <class T>
int getColumnCount(const T& val)
{
	int size = val.size();
	if(size % 3 == 0)
		return 3;
	if(size % 2 == 0)
		return 2;
	return 1;
}

QWidget* PropertyWidget<std::vector<int>>::createWidgets(bool readOnly)
{
	auto table = new QTableView(parentWidget());
	table->setEnabled(!readOnly);
	auto& value = m_property->value();
	auto accessor = std::make_shared<TableValueAccessor<int>>(value, getColumnCount(value));
	auto model = new TablePropertyModel(table, accessor);
	table->setModel(model);

	return table;
}

QWidget* PropertyWidget<std::vector<float>>::createWidgets(bool readOnly)
{
	auto table = new QTableView(parentWidget());
	table->setEnabled(!readOnly);
	auto& value = m_property->value();
	auto accessor = std::make_shared<TableValueAccessor<float>>(value, getColumnCount(value));
	auto model = new TablePropertyModel(table, accessor);
	table->setModel(model);

	return table;
}

QWidget* PropertyWidget<std::vector<double>>::createWidgets(bool readOnly)
{
	auto table = new QTableView(parentWidget());
	table->setEnabled(!readOnly);
	auto& value = m_property->value();
	auto accessor = std::make_shared<TableValueAccessor<double>>(value, getColumnCount(value));
	auto model = new TablePropertyModel(table, accessor);
	table->setModel(model);

	return table;
}

/*****************************************************************************/

TablePropertyModel::TablePropertyModel(QObject* parent, std::shared_ptr<BaseTableValueAccessor> accessor)
	: QAbstractTableModel(parent)
	, m_accessor(accessor)
{
}

int TablePropertyModel::rowCount(const QModelIndex& /*index*/) const
{
	return m_accessor->rowCount();
}

int TablePropertyModel::columnCount(const QModelIndex& /*index*/) const
{
	return m_accessor->columnCount();
}

QVariant TablePropertyModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	return m_accessor->data(index.row(), index.column());
}
