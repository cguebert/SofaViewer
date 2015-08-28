#pragma once

#include <core/core.h>
#include <core/MouseEvent.h>

class Scene;

class CORE_API MouseManipulator
{
public:
	MouseManipulator(Scene& scene);
	virtual bool mouseEvent(const MouseEvent& /*event*/) = 0; // Return true if an update is necessary

protected:
	Scene& m_scene;
};

class CORE_API SofaMouseManipulator : public MouseManipulator
{
public:
	SofaMouseManipulator(Scene& scene);
	bool mouseEvent(const MouseEvent& event) override;

protected:
	enum class MouseManipulation
	{ None, Rotation, Translation, Zoom, };

	int m_prevX = 0, m_prevY = 0;
	MouseManipulation m_mouseManipulation = MouseManipulation::None;
	unsigned char m_buttonPressed = 0; // Button responsible for the start of the manipulation
};
