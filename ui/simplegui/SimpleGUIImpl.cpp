#include <core/BaseDocument.h>
#include <core/Graph.h>

#include <ui/MainWindow.h>
#include <ui/OpenGLView.h>
#include <ui/PropertiesDialog.h>

#include <ui/simplegui/SimpleGUIImpl.h>
#include <ui/simplegui/DialogImpl.h>
#include <ui/simplegui/MenuImpl.h>
#include <ui/simplegui/PanelImpl.h>
#include <ui/simplegui/SettingsImpl.h>

#include <QtWidgets>

SimpleGUIImpl::SimpleGUIImpl(MainWindow* mainWindow)
	: m_mainWindow(mainWindow)
	, m_buttonsPanel(std::make_shared<PanelImpl>(mainWindow, mainWindow->buttonsLayout()))
	, m_settings(std::make_shared<SettingsImpl>(mainWindow))
{}

simplegui::Menu& SimpleGUIImpl::getMenu(MenuType menuType)
{
	return *m_mainMenus[static_cast<unsigned char>(menuType)];
}

simplegui::Panel& SimpleGUIImpl::buttonsPanel()
{
	return *m_buttonsPanel;
}

int SimpleGUIImpl::addStatusBarZone(const std::string& text)
{
	auto label = new QLabel(text.c_str());
	label->setAlignment(Qt::AlignLeft);
	label->setMinimumSize(label->sizeHint());

	m_mainWindow->statusBar()->addWidget(label);
	auto id = m_statusBarLabels.size();
	m_statusBarLabels.push_back(label);
	return id;
}

void SimpleGUIImpl::setStatusBarText(int id, const std::string& text)
{
	m_statusBarLabels[id]->setText(text.c_str());
}

void SimpleGUIImpl::clear()
{
	// Status bar
	m_mainWindow->setStatusBar(new QStatusBar);
	m_statusBarLabels.clear();

	// Menus
	m_mainMenus.clear();
	for (unsigned char i = 0; i < 4; ++i)
		m_mainMenus.push_back(std::make_shared<MenuImpl>(m_mainWindow->menu(i)));

	// Buttons box
	auto layout = m_mainWindow->buttonsLayout();
	auto buttonsWidget = layout->parentWidget();
	if(layout)
		QWidget().setLayout(layout); // delete Layout doesn't remove the widgets
	layout = new QGridLayout(buttonsWidget);
	m_buttonsPanel = std::make_shared<PanelImpl>(m_mainWindow, layout);

	// Dialogs
	for(auto dialog : m_dialogs)
		dialog->close();
	m_dialogs.clear();

	for (auto dialog : m_propertiesDialogs)
		dialog.second->deleteLater();
	m_propertiesDialogs.clear();
}

void SimpleGUIImpl::setDocument(std::shared_ptr<BaseDocument> doc)
{
	m_document = doc;
	m_settings->setDocumentType(doc ? doc->documentType() : "");
}

simplegui::Dialog::SPtr SimpleGUIImpl::createDialog(const std::string& title)
{
	auto dialog = std::make_shared<DialogImpl>(m_mainWindow, title);
	m_dialogs.push_back(dialog);
	return dialog;
}

void SimpleGUIImpl::updateView()
{
	m_mainWindow->view()->update();
}

void SimpleGUIImpl::openPropertiesDialog(GraphNode* item)
{
	size_t uniqueId = item->uniqueId;
	auto it = std::find_if(m_propertiesDialogs.begin(), m_propertiesDialogs.end(), [uniqueId](const PropertiesDialogPair& p){
		return p.first == uniqueId;
	});
	if (it != m_propertiesDialogs.end()) // Show existing dialog
	{
		it->second->activateWindow();
		it->second->raise();
		return;
	}

	// Else create a new one
	auto properties = m_document->objectProperties(item);
	if (properties)
	{
		PropertiesDialog* dlg = new PropertiesDialog(properties, m_mainWindow);
		QObject::connect(dlg, &QDialog::finished, [this, dlg](int result) { dialogFinished(dlg, result); });
		m_propertiesDialogs.emplace_back(uniqueId, dlg);
		dlg->show();
	}
}

void SimpleGUIImpl::closePropertiesDialog(ObjectProperties* objProp)
{
	auto it = std::find_if(m_propertiesDialogs.begin(), m_propertiesDialogs.end(), [objProp](const PropertiesDialogPair& p){
		return p.second->objectProperties().get() == objProp;
	});

	if (it != m_propertiesDialogs.end())
		it->second->reject();
}

void SimpleGUIImpl::dialogFinished(PropertiesDialog* dialog, int result)
{
	m_document->closeObjectProperties(dialog->objectProperties(), result == QDialog::Accepted);
	auto it = std::find_if(m_propertiesDialogs.begin(), m_propertiesDialogs.end(), [dialog](const PropertiesDialogPair& p){
		return p.second == dialog;
	});
	if (it != m_propertiesDialogs.end())
		m_propertiesDialogs.erase(it);
	dialog->deleteLater();
}

simplegui::Settings& SimpleGUIImpl::settings()
{
	return *m_settings;
}
