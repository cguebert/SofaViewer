#include <ui/GraphView.h>
#include <ui/MainWindow.h>
#include <ui/OpenGLView.h>
#include <ui/simplegui/SimpleGUIImpl.h>

#include <core/DocumentFactory.h>

#include <QtWidgets>

#include <iostream>

class ChangeDir
{
public:
	ChangeDir(const QString& path)
	{
		prevDir = QDir::current();
		QDir::setCurrent(QFileInfo(path).absolutePath());
	}

	~ChangeDir() { QDir::setCurrent(prevDir.absolutePath()); }

protected:
	QDir prevDir;
};

/******************************************************************************/

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	// Buttons
	auto buttonsDock = new QDockWidget(tr("Buttons"));
	m_buttonsDockWidget = new QWidget(this);
	auto buttonsLayout = new QGridLayout;
	m_buttonsDockWidget->setLayout(buttonsLayout);
	buttonsDock->setObjectName("ButtonsDock");
	buttonsDock->setWidget(m_buttonsDockWidget);
	buttonsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, buttonsDock);

	// Graph tree
	m_graphView = new GraphView(this);
	auto graphDock = new QDockWidget(tr("Graph"), this);
	graphDock->setObjectName("GraphDock");
	graphDock->setWidget(m_graphView->view());
	graphDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, graphDock);

	m_openGLView = new OpenGLView(this);
	setCentralWidget(m_openGLView);

	createActions();
	createMenus();

	statusBar();
	
	std::vector<QMenu*> menus = { m_fileMenu, m_toolsMenu, m_viewMenu, m_helpMenu };
	m_simpleGUI = std::make_shared<SimpleGUIImpl>(this, m_openGLView, m_buttonsDockWidget, menus);

	connect(m_graphView, &GraphView::itemOpened, [this](void* item) {
		m_simpleGUI->openPropertiesDialog(static_cast<GraphNode*>(item));
	});

	setWindowIcon(QIcon(":/share/icons/icon.png"));
	setCurrentFile("");
	setAcceptDrops(true);

	readSettings();

	loadModules();

	auto& factory = DocumentFactory::instance();
	if (factory.creatableDocuments().empty())
		m_newAction->setEnabled(false);
	if (factory.loadFilesFilter().empty())
		m_openAction->setEnabled(false);
}

void MainWindow::createActions()
{
	m_newAction = new QAction(tr("&New..."), this);
	m_newAction->setIcon(QIcon(":/share/icons/new.png"));
	m_newAction->setShortcut(QKeySequence::New);
	m_newAction->setStatusTip(tr("Create a new document"));
	connect(m_newAction, &QAction::triggered, this, &MainWindow::newDoc);

	m_openAction = new QAction(tr("&Open..."), this);
	m_openAction->setIcon(QIcon(":/share/icons/open.png"));
	m_openAction->setShortcut(QKeySequence::Open);
	m_openAction->setStatusTip(tr("Open a file"));
	connect(m_openAction, &QAction::triggered, this, &MainWindow::open);

	m_saveAction = new QAction(tr("&Save"), this);
	m_saveAction->setIcon(QIcon(":/share/icons/save.png"));
	m_saveAction->setShortcut(QKeySequence::Save);
	m_saveAction->setStatusTip(tr("Save the scene to disk"));
	m_saveAction->setEnabled(false);
	connect(m_saveAction, &QAction::triggered, this, &MainWindow::save);

	m_saveAsAction = new QAction(tr("Save &As..."), this);
	m_saveAsAction->setStatusTip(tr("Save the scene under a new name"));
	m_saveAsAction->setEnabled(false);
	connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);

	for (int i = 0; i < MaxRecentFiles; ++i) {
		m_recentFileActions[i] = new QAction(this);
		m_recentFileActions[i]->setVisible(false);
		connect(m_recentFileActions[i], &QAction::triggered, this, &MainWindow::openRecentFile);
	}

	m_exitAction = new QAction(tr("E&xit"), this);
	m_exitAction->setShortcut(tr("Ctrl+Q"));
	m_exitAction->setStatusTip(tr("Exit Panda"));
	connect(m_exitAction, &QAction::triggered, this, &MainWindow::close);

	m_aboutAction = new QAction(tr("&About"), this);
	m_aboutAction->setStatusTip(tr("Show the application's About box"));
	connect(m_aboutAction, &QAction::triggered, this, &MainWindow::about);

	m_aboutQtAction = new QAction(tr("About &Qt"), this);
	m_aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
	connect(m_aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::createMenus()
{
	m_fileMenu = menuBar()->addMenu(tr("&File"));
	m_fileMenu->addAction(m_newAction);
	m_fileMenu->addAction(m_openAction);
	m_fileMenu->addAction(m_saveAction);
	m_fileMenu->addAction(m_saveAsAction);
	m_separatorAction = m_fileMenu->addSeparator();
	for (int i = 0; i < MaxRecentFiles; ++i)
		m_fileMenu->addAction(m_recentFileActions[i]);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_exitAction);

	m_toolsMenu = menuBar()->addMenu(tr("&Tools"));

	m_viewMenu = menuBar()->addMenu(tr("&View"));

	m_helpMenu = menuBar()->addMenu(tr("&Help"));
	m_helpMenu->addSeparator();
	m_helpMenu->addAction(m_aboutAction);
	m_helpMenu->addAction(m_aboutQtAction);
}

