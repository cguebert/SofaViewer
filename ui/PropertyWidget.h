#pragma once

#include <QWidget>

#include <memory>

class BasePropertyValue;
template <class T> class PropertyValue;

class BasePropertyWidget : public QWidget
{
	Q_OBJECT

public:
	BasePropertyWidget(std::shared_ptr<BasePropertyValue> property, QWidget *parent = nullptr);

	virtual QWidget* createWidgets(bool readOnly = true) = 0;

protected:
	std::shared_ptr<BasePropertyValue> m_baseProperty;
};

template <class T>
class PropertyWidget : public BasePropertyWidget
{
public:
	PropertyWidget(std::shared_ptr<PropertyValue<T>> property, QWidget *parent = nullptr)
		: BasePropertyWidget(property, parent)
		, m_property(property)
	{
	}

	QWidget* createWidgets(bool readOnly) override;

protected:
	std::shared_ptr<PropertyValue<T>> m_property;
};
