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
		QObject::connect(m_lineEdit, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()) );
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

RegisterWidget<SimplePropertyWidget<std::string>> PW_string("default");
