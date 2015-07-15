#pragma once

#include <QMainWindow>
#include <memory>

class OpenGLView;
class QTreeView;
class Document;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);

private slots:
	void open();
	bool save();
	bool saveAs();
	void about();
	void openRecentFile();
	void showStatusBarMessage(QString);
	void step();

private:
	void createActions();
	void createMenus();

	bool loadFile(const QString &fileName);
	bool saveFile(const QString &fileName);

	void readSettings();
	void writeSettings();
	bool okToContinue();
	void closeEvent(QCloseEvent* event);
	void setCurrentFile(const QString &fileName);
	void updateRecentFileActions();

	OpenGLView* m_view;
	QTreeView* m_graph;
	std::shared_ptr<Document> m_document;

	QString strippedName(const QString &fullFileName);

	QString m_curFile;
	QStringList m_recentFiles;

	enum { MaxRecentFiles = 5 };
	QAction* m_recentFileActions[MaxRecentFiles];
	QAction* m_separatorAction;

	QMenu* m_fileMenu;
	QMenu* m_toolsMenu;
	QMenu* m_viewMenu;
	QMenu* m_helpMenu;

	QAction* m_openAction;
	QAction* m_saveAction;
	QAction* m_saveAsAction;
	QAction* m_exitAction;

	QAction* m_aboutAction;
	QAction* m_aboutQtAction;
};

