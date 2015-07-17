#include <ui/PropertiesDialog.h>

#include <core/ObjectProperties.h>

#include <QtWidgets>

PropertiesDialog::PropertiesDialog(std::shared_ptr<ObjectProperties> properties, QWidget* parent)
	: QDialog(parent)
	, m_properties(properties)
{
	setMinimumSize(300, 200);
	setWindowTitle(properties->m_name.c_str());

	auto layout = new QVBoxLayout;
	for(const auto& prop : properties->m_properties)
	{
		auto label = new QLabel(prop.name().c_str());
		layout->addWidget(label);
	}

	setLayout(layout);
}
