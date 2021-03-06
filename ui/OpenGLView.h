#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_0>

class BaseDocument;

class OpenGLView : public QOpenGLWidget, public QOpenGLFunctions_3_0
{
	Q_OBJECT

public:
	OpenGLView(QWidget *parent = nullptr);
	void setDocument(BaseDocument* doc);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;

	int m_width = 0, m_height = 0;
	BaseDocument* m_document = nullptr;
	bool m_OpenGLInitialized = false, m_documentInitialized = false;
};
