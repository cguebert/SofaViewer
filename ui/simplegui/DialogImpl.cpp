#include <ui/MainWindow.h>
#include <ui/simplegui/DialogImpl.h>
#include <ui/simplegui/PanelImpl.h>
#include <ui/widget/PropertyWidget.h>

#include <QtWidgets>

DialogImpl::DialogImpl(MainWindow* mainWindow, const std::string& title)
{
	m_dialog = new QDialog(mainWindow);
	m_dialog->setWindowFlags(m_dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
	m_dialog->setWindowTitle(title.c_str());

	m_panelLayout = new QGridLayout;
	m_dialogPanel = std::make_shared<PanelImpl>(mainWindow, m_panelLayout);
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

	QObject::connect(buttonBox, SIGNAL(accepted()), m_dialog, SLOT(accept()));
	QObject::connect(buttonBox, SIGNAL(rejected()), m_dialog, SLOT(reject()));

	auto mainLayout = new QVBoxLayout;
	mainLayout->addLayout(m_panelLayout);
	mainLayout->addWidget(buttonBox);
	mainLayout->setContentsMargins(5, 5, 5, 5);

	auto layout = m_dialog->layout();
	if(layout)
		delete layout;

	m_dialog->setLayout(mainLayout);
}