void MainWindow::dropEvent(QDropEvent* event)
{
	auto path = event->mimeData()->urls().first().toLocalFile();
	event->acceptProposedAction();
	if (!path.isEmpty() && okToContinue())
		loadFile(path);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasUrls())
		event->acceptProposedAction();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (okToContinue())
	{
		writeSettings();
		event->accept();
	}
	else
		event->ignore();
}

void MainWindow::readSettings()
{
	QSettings settings;
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());

	m_recentFiles = settings.value("recentFiles").toStringList();
	updateRecentFileActions();
}

void MainWindow::writeSettings()
{
	QSettings settings;
	settings.setValue("recentFiles", m_recentFiles);
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
}

bool MainWindow::okToContinue()
{
	if (isWindowModified())
	{
		int r = QMessageBox::warning(this, tr("Panda"),
						tr("The document has been modified.\n"
						   "Do you want to save your changes?"),
						QMessageBox::Yes | QMessageBox::No
						| QMessageBox::Cancel);
		if (r == QMessageBox::Yes)
			return save();
		else if (r == QMessageBox::Cancel)
			return false;
	}
	return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
	m_curFile = fileName;
	setWindowModified(false);

	QString shownName = tr("Untitled");
	if (!m_curFile.isEmpty())
	{
		shownName = strippedName(m_curFile);
		m_recentFiles.removeAll(m_curFile);
		m_recentFiles.prepend(m_curFile);
		updateRecentFileActions();
	}

	setWindowTitle(tr("%1[*] - %2").arg(shownName)
								   .arg(tr("Sofa Front End Viewer")));
}

void MainWindow::updateRecentFileActions()
{
	QMutableStringListIterator i(m_recentFiles);
	while (i.hasNext()) {
		if (!QFile::exists(i.next()))
			i.remove();
	}

	for (int j = 0; j < MaxRecentFiles; ++j)
	{
		if (j < m_recentFiles.count())
		{
			QString text = tr("&%1 %2")
						   .arg(j + 1)
						   .arg(strippedName(m_recentFiles[j]));
			m_recentFileActions[j]->setText(text);
			m_recentFileActions[j]->setData(m_recentFiles[j]);
			m_recentFileActions[j]->setVisible(true);
		}
		else
			m_recentFileActions[j]->setVisible(false);
	}
	m_separatorAction->setVisible(!m_recentFiles.isEmpty());
}

void MainWindow::newDoc()
{
	const auto creatable = DocumentFactory::instance().creatableDocuments();
	QStringList items;
	for (const auto& c : creatable)
		items.push_back(QString::fromStdString(c));

	bool ok = false;
	QString item = QInputDialog::getItem(this, tr("New document"), tr("Document type:"), items, 0, false, &ok);
	if (!ok)
		return;

	auto document = DocumentFactory::instance().create(item.toStdString());
	if (!document)
	{
		statusBar()->showMessage(tr("Cannot create the document").arg(item), 2000);
		return;
	}

	setDocument(document);

	setCurrentFile("");
	statusBar()->showMessage(tr("New document created"), 2000);
}

