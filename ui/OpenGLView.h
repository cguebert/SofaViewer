#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

class Document;

class OpenGLView : public QOpenGLWidget, public QOpenGLFunctions_3_3_Core
{
	Q_OBJECT

public:
	OpenGLView(QWidget *parent = nullptr);
	void setDocument(Document* doc);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;

	int m_width = 0, m_height = 0;
	Document* m_document = nullptr;
	bool m_OpenGLInitialized = false, m_documentInitialized = false;
};
