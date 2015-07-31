#pragma once

#include <QMainWindow>

#include <functional>
#include <memory>
#include <vector>

class OpenGLView;
class QTreeView;
class BaseDocument;
class SimpleGUIImpl;
class QGridLayout;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);

	using CallbackFunc = std::function<void()>;
	int addCallback(CallbackFunc func);

	QMenu* menu(unsigned char menuId);
	QGridLayout* buttonsLayout();

private slots:
	void open();
	bool save();
	bool saveAs();
	void about();
	void openRecentFile();
	void showStatusBarMessage(QString);
	void graphItemDoubleClicked(const QModelIndex&);
	void executeCallback();

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
	std::shared_ptr<BaseDocument> m_document;
	std::shared_ptr<SimpleGUIImpl> m_simpleGUI;

	std::vector<CallbackFunc> m_callbacks;

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

	QWidget* m_buttonsDockWidget;
};

