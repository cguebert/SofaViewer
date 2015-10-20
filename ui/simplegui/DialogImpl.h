#pragma once

#include <core/SimpleGUI.h>

#include <memory>

class PanelImpl;

class QDialog;
class QGridLayout;
class QWidget;

class DialogImpl : public simplegui::Dialog
{
public:
	DialogImpl(QWidget* parent, const std::string& title);

	simplegui::Panel& content() override;
	bool exec() override;
	void show() override;

	void close();

protected:
	void completeLayout();

	bool m_created = false;
	QDialog* m_dialog;
	QGridLayout* m_panelLayout;
	std::shared_ptr<PanelImpl> m_dialogPanel;
};
