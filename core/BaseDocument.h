#pragma once

#include <core/Graph.h>
#include <core/MouseManipulator.h>

#include <functional>

class ViewUpdater
{
public:
	static ViewUpdater& get();

	using UpdateFunc = std::function<void ()>;

	void setSignal(UpdateFunc func);
	void update();

protected:
	UpdateFunc m_func;
};

/******************************************************************************/

class ObjectProperties;

namespace ui { class SimpleGUI; }

class BaseDocument
{
public:
	BaseDocument(ui::SimpleGUI& /*gui*/) {}
	virtual ~BaseDocument() {}

	virtual bool loadFile(const std::string& /*path*/) { return false; }

	virtual void initOpenGL() {}
	virtual void resize(int /*width*/, int /*height*/) {}
	virtual void render() {}

	virtual bool mouseEvent(const MouseEvent& /*event*/) { return false; } // Return true if an view update is necessary

	virtual Graph& graph() = 0;

	using ObjectPropertiesPtr = std::shared_ptr<ObjectProperties>;
	virtual ObjectPropertiesPtr objectProperties(Graph::Node* /*item*/) const { return nullptr; }
};
