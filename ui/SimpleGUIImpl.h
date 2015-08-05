#pragma once

#include <core/SimpleGUI.h>

class BasePropertyWidget;
class MainWindow;
class QDialog;
class QGridLayout;
class QLabel;
class QLayout;

class PanelImpl : public ui::Panel
{
public:
	PanelImpl(MainWindow* mainWindow, QGridLayout* layout);

	void addButton(const std::string& name, const std::string& help,
				   ui::CallbackFunc callback,
				   int row, int column,
				   int rowSpan, int columnSpan) override;

	void addProperty(Property::PropertyPtr property,
								 int row, int column,
								 int rowSpan, int columnSpan) override;

	using PropertyWidgets = std::vector<std::shared_ptr<BasePropertyWidget>>;
	const PropertyWidgets& propertyWidgets() { return m_propertyWidgets; }

protected:
	MainWindow* m_mainWindow;
	QGridLayout* m_layout;
	PropertyWidgets m_propertyWidgets;
};

/******************************************************************************/

class DialogImpl : public ui::Dialog
{
public:
	DialogImpl(MainWindow* mainWindow, const std::string& title);

	ui::Panel& content() override;
	bool exec() override;
	void show() override;

	void close();

protected:
	void completeLayout();

	QDialog* m_dialog;
	QGridLayout* m_panelLayout;
	std::shared_ptr<PanelImpl> m_dialogPanel;
};

/******************************************************************************/

class SimpleGUIImpl : public ui::SimpleGUI
{
public:
	SimpleGUIImpl(MainWindow* mainWindow);

	void addMenuItem(Menu menu, const std::string& name, const std::string& help, ui::CallbackFunc callback) override;
	ui::Panel& buttonsPanel() override;
	int addStatusBarZone(const std::string& text) override;
	void setStatusBarText(int id, const std::string& text) override;
	DialogPtr createDialog(const std::string& title) override;

	void clear();

protected:
	MainWindow* m_mainWindow;
	PanelImpl m_buttonsPanel;
	std::vector<QLabel*> m_statusBarLabels;
	std::vector<QAction*> m_menuActions;
	using DialogImplPtr = std::shared_ptr<DialogImpl>;
	std::vector<DialogImplPtr> m_dialogs;
};
