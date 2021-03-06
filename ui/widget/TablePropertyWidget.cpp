#include <ui/widget/TablePropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>
#include <ui/widget/SimplePropertyWidget.h>

#include <core/VectorWrapper.h>

#include <QtWidgets>

#include <cassert>

namespace
{

template <class T>
QVariant toVariant(const T& value)
{ return QVariant(value); }

template <class T>
T fromVariant(const QVariant& data)
{ return data.value<T>(); }

template <>
QVariant toVariant(const std::string& value)
{ return QVariant(QString::fromStdString(value)); }

template <>
std::string fromVariant(const QVariant& data)
{ return data.toString().toStdString(); }

}

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

// Specialization for each supported types
template <class T> class TableValueAccessor; // Don't forget to implement value/setValue

template <class T>
class TableWidgetContainer
{
protected:
	using value_type = T;
	QTableView* m_view = nullptr;
	TablePropertyModel* m_model = nullptr;
	std::shared_ptr<TableValueAccessor<value_type>> m_accessor = nullptr;
	QSpinBox* m_spinBox = nullptr;
	QPushButton* m_toggleButton = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_view = new QTableView(parent);
		m_view->setEnabled(!parent->readOnly());

		auto value = parent->property()->value<value_type>();
		m_accessor = std::make_shared<TableValueAccessor<value_type>>(value->value());
		m_model = new TablePropertyModel(m_view, m_accessor);
		QObject::connect(m_model, &TablePropertyModel::modified, parent, &BasePropertyWidget::setWidgetDirty);
		m_view->setModel(m_model);

		if(m_accessor->fixed())
		{
			// Hide headers
			m_view->horizontalHeader()->hide();
			m_view->verticalHeader()->hide();
			m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

			// Set height
			auto height = m_view->rowHeight(0) * m_accessor->rowCount() + 2;
			m_view->setMinimumHeight(height);
			m_view->setMaximumHeight(height);

			// Set min width
			int width = 0;
			for(int i = 0, nb = m_accessor->columnCount(); i < nb; ++i)
				width += m_view->columnWidth(i);
			m_view->setMinimumWidth(width);

			return m_view;
		}
		else
		{
			// Add a spinbox and a button to be able to resize the value
			// and a button to show / hide the table view
			auto container = new QWidget(parent);
			auto layout = new QVBoxLayout(container);
			layout->setContentsMargins(0, 0, 0, 0);

			auto topLayout = new QHBoxLayout;
			m_toggleButton = new QPushButton(QPushButton::tr("show"));
			m_toggleButton->setCheckable(true);
			QObject::connect(m_toggleButton, &QPushButton::toggled, [this](bool toggled) { toggleView(toggled); });
			topLayout->addWidget(m_toggleButton);

			m_spinBox = new QSpinBox;
			m_spinBox->setMaximum(INT_MAX);
			m_spinBox->setValue(m_accessor->rowCount());
			topLayout->addWidget(m_spinBox, 1);

			auto resizeButton = new QPushButton(QPushButton::tr("resize"));
			QObject::connect(resizeButton, &QPushButton::clicked, [this]() { resizeValue(); });
			QObject::connect(resizeButton, &QPushButton::clicked, parent, &BasePropertyWidget::setWidgetDirty);
			topLayout->addWidget(resizeButton);

			layout->addLayout(topLayout);
			layout->addWidget(m_view, 1);

			m_view->setMinimumHeight(200);
			m_view->hide();
			return container;
		}
	}
	void readFromProperty(const value_type& v)
	{
		m_model->beginReset();
		m_accessor->setValue(v);
		m_model->endReset();

		if(m_spinBox)
			m_spinBox->setValue(m_accessor->rowCount());

	}
	void writeToProperty(value_type& v)
	{
		v = m_accessor->value();
	}
	void resizeValue()
	{
		int nb = m_spinBox->value();
		m_model->resizeValue(nb);
	}
	void toggleView(bool show)
	{
		m_view->setVisible(show);
		m_toggleButton->setText(show ? QPushButton::tr("hide") : QPushButton::tr("show"));
	}
};

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

	if (role != Qt::DisplayRole && role != Qt::EditRole)
		return QVariant();

	return m_accessor->data(index.row(), index.column());
}

