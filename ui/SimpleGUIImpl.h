#pragma once

#include <core/SimpleGUI.h>

class QGridLayout;
class MainWindow;
class QLabel;

class ButtonsPanel : public ui::Panel
{
public:
	ButtonsPanel(MainWindow* mainWindow);

	void addButton(const std::string& name, const std::string& help,
				   ui::CallbackFunc callback,
				   int row, int column,
				   int rowSpan, int columnSpan) override;

protected:
	MainWindow* m_mainWindow;
};

/******************************************************************************/

class SimpleGUIImpl : public ui::SimpleGUI
{
public:
	SimpleGUIImpl(MainWindow* mainWindow);

	virtual void addMenuItem(Menu menu, const std::string& name, const std::string& help, ui::CallbackFunc callback) override;
	virtual ui::Panel& buttonsPanel() override;
	virtual int addStatusBarZone(const std::string& text) override;
	virtual void setStatusBarText(int id, const std::string& text) override;

	void clear();

protected:
	MainWindow* m_mainWindow;
	ButtonsPanel m_buttonsPanel;
	std::vector<QLabel*> m_statusBarLabels;
	std::vector<QAction*> m_menuActions;
};
