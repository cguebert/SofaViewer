#include "MouseManipulator.h"
#include "Scene.h"

MouseManipulator::MouseManipulator(Scene& scene)
	: m_scene(scene)
{
}

/******************************************************************************/

SofaMouseManipulator::SofaMouseManipulator(Scene& scene)
	: MouseManipulator(scene)
{

}

bool SofaMouseManipulator::mouseEvent(const MouseEvent& event)
{
	switch(event.type)
	{
	case MouseEvent::EventType::MousePress:
		if(!m_buttonPressed)
		{
			switch(event.button)
			{
			case MouseEvent::LeftButton:	m_mouseManipulation = MouseManipulation::Rotation;		break;
			case MouseEvent::RightButton:	m_mouseManipulation = MouseManipulation::Zoom;			break;
			case MouseEvent::MiddleButton:	m_mouseManipulation = MouseManipulation::Translation;	break;
			}

			if(m_mouseManipulation != MouseManipulation::None)
			{
				m_buttonPressed = event.button;
				m_prevX = event.x; m_prevY = event.y;
			}
		}
		break;

	case MouseEvent::EventType::MouseRelease:
		if(m_buttonPressed == event.button)
		{
			m_buttonPressed = 0;
			m_mouseManipulation = MouseManipulation::None;
		}
		break;

	case MouseEvent::EventType::MouseMove:
	{
		int dx = event.x - m_prevX, dy = event.y - m_prevY;
		switch(m_mouseManipulation)
		{
		case MouseManipulation::Rotation:
		{
			glm::quat& rot = m_scene.rotation();
			glm::quat invRot = glm::inverse(rot);
			rot = glm::rotate(rot, glm::radians(dx * 180.0f / event.width), invRot * glm::vec3(0, 1, 0));
			rot = glm::rotate(rot, glm::radians(dy * 180.0f / event.height), invRot * glm::vec3(1, 0, 0));
			break;
		}

		case MouseManipulation::Translation:
		{
			auto size = m_scene.size();
			float amp = 2 * std::min({size[0], size[1], size[2]}) / std::max(event.width, event.height);
			m_scene.translation() += glm::vec3(dx * amp, -dy * amp, 0);
			break;
		}

		case MouseManipulation::Zoom:
		{
			auto size = m_scene.size();
			auto vMax = std::max({size[0], size[1], size[2]});
			m_scene.translation()[2] += dy * vMax / 500;
			break;
		}
		}

		m_prevX = event.x; m_prevY = event.y;

		break;
	}

	default:
		return false;
	}

	return true;
}
