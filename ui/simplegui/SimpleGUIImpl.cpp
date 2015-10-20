#include <core/BaseDocument.h>
#include <core/Graph.h>

#include <ui/PropertiesDialog.h>

#include <ui/simplegui/SimpleGUIImpl.h>
#include <ui/simplegui/DialogImpl.h>
#include <ui/simplegui/MenuImpl.h>
#include <ui/simplegui/PanelImpl.h>
#include <ui/simplegui/SettingsImpl.h>

#include <QtWidgets>

SimpleGUIImpl::SimpleGUIImpl(QMainWindow* mainWindow, QWidget* view, QWidget* buttonsPanelContainer, const std::vector<QMenu*>& menus)
	: m_mainWindow(mainWindow)
	, m_mainView(view)
	, m_buttonsPanelContainer(buttonsPanelContainer)
	, m_mainMenus(menus)
	, m_settings(std::make_shared<SettingsImpl>(mainWindow))
{
	createButtonsPanel();
}

simplegui::Menu& SimpleGUIImpl::getMenu(MenuType menuType)
{
	return *m_menus[static_cast<unsigned char>(menuType)];
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
	m_menus.clear();
	for (auto menu : m_mainMenus)
		m_menus.push_back(std::make_shared<MenuImpl>(menu));

	// Buttons box
	createButtonsPanel();

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
	DialogImpl* dlgPtr = dialog.get();
	auto removeDialogFunc = [this, dlgPtr](){
		auto it = std::find_if(m_dialogs.begin(), m_dialogs.end(), [dlgPtr](const DialogImplPtr ptr) {
			return dlgPtr == ptr.get();
		});
		if (it != m_dialogs.end())
			m_dialogs.erase(it);
	};
	dialog->setFinishedCallback(removeDialogFunc);
	m_dialogs.push_back(dialog);
	return dialog;
}

void SimpleGUIImpl::updateView()
{
	if (m_mainView)
		m_mainView->update();
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

void SimpleGUIImpl::createButtonsPanel()
{
	auto prevLayout = m_buttonsPanelContainer->layout();
	if (prevLayout)
		QWidget().setLayout(prevLayout); // deleting Layout will not remove the widgets
	auto layout = new QGridLayout(m_buttonsPanelContainer);
	m_buttonsPanel = std::make_shared<PanelImpl>(m_mainWindow, layout);
}