QVariant TablePropertyModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
	if(role != Qt::DisplayRole)
		return QVariant();

	return QVariant(section);
}

Qt::ItemFlags TablePropertyModel::flags(const QModelIndex& ) const
{
	return Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

bool TablePropertyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(role != Qt::EditRole)
		return false;

	m_accessor->setData(index.row(), index.column(), value);
	emit modified();
	return true;
}

void TablePropertyModel::resizeValue(int nb)
{
	int prevNb = m_accessor->rowCount();
	if(nb == prevNb)
		return;

	if(nb > prevNb)
	{
		beginInsertRows(QModelIndex(), prevNb, nb - 1);
		m_accessor->resize(nb);
		endInsertRows();
	}
	else
	{
		beginRemoveRows(QModelIndex(), nb, prevNb - 1);
		m_accessor->resize(nb);
		endRemoveRows();
	}
}

void TablePropertyModel::beginReset()
{
	beginResetModel();
}

void TablePropertyModel::endReset()
{
	endResetModel();
}

/*****************************************************************************/

template <class T>
class TableValueAccessor<VectorWrapper<T>> : public BaseTableValueAccessor
{
public:
	using wrapper_type = VectorWrapper<T>;
	using base_value = typename T::value_type;
	TableValueAccessor(const wrapper_type& value)
		: m_value(value) {}

	const wrapper_type& value() const { return m_value; }
	void setValue(const wrapper_type& value)
	{
		assert(m_value.columnCount() == value.columnCount());
		assert(m_value.fixedSize() == value.fixedSize());
		m_value = value;
	}

	int rowCount() const override 	{ return m_value.rowCount(); }
	int columnCount() const override	{ return m_value.columnCount(); }
	bool fixed() const override		{ return m_value.fixedSize(); }
	QVariant data(int row, int column) const override
	{ return toVariant(m_value.get(row, column)); }
	void setData(int row, int column, QVariant value) override
	{ m_value.set(row, column, fromVariant<base_value>(value)); }
	void resize(int nb) override
	{ m_value.setRowCount(nb); }

protected:
	wrapper_type m_value;
};

/*****************************************************************************/

template <class T>
class TableValueAccessor<std::vector<T>> : public BaseTableValueAccessor
{
public:
	using vector_type = std::vector<T>;
	using base_value = typename T;
	TableValueAccessor(const vector_type& value) : m_value(value) {}

	const vector_type& value() const { return m_value; }
	void setValue(const vector_type& value) { m_value = value; }

	int rowCount() const override 	{ return m_value.size(); }
	int columnCount() const override	{ return 1; }
	bool fixed() const override		{ return false; }
	QVariant data(int row, int /*column*/) const override
	{ return toVariant(m_value[row]); }
	void setData(int row, int /*column*/, QVariant value) override
	{ m_value[row] = fromVariant<base_value>(value); }
	void resize(int nb) override
	{ m_value.resize(nb); }

protected:
	vector_type m_value;
};

/*****************************************************************************/

template <class T>
class RegisterTableWidget
{
public:
	template <class U> using TablePropertyWidget = SimplePropertyWidget<U, TableWidgetContainer<U>>;

	explicit RegisterTableWidget(const std::string& widgetName)
	{
		using vector_type = std::vector<T>;
		RegisterWidget<TablePropertyWidget<vector_type>> PW_vector(widgetName);
		RegisterWidget<TablePropertyWidget<VectorWrapper<vector_type>>> PW_vector_wrapper(widgetName);
	}
};
/*****************************************************************************/

RegisterTableWidget<int> PW_vector_int("default");
RegisterTableWidget<unsigned int> PW_vector_unsigned_int("default");
RegisterTableWidget<float> PW_vector_float("default");
RegisterTableWidget<double> PW_vector_double("default");
RegisterTableWidget<std::string> PW_vector_string("default");