bool MainWindow::save()
{
	if (m_curFile.isEmpty())
		return saveAs();
	else
		return saveFile(m_curFile);
}

bool MainWindow::saveAs()
{
	auto filters = DocumentFactory::instance().saveFilesFilter(m_document.get());
	QString fileName = QFileDialog::getSaveFileName(this,
							   tr("Save Document"), ".",
							   filters.c_str());
	if (fileName.isEmpty())
		return false;

	return saveFile(fileName);
}

QString MainWindow::strippedName(const QString& fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

bool MainWindow::loadFile(const QString& fileName)
{
	std::string cpath = fileName.toStdString();
	auto document = DocumentFactory::instance().createForFile(cpath);
	if(!document) // TODO: show a dialog so that the user can choose the correct document to use
	{
		statusBar()->showMessage(tr("Cannot create a document to load this file"), 2000);
		return false;
	}

	ChangeDir cd(fileName);
	setDocument(document);

	if (!document->loadFile(cpath))
	{
		setDocument(nullptr);
		statusBar()->showMessage(tr("Loading failed"), 2000);
		return false;
	}

	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File loaded"), 2000);
	return true;
}

bool MainWindow::saveFile(const QString& fileName)
{
	if (!m_document->saveFile(fileName.toStdString()))
	{
		statusBar()->showMessage(tr("Saving failed"), 2000);
		return false;
	}

	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File saved"), 2000);
	return true;
}

void MainWindow::open()
{
	if (okToContinue()) {
		auto filters = DocumentFactory::instance().loadFilesFilter();
		QString fileName = QFileDialog::getOpenFileName(this,
								   tr("Open Document"), ".",
								   filters.c_str());
		if (!fileName.isEmpty())
			loadFile(fileName);
	}
}

void MainWindow::openRecentFile()
{
	if (okToContinue())
	{
		QAction *action = qobject_cast<QAction *>(sender());
		if(action)
		{
			loadFile(action->data().toString());
		}
	}
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About SFE Viewer"),
			tr("<h2>Sofa Front End Viewer</h2>"
			   "<p>Copyright &copy; 2015 Christophe Guébert"
			   "<p>Using Sofa and Sofa Front End"));
}

void MainWindow::setDocument(std::shared_ptr<BaseDocument> document)
{
	m_simpleGUI->clear();

	m_simpleGUI->setDocument(document);
	m_graphView->setDocument(document);
	m_openGLView->setDocument(document.get());

	m_document = document;

	if (m_document)
	{
		m_saveFilter = DocumentFactory::instance().saveFilesFilter(m_document.get()).c_str();
		bool canSave = !m_saveFilter.isEmpty();
		m_saveAction->setEnabled(canSave);
		m_saveAsAction->setEnabled(canSave);

		m_document->initUI(*m_simpleGUI.get());
	}
}

void MainWindow::loadModules()
{
	const QString appDirPath = QApplication::applicationDirPath();
	const QDir dir(appDirPath + "/modules");
	const QDir sharedDir(appDirPath + "/shared");
	const auto modules = dir.entryList(QDir::Dirs, QDir::Name);

	auto& factory = DocumentFactory::instance(); // Just to make sure it is created here and now

#ifdef WIN32
	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	QString sharedDirPath = sharedDir.absolutePath();
	auto sharedDllDir = AddDllDirectory(sharedDirPath.toStdWString().c_str());
#endif

	for (const auto& module : modules)
	{
		if (module.startsWith("."))
			continue;

		QDir moduleDir = dir;
		moduleDir.cd(module);
		QString dirPath = moduleDir.absolutePath();
		factory.setModuleDirPath(dirPath.toStdString());

#ifdef WIN32
		auto moduleDllDir = AddDllDirectory(dirPath.toStdWString().c_str());
#endif

		QFileInfo file(moduleDir, module);
		QString path = file.absoluteFilePath();
		QLibrary lib(path);
		lib.load();

#ifdef WIN32
		RemoveDllDirectory(moduleDllDir);
#endif
	}

#ifdef WIN32
	RemoveDllDirectory(sharedDllDir);
#endif
}
