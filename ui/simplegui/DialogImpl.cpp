#include <ui/simplegui/DialogImpl.h>
#include <ui/simplegui/PanelImpl.h>
#include <ui/widget/PropertyWidget.h>

#include <QtWidgets>

DialogImpl::DialogImpl(QWidget* parent, const std::string& title)
	: m_dialog(new QDialog(parent))
	, m_panelLayout(new QGridLayout)
	, m_dialogPanel(std::make_shared<PanelImpl>(parent, m_panelLayout))
{
	m_dialog->setWindowFlags(m_dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
	m_dialog->setWindowTitle(title.c_str());
}

simplegui::Panel& DialogImpl::content()
{
	return *m_dialogPanel;
}

bool DialogImpl::exec()
{
	completeLayout();
	auto result = m_dialog->exec();
	if(result)
	{
		for(const auto& widget : m_dialogPanel->propertyWidgets())
			widget->updatePropertyValue();
	}
	return result != 0;
}

void DialogImpl::show()
{
	if (!m_created)
		completeLayout();
	m_dialog->show();
}

void DialogImpl::close()
{
	m_dialog->reject();
}

void DialogImpl::completeLayout()
{
	auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
										  QDialogButtonBox::Cancel);

	QObject::connect(buttonBox, &QDialogButtonBox::accepted, m_dialog, &QDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, m_dialog, &QDialog::reject);

	auto mainLayout = new QVBoxLayout;
	mainLayout->addLayout(m_panelLayout);
	mainLayout->addWidget(buttonBox);
	mainLayout->setContentsMargins(5, 5, 5, 5);

	auto layout = m_dialog->layout();
	if(layout)
		delete layout;

	m_dialog->setLayout(mainLayout);
	m_created = true;
}
