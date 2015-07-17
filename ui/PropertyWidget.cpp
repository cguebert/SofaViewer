#include <ui/PropertyWidget.h>

#include <core/Property.h>

#include <QtWidgets>

BasePropertyWidget::BasePropertyWidget(std::shared_ptr<BasePropertyValue> property, QWidget* parent)
	: QWidget(parent)
	, m_baseProperty(property)
{
}

QWidget* PropertyWidget<int>::createWidgets(bool readOnly)
{
	auto spinBox = new QSpinBox(parentWidget());
	spinBox->setMinimum(INT_MIN);
	spinBox->setMaximum(INT_MAX);
	spinBox->setSingleStep(1);
	spinBox->setEnabled(!readOnly);

	return spinBox;
}
