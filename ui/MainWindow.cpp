#include <ui/GraphModel.h>
#include <ui/MainWindow.h>
#include <ui/OpenGLView.h>
#include <core/Document.h>

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	m_document = std::make_shared<Document>();

	// Buttons
	auto buttonsDock = new QDockWidget(tr("Buttons"));
	auto buttonsWidget = new QWidget(this);
	auto buttonsLayout = new QHBoxLayout;
	auto stepButton = new QPushButton(tr("Step"));
	connect(stepButton, SIGNAL(clicked(bool)), this, SLOT(step()));
	buttonsLayout->addWidget(stepButton);
	buttonsWidget->setLayout(buttonsLayout);
	buttonsDock->setObjectName("ButtonsDock");
	buttonsDock->setWidget(buttonsWidget);
	buttonsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, buttonsDock);

	// Graph tree
	m_graph = new QTreeView(this);
	m_graph->setUniformRowHeights(true);
//	m_graph->setRootIsDecorated(true);
	m_graph->header()->hide();
	auto graphDock = new QDockWidget(tr("Graph"), this);
	graphDock->setObjectName("GraphDock");
	graphDock->setWidget(m_graph);
	graphDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, graphDock);

	m_view = new OpenGLView(this);
	setCentralWidget(m_view);

	createActions();
	createMenus();

	statusBar();

	setWindowIcon(QIcon(":/share/icons/icon.png"));
	setCurrentFile("");

	readSettings();
}

void MainWindow::createActions()
{
	m_openAction = new QAction(tr("&Open..."), this);
	m_openAction->setIcon(QIcon(":/share/icons/open.png"));
	m_openAction->setShortcut(QKeySequence::Open);
	m_openAction->setStatusTip(tr("Open an existing Sofa scene"));
	connect(m_openAction, SIGNAL(triggered()), this, SLOT(open()));

	m_saveAction = new QAction(tr("&Save"), this);
	m_saveAction->setIcon(QIcon(":/share/icons/save.png"));
	m_saveAction->setShortcut(QKeySequence::Save);
	m_saveAction->setStatusTip(tr("Save the scene to disk"));
	connect(m_saveAction, SIGNAL(triggered()), this, SLOT(save()));

	m_saveAsAction = new QAction(tr("Save &As..."), this);
	m_saveAsAction->setStatusTip(tr("Save the scene under a new name"));
	connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

	for (int i = 0; i < MaxRecentFiles; ++i) {
		m_recentFileActions[i] = new QAction(this);
		m_recentFileActions[i]->setVisible(false);
		connect(m_recentFileActions[i], SIGNAL(triggered()),
				this, SLOT(openRecentFile()));
	}

	m_exitAction = new QAction(tr("E&xit"), this);
	m_exitAction->setShortcut(tr("Ctrl+Q"));
	m_exitAction->setStatusTip(tr("Exit Panda"));
	connect(m_exitAction, SIGNAL(triggered()), this, SLOT(close()));

	m_aboutAction = new QAction(tr("&About"), this);
	m_aboutAction->setStatusTip(tr("Show the application's About box"));
	connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	m_aboutQtAction = new QAction(tr("About &Qt"), this);
	m_aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
	connect(m_aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
	m_fileMenu = menuBar()->addMenu(tr("&File"));
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
	QSettings settings("Christophe Guebert", "SofaFrontEndViewer");

	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());

	m_recentFiles = settings.value("recentFiles").toStringList();
	updateRecentFileActions();
}

void MainWindow::writeSettings()
{
	QSettings settings("Christophe Guebert", "SofaFrontEndViewer");

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

bool MainWindow::save()
{
	if (m_curFile.isEmpty())
		return saveAs();
	else
		return saveFile(m_curFile);
}

bool MainWindow::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this,
							   tr("Save Document"), ".",
							   tr("Panda files (*.pnd);;XML Files (*.xml)"));
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
	if (!m_document->loadFile(fileName))
	{
		statusBar()->showMessage(tr("Loading failed"), 2000);

		auto oldModel = m_graph->model();
		m_graph->setModel(nullptr);
		if(oldModel)
			delete oldModel;
		return false;
	}

	m_view->setDocument(m_document.get());

	auto oldModel = m_graph->model();
	m_graph->setModel(new GraphModel(this, m_document->graph()));
	if(oldModel)
		delete oldModel;

	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File loaded"), 2000);
	return true;
}

bool MainWindow::saveFile(const QString& fileName)
{
//	if (!m_document->writeFile(fileName))
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
		QString fileName = QFileDialog::getOpenFileName(this,
								   tr("Open Document"), ".",
								   tr("Sofa scene (*.scn);;Obj Mesh (*.obj)"));
		if (!fileName.isEmpty())
		{
			loadFile(fileName);
		}
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

void MainWindow::showStatusBarMessage(QString text)
{
	statusBar()->showMessage(text, 2000);
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About SFE Viewer"),
			tr("<h2>Sofa Front End Viewer</h2>"
			   "<p>Copyright &copy; 2015 Christophe Guébert"
			   "<p>Using Sofa and Sofa Front End"));
}

void MainWindow::step()
{
	if(m_document)
		m_document->step();
}
