#pragma once

#include <core/SimpleGUI.h>

class QGridLayout;
class MainWindow;

class ButtonsPanel : public ui::Panel
{
public:
	ButtonsPanel(MainWindow* mainWindow, QGridLayout* layout);

	void addButton(const std::string& name, const std::string& help,
				   ui::CallbackFunc callback,
				   int row, int column,
				   int rowSpan, int columnSpan) override;

protected:
	MainWindow* m_mainWindow;
	QGridLayout* m_layout;
};

/******************************************************************************/

class SimpleGUIImpl : public ui::SimpleGUI
{
public:
	SimpleGUIImpl(MainWindow* mainWindow, QGridLayout* buttonsLayout);

	virtual void addMenuItem(Menu menu, const std::string& name, const std::string& help, ui::CallbackFunc callback) override;
	virtual ui::Panel& buttonsPanel() override;

protected:
	MainWindow* m_mainWindow;
	ButtonsPanel m_buttonsPanel;
};
