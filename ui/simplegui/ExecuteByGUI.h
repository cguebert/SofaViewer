#pragma once

#include <QObject>

#include <functional>
#include <mutex>
#include <vector>

class ExecuteByGUI : public QObject
{
	Q_OBJECT
public:
	ExecuteByGUI(QObject* parent = nullptr);
	using VoidFunction = std::function<void()>;

	void addFunction(VoidFunction function);

protected slots:
	void execute();
	
private:
	std::vector<VoidFunction> m_functions;
	std::mutex m_functionsMutex;
};
