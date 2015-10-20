#pragma once

#include <QMainWindow>

#include <memory>

class QGridLayout;

class BaseDocument;
class GraphView;
class ObjectProperties;
class OpenGLView;
class PropertiesDialog;
class SimpleGUIImpl;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);

	QMenu* menu(unsigned char menuId);
	QGridLayout* buttonsLayout();
	OpenGLView* view() const;

private slots:
	void newDoc();
	void open();
	bool save();
	bool saveAs();
	void about();
	void openRecentFile();
	void showStatusBarMessage(QString);

private:
	void createActions();
	void createMenus();
	void loadModules();

	bool loadFile(const QString &fileName);
	bool saveFile(const QString &fileName);

	void readSettings();
	void writeSettings();
	bool okToContinue();
	void closeEvent(QCloseEvent* event);
	void setCurrentFile(const QString &fileName);
	void updateRecentFileActions();

	void setDocument(std::shared_ptr<BaseDocument> doc);

	OpenGLView* m_openGLView;
	GraphView* m_graphView;
	std::shared_ptr<BaseDocument> m_document;
	std::shared_ptr<SimpleGUIImpl> m_simpleGUI;

	QString strippedName(const QString &fullFileName);

	QString m_curFile;
	QString m_saveFilter;
	QStringList m_recentFiles;

	enum { MaxRecentFiles = 5 };
	QAction* m_recentFileActions[MaxRecentFiles];
	QAction* m_separatorAction;

	QMenu* m_fileMenu;
	QMenu* m_toolsMenu;
	QMenu* m_viewMenu;
	QMenu* m_helpMenu;

	QAction* m_newAction;
	QAction* m_openAction;
	QAction* m_saveAction;
	QAction* m_saveAsAction;
	QAction* m_exitAction;

	QAction* m_aboutAction;
	QAction* m_aboutQtAction;

	QWidget* m_buttonsDockWidget;
};

inline OpenGLView* MainWindow::view() const
{ return m_openGLView; }
