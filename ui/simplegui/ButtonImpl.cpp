#include <ui/simplegui/ButtonImpl.h>

#include <QtWidgets>

Action_ButtonImpl::Action_ButtonImpl(QAction* action)
	: m_action(action) { }

std::string Action_ButtonImpl::title() const
{ return m_action->text().toStdString(); }

void Action_ButtonImpl::setTitle(const std::string& title)
{ m_action->setText(QString::fromStdString(title)); }

std::string Action_ButtonImpl::help() const
{ return m_action->statusTip().toStdString(); }

void Action_ButtonImpl::setHelp(const std::string& help)
{ m_action->setStatusTip(QString::fromStdString(help)); }

bool Action_ButtonImpl::checkable() const
{ return m_action->isCheckable(); }

void Action_ButtonImpl::setCheckable(bool checkable)
{ m_action->setCheckable(checkable); }

bool Action_ButtonImpl::checked() const
{ return m_action->isChecked(); }

void Action_ButtonImpl::setChecked(bool checked)
{ m_action->setChecked(checked); }

bool Action_ButtonImpl::enabled() const
{ return m_action->isEnabled(); }

void Action_ButtonImpl::setEnabled(bool enabled)
{ m_action->setEnabled(enabled); }

/******************************************************************************/

PushButton_ButtonImpl::PushButton_ButtonImpl(QPushButton* button)
	: m_button(button) { }

std::string PushButton_ButtonImpl::title() const
{ return m_button->text().toStdString(); }

void PushButton_ButtonImpl::setTitle(const std::string& title)
{ m_button->setText(QString::fromStdString(title)); }

std::string PushButton_ButtonImpl::help() const
{ return m_button->statusTip().toStdString(); }

void PushButton_ButtonImpl::setHelp(const std::string& help)
{ m_button->setStatusTip(QString::fromStdString(help)); }

bool PushButton_ButtonImpl::checkable() const
{ return m_button->isCheckable(); }

void PushButton_ButtonImpl::setCheckable(bool checkable)
{ m_button->setCheckable(checkable); }

bool PushButton_ButtonImpl::checked() const
{ return m_button->isChecked(); }

void PushButton_ButtonImpl::setChecked(bool checked)
{ m_button->setChecked(checked); }

bool PushButton_ButtonImpl::enabled() const
{ return m_button->isEnabled(); }

void PushButton_ButtonImpl::setEnabled(bool enabled)
{ m_button->setEnabled(enabled); }