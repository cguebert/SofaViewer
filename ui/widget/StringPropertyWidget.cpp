#include <ui/widget/PropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>
#include <ui/widget/SimplePropertyWidget.h>

#include <QtWidgets>

template<>
class PropretyWidgetContainer<std::string>
{
protected:
	typedef std::string value_type;
	QLineEdit* m_lineEdit = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_lineEdit = new QLineEdit(parent);
		m_lineEdit->setEnabled(!parent->readOnly());
		QObject::connect(m_lineEdit, &QLineEdit::editingFinished, parent, &BasePropertyWidget::setWidgetDirty);
		return m_lineEdit;
	}
	void readFromProperty(const value_type& v)
	{
		QString text = v.c_str();
		if (m_lineEdit->text() != text)
			m_lineEdit->setText(text);
	}
	void writeToProperty(value_type& v)
	{
		v = m_lineEdit->text().toStdString();
	}
};

//****************************************************************************//

class EnumStringPropertyWidget
{
protected:
	typedef std::string value_type;
	QComboBox* m_comboBox = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_comboBox = new QComboBox(parent);
		m_comboBox->setEnabled(!parent->readOnly());
		auto enumMeta = parent->property()->value()->meta().get<meta::Enum>();
		if (enumMeta)
		{
			for (const auto& v : enumMeta->values)
				m_enumValues.push_back(v.c_str());
		}
		m_comboBox->addItems(m_enumValues);

		QObject::connect(m_comboBox, &QComboBox::setCurrentIndex, parent, &BasePropertyWidget::setWidgetDirty);
		return m_comboBox;
	}
	void readFromProperty(const value_type& v)
	{
		QString text = v.c_str();
		if (m_comboBox->currentText() != text)
			m_comboBox->setCurrentText(text);
	}
	void writeToProperty(value_type& v)
	{
		v = m_comboBox->currentText().toStdString();
	}

protected:
	QStringList m_enumValues;
};

//****************************************************************************//

RegisterWidget<SimplePropertyWidget<std::string>> PW_string("default");
RegisterWidget<SimplePropertyWidget<std::string, EnumStringPropertyWidget>> PW_string_enum("enum");
