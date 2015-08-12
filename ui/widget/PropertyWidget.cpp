#include <ui/widget/PropertyWidget.h>

void BasePropertyWidget::resolveConflict(Source source)
{
	if(source == Source::property)
	{
		readFromProperty();
		update();
	}
	else if(source == Source::widget)
		writeToProperty();

	setState(State::unchanged);
}

void BasePropertyWidget::setSaveTrigger(SaveTrigger trigger)
{
	if(trigger == SaveTrigger::asap && m_state == State::modified)
		writeToProperty();

	m_saveTrigger = trigger;
}

void BasePropertyWidget::updatePropertyValue()
{
	if(m_state == State::modified)
	{
		writeToProperty();
		setState(State::unchanged);
	}
}

void BasePropertyWidget::updateWidgetValue()
{
	if(m_state == State::unchanged)
	{
		readFromProperty();
		update();
	}
	else if(m_state == State::modified)
		setState(State::conflict);
}

void BasePropertyWidget::setWidgetDirty()
{
	if(isModified())
	{
		setState(State::modified);
		if(m_saveTrigger == SaveTrigger::asap)
			updatePropertyValue();
	}
	else if(m_state == State::modified)
		setState(State::unchanged);
}

void BasePropertyWidget::setState(State state)
{
	if(m_state != state)
	{
		m_state = state;
		auto stateVal = static_cast<int>(state);
		emit stateChanged(this, stateVal);
	}
}
