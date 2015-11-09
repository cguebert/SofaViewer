#pragma once

#include <core/SimpleGUI.h>

#include <memory>

class QAction;
class QPushButton;

class Action_ButtonImpl : public simplegui::Button
{
public:
	using SPtr = std::shared_ptr<Action_ButtonImpl>;
	Action_ButtonImpl(QAction* action);

	std::string title() const override;
	void setTitle(const std::string& title) override;

	std::string help() const override;
	void setHelp(const std::string& help) override;

	bool checkable() const override;
	void setCheckable(bool checkable) override;

	bool checked() const override;
	void setChecked(bool checked) override;

	bool enabled() const override;
	void setEnabled(bool enabled) override;

protected:
	QAction* m_action;
};

/******************************************************************************/

class PushButton_ButtonImpl : public simplegui::Button
{
public:
	using SPtr = std::shared_ptr<PushButton_ButtonImpl>;
	PushButton_ButtonImpl(QPushButton* button);

	std::string title() const override;
	void setTitle(const std::string& title) override;

	std::string help() const override;
	void setHelp(const std::string& help) override;

	bool checkable() const override;
	void setCheckable(bool checkable) override;

	bool checked() const override;
	void setChecked(bool checked) override;

	bool enabled() const override;
	void setEnabled(bool enabled) override;

protected:
	QPushButton* m_button;
};
