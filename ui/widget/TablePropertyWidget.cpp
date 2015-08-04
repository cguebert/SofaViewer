#include <ui/widget/TablePropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>
#include <ui/widget/SimplePropertyWidget.h>

#include <QtWidgets>

template <class T>
class TableWidgetContainer
{
protected:
	using value_type = T;
	using base_type = typename T::value_type;
	QTableView* m_view = nullptr;
	TablePropertyModel* m_model = nullptr;
	std::shared_ptr<BaseTableValueAccessor> m_accessor = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_view = new QTableView(parent);
		m_view->setEnabled(!parent->readOnly());

		m_accessor = std::make_shared<TableValueAccessor<value_type>>(parent->property());
		m_model = new TablePropertyModel(m_view, m_accessor);
		m_view->setModel(m_model);
		return m_view;
	}
	void readFromProperty(const value_type& v)
	{
	}
	void writeToProperty(value_type& v)
	{
	}
};

template <>
QVariant toVariant(const std::string& value)
{ return QVariant(value.c_str()); }

template <>
std::string fromVariant(const QVariant& data)
{ return data.toString().toLocal8Bit().constData(); }

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

/*****************************************************************************/

template <class T> using TablePropertyWidget = SimplePropertyWidget<T, TableWidgetContainer<T>>;
RegisterWidget<TablePropertyWidget<std::vector<int>>> PW_vector_int("default");
RegisterWidget<TablePropertyWidget<std::vector<float>>> PW_vector_float("default");
RegisterWidget<TablePropertyWidget<std::vector<double>>> PW_vector_double("default");
RegisterWidget<TablePropertyWidget<std::vector<std::string>>> PW_vector_string("default");
