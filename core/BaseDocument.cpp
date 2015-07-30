#include <core/BaseDocument.h>

ViewUpdater& ViewUpdater::get()
{
	static ViewUpdater instance;
	return instance;
}

void ViewUpdater::setSignal(UpdateFunc func)
{
	m_func = func;
}

void ViewUpdater::update()
{
	if(m_func)
		m_func();
}
