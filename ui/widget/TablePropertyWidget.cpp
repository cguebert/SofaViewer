#include <ui/widget/TablePropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>
#include <ui/widget/SimplePropertyWidget.h>

#include <core/VectorWrapper.h>

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
	QSpinBox* m_spinBox = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_view = new QTableView(parent);
		m_view->setEnabled(!parent->readOnly());

		m_accessor = std::make_shared<TableValueAccessor<value_type>>(parent->property());
		m_model = new TablePropertyModel(m_view, m_accessor);
		m_view->setModel(m_model);

		if(m_accessor->fixed())
		{
			// Hide headers
			m_view->horizontalHeader()->hide();
			m_view->verticalHeader()->hide();
			m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

			// Set max height
			auto maxHeight = m_view->rowHeight(0) * m_accessor->rowCount();
			m_view->setMaximumHeight(maxHeight);

			// Set min width
			int width = 0;
			for(int i = 0, nb = m_accessor->columnCount(); i < nb; ++i)
				width += m_view->columnWidth(i);
			m_view->setMinimumWidth(width);

			return m_view;
		}
		else
		{
			// Add a spinbox to be able to resize the value
			auto container = new QWidget(parent);
			auto layout = new QVBoxLayout(container);
			layout->setContentsMargins(0, 0, 0, 0);
			m_spinBox = new QSpinBox;
			m_spinBox->setMaximum(INT_MAX);
			m_spinBox->setValue(m_accessor->rowCount());
			layout->addWidget(m_spinBox);
			layout->addWidget(m_view);
			return container;
		}
	}
	void readFromProperty(const value_type& /*v*/)
	{
	}
	void writeToProperty(value_type& /*v*/)
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

template <class T>
class TableValueAccessor<VectorWrapper<T>> : public BaseTableValueAccessor
{
public:
	using wrapper_type = VectorWrapper<T>;
	using base_value = typename T::value_type;
	TableValueAccessor(Property::PropertyPtr property)
		: m_property(property)
	{
		m_value = std::dynamic_pointer_cast<PropertyValue<wrapper_type>>(property->value());
		m_columnCount = m_value->value().columnCount();
		m_fixed = m_value->value().fixedSize();
	}

	int rowCount() override 	{ return m_value->value().value().size() / m_columnCount; }
	int columnCount() override	{ return m_columnCount; }
	bool fixed() override		{ return m_fixed; }
	QVariant data(int row, int column) override
	{ return toVariant(m_value->value().value()[row * m_columnCount + column]); }
	void setData(int row, int column, QVariant value) override
	{ m_value->value().value()[row * m_columnCount + column] = fromVariant<base_value>(value); }

protected:
	Property::PropertyPtr m_property;
	std::shared_ptr<PropertyValue<wrapper_type>> m_value;
	int m_columnCount;
	bool m_fixed;
};

/*****************************************************************************/

template <class T> using TablePropertyWidget = SimplePropertyWidget<T, TableWidgetContainer<T>>;
RegisterWidget<TablePropertyWidget<VectorWrapper<std::vector<int>>>> PW_vector_wrapper_int("default");
RegisterWidget<TablePropertyWidget<VectorWrapper<std::vector<float>>>> PW_vector_wrapper_float("default");
RegisterWidget<TablePropertyWidget<VectorWrapper<std::vector<double>>>> PW_vector_wrapper_double("default");
RegisterWidget<TablePropertyWidget<VectorWrapper<std::vector<std::string>>>> PW_vector_wrapper_string("default");
