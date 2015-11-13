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
	auto label = new QLabel(QString::fromStdString(text));
	label->setAlignment(Qt::AlignLeft);
	label->setMinimumSize(label->sizeHint());

	m_mainWindow->statusBar()->addWidget(label);
	auto id = m_statusBarLabels.size();
	m_statusBarLabels.push_back(label);
	return id;
}

void SimpleGUIImpl::setStatusBarText(int id, const std::string& text)
{
	m_statusBarLabels[id]->setText(QString::fromStdString(text));
}

void SimpleGUIImpl::clear()
{
	// Status bar
	m_mainWindow->setStatusBar(new QStatusBar);
	m_statusBarLabels.clear();

	// Menus
	m_menus.clear();
	for (auto menu : m_mainMenus)
		m_menus.push_back(std::make_shared<MenuImpl>(menu, true));

	// Buttons box
	createButtonsPanel();

	// Dialogs
	for(auto dialog : m_dialogs)
		dialog->close();
	m_dialogs.clear();

	std::lock_guard<std::mutex> lock(m_propertiesDialogsMutex);
	for (auto dialog : m_propertiesDialogs)
		dialog->deleteLater();
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

std::string SimpleGUIImpl::getOpenFileName(const std::string& caption, const std::string& path, const std::string& filters)
{
	return QFileDialog::getOpenFileName(m_mainWindow, 
		QString::fromStdString(caption), 
		QString::fromStdString(path), 
		QString::fromStdString(filters)
		).toStdString();
}

std::string SimpleGUIImpl::getSaveFileName(const std::string& caption, const std::string& path, const std::string& filters)
{
	return QFileDialog::getSaveFileName(m_mainWindow, 
		QString::fromStdString(caption), 
		QString::fromStdString(path), 
		QString::fromStdString(filters)
		).toStdString();
}

int SimpleGUIImpl::messageBox(MessageBoxType type, const std::string& caption, const std::string& text, int buttons)
{
	switch (type)
	{
	case MessageBoxType::about:
		QMessageBox::about(m_mainView, QString::fromStdString(caption), QString::fromStdString(text));
		return 0;
	case MessageBoxType::critical:
		return QMessageBox::critical(m_mainView, QString::fromStdString(caption), QString::fromStdString(text), static_cast<QMessageBox::StandardButtons>(buttons));
	case MessageBoxType::information:
		return QMessageBox::information(m_mainView, QString::fromStdString(caption), QString::fromStdString(text), static_cast<QMessageBox::StandardButtons>(buttons));
	case MessageBoxType::question:
		return QMessageBox::question(m_mainView, QString::fromStdString(caption), QString::fromStdString(text), static_cast<QMessageBox::StandardButtons>(buttons));
	case MessageBoxType::warning:
		return QMessageBox::warning(m_mainView, QString::fromStdString(caption), QString::fromStdString(text), static_cast<QMessageBox::StandardButtons>(buttons));
	}
}

void SimpleGUIImpl::updateView()
{
	if (m_mainView)
		m_mainView->update();
}

void SimpleGUIImpl::openPropertiesDialog(GraphNode* item)
{
	std::lock_guard<std::mutex> lock(m_propertiesDialogsMutex);
	auto uniqueId = item->uniqueId;
	auto it = std::find_if(m_propertiesDialogs.begin(), m_propertiesDialogs.end(), [uniqueId](const PropertiesDialog* dlg){
		return dlg->graphNode()->uniqueId == uniqueId;
	});
	if (it != m_propertiesDialogs.end()) // Show existing dialog
	{
		auto dlg = *it;
		dlg->activateWindow();
		dlg->raise();
		return;
	}

	// Else create a new one
	auto properties = m_document->objectProperties(item);
	if (properties)
	{
		PropertiesDialog* dlg = new PropertiesDialog(properties, item, m_mainWindow);
		QObject::connect(dlg, &QDialog::finished, [this, dlg](int result) { dialogFinished(dlg, result); });
		m_propertiesDialogs.push_back(dlg);
		dlg->show();
	}
}

void SimpleGUIImpl::closePropertiesDialog(GraphNode* node)
{
	PropertiesDialog* dlg = nullptr;
	{
		std::lock_guard<std::mutex> lock(m_propertiesDialogsMutex);
		auto it = std::find_if(m_propertiesDialogs.begin(), m_propertiesDialogs.end(), [node](const PropertiesDialog* dlg) {
			return dlg->graphNode() == node;
		});

		if (it != m_propertiesDialogs.end())
			dlg = *it;
	}

	if(dlg)
		dlg->reject();
}

void SimpleGUIImpl::closeAllPropertiesDialogs()
{
	std::vector<PropertiesDialog*> dlgs;
	{
		std::lock_guard<std::mutex> lock(m_propertiesDialogsMutex);
		dlgs = m_propertiesDialogs;
		m_propertiesDialogs.clear();
	}

	for (auto dlg : dlgs)
		dlg->reject();
}

std::vector<simplegui::SimpleGUI::ObjectPropertiesPair> SimpleGUIImpl::getOpenedPropertiesDialogs()
{
	std::lock_guard<std::mutex> lock(m_propertiesDialogsMutex);
	std::vector<ObjectPropertiesPair> pairs;
	for (auto dlg : m_propertiesDialogs)
		pairs.emplace_back(dlg->graphNode(), dlg->objectProperties());
	return pairs;
}

void SimpleGUIImpl::dialogFinished(PropertiesDialog* dialog, int result)
{
	m_document->closeObjectProperties(dialog->graphNode(), dialog->objectProperties(), result == QDialog::Accepted);

	std::lock_guard<std::mutex> lock(m_propertiesDialogsMutex);
	auto it = std::find_if(m_propertiesDialogs.begin(), m_propertiesDialogs.end(), [dialog](const PropertiesDialog* dlg){
		return dlg == dialog;
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
