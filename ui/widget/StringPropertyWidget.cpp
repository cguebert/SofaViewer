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
		QString text = QString::fromStdString(v);
		if (m_lineEdit->text() != text)
			m_lineEdit->setText(text);
	}
	void writeToProperty(value_type& v)
	{
		v = m_lineEdit->text().toStdString();
	}
};

//****************************************************************************//

class DirectoryPropertyWidget
{
protected:
	typedef std::string value_type;
	QLineEdit* m_lineEdit = nullptr;
	BasePropertyWidget* m_parent = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_parent = parent;
		
		m_lineEdit = new QLineEdit(parent);
		m_lineEdit->setEnabled(!parent->readOnly());
		QObject::connect(m_lineEdit, &QLineEdit::editingFinished, parent, &BasePropertyWidget::setWidgetDirty);

		auto button = new QPushButton("...", parent);
		QObject::connect(button, &QPushButton::clicked, [this]() { chooseDirectory(); });

		auto container = new QWidget(parent);
		auto layout = new QHBoxLayout();
		layout->addWidget(m_lineEdit);
		layout->addWidget(button);
		layout->setContentsMargins(0, 0, 0, 0);
		container->setLayout(layout);

		return container;
	}
	void chooseDirectory()
	{
		auto dir = QFileDialog::getExistingDirectory(m_parent, "", m_lineEdit->text());
		if (!dir.isEmpty())
		{
			m_lineEdit->setText(dir);
			m_parent->setWidgetDirty();
		}
	}
	void readFromProperty(const value_type& v)
	{
		QString text = QString::fromStdString(v);
		if (m_lineEdit->text() != text)
			m_lineEdit->setText(text);
	}
	void writeToProperty(value_type& v)
	{
		v = m_lineEdit->text().toStdString();
	}
};

//****************************************************************************//

class FilePropertyWidget
{
protected:
	typedef std::string value_type;
	QLineEdit* m_lineEdit = nullptr;
	BasePropertyWidget* m_parent = nullptr;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_parent = parent;
		
		m_lineEdit = new QLineEdit(parent);
		m_lineEdit->setEnabled(!parent->readOnly());
		QObject::connect(m_lineEdit, &QLineEdit::editingFinished, parent, &BasePropertyWidget::setWidgetDirty);

		auto button = new QPushButton("...", parent);
		QObject::connect(button, &QPushButton::clicked, [this]() { chooseFile(); });

		auto container = new QWidget(parent);
		auto layout = new QHBoxLayout();
		layout->addWidget(m_lineEdit);
		layout->addWidget(button);
		layout->setContentsMargins(0, 0, 0, 0);
		container->setLayout(layout);

		return container;
	}
	void chooseFile()
	{
		auto fileMeta = m_parent->property()->getMeta<meta::File>();
		QString filter;
		if (fileMeta)
			filter = QString::fromStdString(fileMeta->filter);
		auto path = QFileDialog::getOpenFileName(m_parent, "", "", filter);
		if (!path.isEmpty())
		{
			m_lineEdit->setText(path);
			m_parent->setWidgetDirty();
		}
	}
	void readFromProperty(const value_type& v)
	{
		QString text = QString::fromStdString(v);
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
		auto enumMeta = parent->property()->getMeta<meta::Enum>();
		if (enumMeta)
		{
			for (const auto& v : enumMeta->values)
				m_enumValues.push_back(QString::fromStdString(v));
		}
		m_comboBox->addItems(m_enumValues);

		void(QComboBox::*currentIndexChanged)(int) = &QComboBox::currentIndexChanged; // Get the right overload
		QObject::connect(m_comboBox, currentIndexChanged, parent, &BasePropertyWidget::setWidgetDirty);
		return m_comboBox;
	}
	void readFromProperty(const value_type& v)
	{
		QSignalBlocker block(m_comboBox);
		QString text = QString::fromStdString(v);
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
RegisterWidget<SimplePropertyWidget<std::string, DirectoryPropertyWidget>> PW_string_directory("directory");
RegisterWidget<SimplePropertyWidget<std::string, FilePropertyWidget>> PW_string_file("file");
RegisterWidget<SimplePropertyWidget<std::string, EnumStringPropertyWidget>> PW_string_enum("enum");
