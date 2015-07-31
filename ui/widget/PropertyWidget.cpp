#include <ui/widget/PropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>

#include <core/Property.h>

#include <QtWidgets>

BasePropertyWidget::BasePropertyWidget(std::shared_ptr<Property> property, QWidget* parent)
	: QWidget(parent)
	, m_property(property)
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
	spinBox->setValue(m_propertyValue->value());

	return spinBox;
}

QWidget* PropertyWidget<float>::createWidgets(bool readOnly)
{
	auto lineEdit = new QLineEdit(parentWidget());
	lineEdit->setText(QString::number(m_propertyValue->value()));
	lineEdit->setEnabled(!readOnly);

	return lineEdit;
}

QWidget* PropertyWidget<double>::createWidgets(bool readOnly)
{
	auto lineEdit = new QLineEdit(parentWidget());
	lineEdit->setText(QString::number(m_propertyValue->value()));
	lineEdit->setEnabled(!readOnly);

	return lineEdit;
}

QWidget* PropertyWidget<std::string>::createWidgets(bool readOnly)
{
	auto lineEdit = new QLineEdit(parentWidget());
	lineEdit->setText(m_propertyValue->value().c_str());
	lineEdit->setEnabled(!readOnly);

	return lineEdit;
}

QWidget* PropertyWidget<std::vector<int>>::createWidgets(bool readOnly)
{
	auto table = new QTableView(parentWidget());
	table->setEnabled(!readOnly);
	auto& value = m_propertyValue->value();
	auto accessor = std::make_shared<TableValueAccessor<int>>(value, m_property->columnCount());
	auto model = new TablePropertyModel(table, accessor);
	table->setModel(model);

	return table;
}

QWidget* PropertyWidget<std::vector<float>>::createWidgets(bool readOnly)
{
	auto table = new QTableView(parentWidget());
	table->setEnabled(!readOnly);
	auto& value = m_propertyValue->value();
	auto accessor = std::make_shared<TableValueAccessor<float>>(value, m_property->columnCount());
	auto model = new TablePropertyModel(table, accessor);
	table->setModel(model);

	return table;
}

QWidget* PropertyWidget<std::vector<double>>::createWidgets(bool readOnly)
{
	auto table = new QTableView(parentWidget());
	table->setEnabled(!readOnly);
	auto& value = m_propertyValue->value();
	auto accessor = std::make_shared<TableValueAccessor<double>>(value, m_property->columnCount());
	auto model = new TablePropertyModel(table, accessor);
	table->setModel(model);

	return table;
}

template <>
QVariant TableValueAccessor<std::string>::data(int row, int column)
{
	return QVariant(m_data[row * m_columnCount + column].c_str());
}

QWidget* PropertyWidget<std::vector<std::string>>::createWidgets(bool readOnly)
{
	auto table = new QTableView(parentWidget());
	table->setEnabled(!readOnly);
	auto& value = m_propertyValue->value();
	auto accessor = std::make_shared<TableValueAccessor<std::string>>(value, m_property->columnCount());
	auto model = new TablePropertyModel(table, accessor);
	table->setModel(model);

	return table;
}

RegisterWidget<PropertyWidget<int>> PW_int("default");
RegisterWidget<PropertyWidget<float>> PW_float("default");
RegisterWidget<PropertyWidget<double>> PW_double("default");
RegisterWidget<PropertyWidget<std::string>> PW_string("default");
RegisterWidget<PropertyWidget<std::vector<int>>> PW_vector_int("default");
RegisterWidget<PropertyWidget<std::vector<float>>> PW_vector_float("default");
RegisterWidget<PropertyWidget<std::vector<double>>> PW_vector_double("default");
RegisterWidget<PropertyWidget<std::vector<std::string>>> PW_vector_string("default");

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
