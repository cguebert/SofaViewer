#include <ui/OpenGLView.h>

#include <core/BaseDocument.h>
#include <core/MouseEvent.h>

#include <QMouseEvent>

OpenGLView::OpenGLView(QWidget *parent)
	: QOpenGLWidget(parent)
{
	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(3, 3);
	format.setProfile(QSurfaceFormat::CoreProfile);
	setFormat(format);
}

void OpenGLView::setDocument(BaseDocument* doc)
{
	m_document = doc;

	if(m_OpenGLInitialized)
	{
		makeCurrent();
		m_document->initOpenGL();
		if(m_width && m_height)
			m_document->resize(m_width, m_height);
		m_documentInitialized = true;
		update();
	}
	else
		m_documentInitialized = false;
}

void OpenGLView::initializeGL()
{
	initializeOpenGLFunctions();
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	m_OpenGLInitialized = true;
}

void OpenGLView::resizeGL(int w, int h)
{
	m_width = w; m_height = h;
	if(m_document)
		m_document->resize(w, h);
}

void OpenGLView::paintGL()
{
	if(!m_documentInitialized && m_document)
	{
		m_document->initOpenGL();
		m_documentInitialized = true;
	}

	if(m_document)
		m_document->render();
	else
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

MouseEvent convert(QMouseEvent* event, MouseEvent::EventType type, int w, int h)
{
	MouseEvent tmp;

	tmp.type = type;

	tmp.x = event->x(); tmp.y = event->y();

	tmp.button = event->button();
	auto buttons = event->buttons();
	if(buttons & Qt::LeftButton)	tmp.buttons |= MouseEvent::LeftButton;
	if(buttons & Qt::RightButton)	tmp.buttons |= MouseEvent::RightButton;
	if(buttons & Qt::MiddleButton)	tmp.buttons |= MouseEvent::MiddleButton;

	auto modifiers = event->modifiers();
	if(modifiers & Qt::ShiftModifier)	tmp.modifiers |= MouseEvent::ShiftModifier;
	if(modifiers & Qt::AltModifier)		tmp.modifiers |= MouseEvent::AltModifier;
	if(modifiers & Qt::ControlModifier)	tmp.modifiers |= MouseEvent::ControlModifier;

	tmp.width = w; tmp.height = h;

	return tmp;
}

void OpenGLView::mousePressEvent(QMouseEvent* event)
{
	if(m_document && m_document->mouseEvent(
		convert(event, MouseEvent::EventType::MousePress, m_width, m_height)))
			update();
}

void OpenGLView::mouseDoubleClickEvent(QMouseEvent* event)
{
	if(m_document && m_document->mouseEvent(
		convert(event, MouseEvent::EventType::MouseDoubleClick, m_width, m_height)))
			update();
}

void OpenGLView::mouseReleaseEvent(QMouseEvent* event)
{
	if(m_document && m_document->mouseEvent(
		convert(event, MouseEvent::EventType::MouseRelease, m_width, m_height)))
			update();
}

void OpenGLView::mouseMoveEvent(QMouseEvent* event)
{
	if(m_document && m_document->mouseEvent(
		convert(event, MouseEvent::EventType::MouseMove, m_width, m_height)))
			update();
}
