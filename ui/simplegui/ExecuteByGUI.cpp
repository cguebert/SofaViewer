#include <ui/simplegui/ExecuteByGUI.h>

ExecuteByGUI::ExecuteByGUI(QObject* parent)
	: QObject(parent)
{
}

void ExecuteByGUI::addFunction(VoidFunction function)
{
	bool empty = true;
	{
		std::lock_guard<std::mutex> lock(m_functionsMutex);
		empty = m_functions.empty();
		m_functions.push_back(function);
	}

	if (empty) // Ask for the execution on the thread where the object was created
		QMetaObject::invokeMethod(this, "execute", Qt::QueuedConnection);
}

void ExecuteByGUI::execute()
{
	std::vector<VoidFunction> functions;
	{
		std::lock_guard<std::mutex> lock(m_functionsMutex);
		m_functions.swap(functions);
	}

	for (auto& func : functions)
		func();
}