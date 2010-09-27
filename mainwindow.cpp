#include "mainwindow.h"
#include "qfrdrfcsdata.h"

MainWindow::MainWindow()
{
    settings=NULL;
    project=NULL;
    currentProjectDir="";
    column_separator=",";
    decimal_separator=".";
    comment_start="#";
    header_start="#!";

    createWidgets();
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    setUnifiedTitleAndToolBarOnMac(true);
    logFileMainWidget->log_text(tr("QuickFit started succesfully!\n"));


    newProject();

}

MainWindow::~MainWindow() {
    //std::cout<<"deleting MainWindow\n";
    if (project) delete project;
    //std::cout<<"deleting MainWindow ... OK\n";
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (maybeSave()) {
        writeSettings();
        for (int i=0; i<rawDataPropEditors.size(); i++) {
            if (rawDataPropEditors[i]) {
                //rawDataPropEditors[i]->setCurrent(NULL);
                rawDataPropEditors[i]->writeSettings();
                rawDataPropEditors[i]->close();
                delete rawDataPropEditors[i];
            }
        }
        for (int i=0; i<evaluationPropEditors.size(); i++) {
            if (evaluationPropEditors[i]) {
                //evaluationPropEditors[i]->setCurrent(NULL);
                evaluationPropEditors[i]->writeSettings();
                evaluationPropEditors[i]->close();
                delete evaluationPropEditors[i];
            }
        }
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newProject() {
    if (maybeSave()) {
        setCurrentProject("");
        logFileProjectWidget->clearLog();
        logFileProjectWidget->close_logfile();
        tvMain->setModel(NULL);
        if (project) {
            project->disconnect();
            delete project;
        }
        project=new QFProject(this, logFileProjectWidget, prgMainProgress);
        logFileMainWidget->log_text(tr("created new project!\n"));
        tabLogs->setCurrentWidget(logFileProjectWidget);
        connect(project, SIGNAL(propertiesChanged()), this, SLOT(readProjectProperties()));
        readProjectProperties();
        connect(project, SIGNAL(wasChanged(bool)), this, SLOT(setWindowModified(bool)));
        connect(project->getTreeModel(), SIGNAL(modelReset()), this, SLOT(modelReset()));
        tvMain->setModel(project->getTreeModel());
        connect(project->getTreeModel(), SIGNAL(modelReset()), tvMain, SLOT(expandAll()));
        tvMain->expandAll();
    }
}

void MainWindow::openProject() {
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project ..."), currentProjectDir, tr("QuickFit Project (*.qfp)"));
        if (!fileName.isEmpty()) {
            currentProjectDir=QFileInfo(fileName).dir().absolutePath();
            loadProject(fileName);
        }
    }
}

bool MainWindow::saveProject() {
    if (curFile.isEmpty()) {
        return saveProjectAs();
    } else {
        return saveProject(curFile);
    }
}

bool MainWindow::saveProjectAs() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Project As ..."), currentProjectDir, tr("QuickFit Project (*.qfp)"));
    if (fileName.isEmpty())
        return false;
    currentProjectDir=QFileInfo(fileName).dir().absolutePath();

    return saveProject(fileName);
}


void MainWindow::about() {
   QMessageBox::about(this, tr("About QuickFit 3.0"),
            tr(""));
}

void MainWindow::createWidgets() {
    spCenter=new QSplitter(Qt::Horizontal, this);
    spCenter->setOrientation(Qt::Horizontal);
    spMain=new QSplitter(Qt::Vertical, this);
    spMain->setOrientation(Qt::Vertical);
    tvMain=new QTreeView(this);
    connect(tvMain, SIGNAL(doubleClicked ( const QModelIndex &)), this, SLOT(projectElementDoubleClicked(const QModelIndex&)));
    spCenter->addWidget(tvMain);
    setCentralWidget(spMain);

    QWidget* w=new QWidget(spCenter);
    QFormLayout* fl=new QFormLayout(w);
    w->setLayout(fl);
    spCenter->addWidget(w);
    spMain->addWidget(spCenter);

    edtName=new QLineEdit(w);
    fl->addRow(tr("Project &Name:"), edtName);
    connect(edtName, SIGNAL(textChanged(const QString&)), this, SLOT(projectNameChanged(const QString&)));
    labFile=new QLabel(w);
    fl->addRow(tr("Project File:"), labFile);
    edtCreator=new QLineEdit(w);
    fl->addRow(tr("Project &Creator:"), edtCreator);
    connect(edtCreator, SIGNAL(textChanged(const QString&)), this, SLOT(projectCreatorChanged(const QString&)));
    pteDescription=new QPlainTextEdit(w);
    fl->addRow(tr("Project &Description:"), pteDescription);
    connect(pteDescription, SIGNAL(textChanged()), this, SLOT(projectDescriptionChanged()));

    tabLogs=new QTabWidget(this);
    logFileProjectWidget=new QtLogFile(tabLogs);
    logFileMainWidget=new QtLogFile(tabLogs);
    logFileMainWidget->set_log_file_append(true);
    logFileProjectWidget->set_log_file_append(true);
    tabLogs->addTab(logFileMainWidget, tr("QuickFit Log"));
    QFileInfo fi(QCoreApplication::applicationFilePath());
    logFileMainWidget->open_logfile(QString(fi.absolutePath()+"/"+fi.completeBaseName()+".log"), false);
    logFileMainWidget->log_text(tr("starting up QuickFit 3.0 ...\n"));
    logFileMainWidget->log_text(tr("logging to '%1' ...\n").arg(fi.absolutePath()+"/"+fi.completeBaseName()+".log"));
    tabLogs->addTab(logFileProjectWidget, tr("Project Log"));
    tabLogs->setCurrentWidget(logFileMainWidget);
    spMain->addWidget(tabLogs);

    prgMainProgress=new QProgressBar(this);
    statusBar()->addPermanentWidget(prgMainProgress);
}


void MainWindow::createActions() {
    newProjectAct = new QAction(QIcon(":/project_new.png"), tr("&New Project"), this);
    newProjectAct->setShortcuts(QKeySequence::New);
    newProjectAct->setStatusTip(tr("Create a new project"));
    connect(newProjectAct, SIGNAL(triggered()), this, SLOT(newProject()));


    openProjectAct = new QAction(QIcon(":/project_open.png"), tr("&Open Project..."), this);
    openProjectAct->setShortcuts(QKeySequence::Open);
    openProjectAct->setStatusTip(tr("Open an existing project"));
    connect(openProjectAct, SIGNAL(triggered()), this, SLOT(openProject()));


    saveProjectAct = new QAction(QIcon(":/project_save.png"), tr("&Save Project"), this);
    saveProjectAct->setShortcuts(QKeySequence::Save);
    saveProjectAct->setStatusTip(tr("Save the project to disk"));
    connect(saveProjectAct, SIGNAL(triggered()), this, SLOT(saveProject()));

    saveProjectAsAct = new QAction(QIcon(":/project_saveas.png"), tr("Save Project &As..."), this);
    saveProjectAsAct->setShortcuts(QKeySequence::SaveAs);
    saveProjectAsAct->setStatusTip(tr("Save the project under a new name"));
    connect(saveProjectAsAct, SIGNAL(triggered()), this, SLOT(saveProjectAs()));

    optionsAct = new QAction(QIcon(":/configure.png"), tr("&Settings"), this);
    optionsAct->setStatusTip(tr("Application settings dialog"));
    connect(optionsAct, SIGNAL(triggered()), settings, SLOT(openSettingsDialog()));

    exitAct = new QAction(QIcon(":/exit.png"), tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));


    aboutAct = new QAction(QIcon(":/about.png"), tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(QIcon(":/aboutqt.png"), tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));




    delItemAct = new QAction(QIcon(":/item_delete.png"), tr("&Delete Current Item"), this);
    delItemAct->setStatusTip(tr("delete the currently selected item (if deletable) ..."));
    connect(delItemAct, SIGNAL(triggered()), this, SLOT(deleteItem()));


}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newProjectAct);

    fileMenu->addAction(openProjectAct);
    fileMenu->addAction(saveProjectAct);
    fileMenu->addAction(saveProjectAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(optionsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    dataMenu= menuBar()->addMenu(tr("&Data Items"));
    insertMenu=new QMenu(tr("Insert &Raw Data"), dataMenu);
    insertMenu->setIcon(QIcon(":/rawdata_insert.png"));

    /*QMenu* m=new QMenu(tr("&Table"), insertMenu);
    m->setIcon(QIcon(":/projecttree_tablefile.png"));
    m->addAction(insertRDTableAct);
    m->addAction(insertRDTableFileAct);
    insertMenu->addMenu(m);*/
    //insertMenu->addAction(insertFCSFileAct);

    //qfrdrUserCreateList
    QStringList sl=getRawDataRecordFactory()->getIDList();
    std::cout<<"registering raw data record types: \n";
    for (int i=0; i<sl.size(); i++) {
        std::cout<<"register("<<sl[i].toStdString()<<")\n";
        QList<QPair<QAction*, QFRawDataRecordFactory::QFRawDataRecordUserCreateFunc> > l=getRawDataRecordFactory()->registerMenu(sl[i], this, insertMenu);
        std::cout<<"   "<<l.size()<<" actions\n";
        for (int j=0; j<l.size(); j++) {
            int k=qfrdrUserCreateList.size();
            qfrdrUserCreateList.append(l[j].second);
            QAction* a=l[j].first;
            a->setProperty("call_id", k);
            connect(a, SIGNAL(triggered()), this, SLOT(insertRawData()));
        }
    }

    evaluationMenu= new QMenu(tr("Insert &Evaluation"), dataMenu);
    evaluationMenu->setIcon(QIcon(":/evaluation_insert.png"));
    std::cout<<"registering evaluation types: \n";
    sl=getEvaluationItemFactory()->getIDList();
    for (int i=0; i<sl.size(); i++) {
        std::cout<<"register("<<sl[i].toStdString()<<")\n";
        QList<QPair<QAction*, QFEvaluationItemFactory::QFEvaluationItemUserCreateFunc> > l=getEvaluationItemFactory()->registerMenu(sl[i], this, evaluationMenu);
        std::cout<<"   "<<l.size()<<" actions\n";
        for (int j=0; j<l.size(); j++) {
            int k=qfeiUserCreateList.size();
            qfeiUserCreateList.append(l[j].second);
            QAction* a=l[j].first;
            a->setProperty("call_id", k);
            connect(a, SIGNAL(triggered()), this, SLOT(insertEvaluation()));
        }
    }


    dataMenu->addMenu(insertMenu);
    dataMenu->addMenu(evaluationMenu);
    dataMenu->addAction(delItemAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::insertRawData() {
    QVariant id=sender()->property("call_id");
    if (id.isValid() && project) {
        QFRawDataRecordFactory::QFRawDataRecordUserCreateFunc f=qfrdrUserCreateList[id.toInt()];
        f(project, settings, this, logFileProjectWidget, statusBar(), prgMainProgress);
    }
    tvMain->expandToDepth(2);
    prgMainProgress->setRange(0,1);
    prgMainProgress->setValue(0);
    prgMainProgress->reset();
}

void MainWindow::insertEvaluation() {
    QVariant id=sender()->property("call_id");
    //std::cout<<"insertEvaluation("<<id.toString().toStdString()<<")\n";
    if (id.isValid() && project) {
        QFEvaluationItemFactory::QFEvaluationItemUserCreateFunc f=qfeiUserCreateList[id.toInt()];
        f(project, settings, this, logFileProjectWidget, statusBar(), prgMainProgress);
    }
    tvMain->expandToDepth(2);
    prgMainProgress->setRange(0,1);
    prgMainProgress->setValue(0);
    prgMainProgress->reset();
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newProjectAct);
    fileToolBar->addAction(openProjectAct);
    fileToolBar->addAction(saveProjectAct);
    dataToolBar = addToolBar(tr("Data"));
    QToolButton* tb=new QToolButton(dataToolBar);
    tb->setMenu(insertMenu);
    tb->setIcon(QIcon(":/rawdata_insert.png"));
    tb->setToolTip(tr("insert a new raw data item ..."));
    tb->setPopupMode(QToolButton::InstantPopup);
    dataToolBar->addWidget(tb);
    tb=new QToolButton(dataToolBar);
    tb->setMenu(evaluationMenu);
    tb->setIcon(QIcon(":/evaluation_insert.png"));
    tb->setToolTip(tr("insert a new evaluation ..."));
    tb->setPopupMode(QToolButton::InstantPopup);
    dataToolBar->addWidget(tb);
    dataToolBar->addAction(delItemAct);

}

void MainWindow::createStatusBar() {
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings() {
    if (!settings) return;
    logFileMainWidget->log_text(tr("reading settings from '%1' ...\n").arg(settings->getIniFilename()));
    QPoint pos = settings->getQSettings()->value("mainwindow/pos", QPoint(200, 200)).toPoint();
    QSize size = settings->getQSettings()->value("mainwindow/size", QSize(400, 400)).toSize();

    resize(size.boundedTo(QApplication::desktop()->screenGeometry(this).size()));
    if (pos.x()<-width() || pos.x()>QApplication::desktop()->screenGeometry(this).width()-30) pos.setX(0);
    if (pos.y()<0 || pos.y()>QApplication::desktop()->screenGeometry(this).height()) pos.setY(0);
    move(pos);
    spMain->restoreState(settings->getQSettings()->value("mainwindow/splitterSizesMain").toByteArray());
    spCenter->restoreState(settings->getQSettings()->value("mainwindow/splitterSizesCenter").toByteArray());
    currentProjectDir=settings->getQSettings()->value("mainwindow/currentProjectDir", currentProjectDir).toString();
    currentRawDataDir=settings->getQSettings()->value("mainwindow/currentRawDataDir", currentRawDataDir).toString();
    column_separator=settings->getQSettings()->value("csvimport/column_separator", column_separator).toString();
    decimal_separator=settings->getQSettings()->value("csvimport/decimal_separator", decimal_separator).toString();
    comment_start=settings->getQSettings()->value("csvimport/comment_start", comment_start).toString();
    header_start=settings->getQSettings()->value("csvimport/header_start", header_start).toString();
    currentFCSFileFormatFilter=settings->getQSettings()->value("mainwindow/currentFCSFileFormatFilter", currentFCSFileFormatFilter).toString();

    logFileMainWidget->readSettings(*(settings->getQSettings()), "mainwindow/");
    logFileProjectWidget->readSettings(*(settings->getQSettings()), "mainwindow/");
}

void MainWindow::writeSettings() {
    if (!settings) return;
    logFileMainWidget->log_text(tr("writing settings to '%1' ...\n").arg(settings->getIniFilename()));
    settings->getQSettings()->setValue("mainwindow/pos", pos());
    settings->getQSettings()->setValue("mainwindow/size", size());
    settings->getQSettings()->setValue("mainwindow/splitterSizesMain", spMain->saveState());
    settings->getQSettings()->setValue("mainwindow/splitterSizesCenter", spCenter->saveState());


    logFileMainWidget->saveSettings(*(settings->getQSettings()), "mainwindow/");
    logFileProjectWidget->saveSettings(*(settings->getQSettings()), "mainwindow/");

    settings->getQSettings()->setValue("mainwindow/currentProjectDir", currentProjectDir);
    settings->getQSettings()->setValue("mainwindow/currentRawDataDir", currentRawDataDir);
    settings->getQSettings()->setValue("mainwindow/currentFCSFileFormatFilter", currentFCSFileFormatFilter);
    settings->getQSettings()->setValue("csvimport/column_separator", column_separator);
    settings->getQSettings()->setValue("csvimport/decimal_separator", decimal_separator);
    settings->getQSettings()->setValue("csvimport/comment_start", comment_start);
    settings->getQSettings()->setValue("csvimport/header_start", header_start);
    settings->getQSettings()->sync();



}

bool MainWindow::maybeSave() {
    if (!project) return true;
    if (project->hasChanged()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("QuickFit 3.0"),
                     tr("The project or the data in it has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return saveProject();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::loadProject(const QString &fileName) {
    tabLogs->setCurrentWidget(logFileProjectWidget);
    logFileProjectWidget->close_logfile();
    logFileProjectWidget->clearLog();
    logFileProjectWidget->open_logfile(fileName+".log", false);
    logFileProjectWidget->log_line();
    logFileProjectWidget->log_text(tr("loading project file '%1' ...\n").arg(fileName));
    statusBar()->showMessage(tr("loading project file '%1' ...").arg(fileName), 2000);
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::critical(this, tr("QuickFit 3.0"),
                             tr("Cannot access project file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));

        logFileProjectWidget->log_error(tr("Cannot access project file %1:\n    %2.\n")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (project) {
        for (int i=0; i<rawDataPropEditors.size(); i++) {
            if (rawDataPropEditors[i]) {
                rawDataPropEditors[i]->writeSettings();
                rawDataPropEditors[i]->setCurrent(NULL);
                rawDataPropEditors[i]->close();
            }
        }
        rawDataPropEditors.clear();
        project->disconnect();
        tvMain->setModel(NULL);
        delete project;
    }
    QString fn=fileName;
    logFileProjectWidget->open_logfile(tr("%1.log").arg(fn), false);
    logFileProjectWidget->clearLogStore();
    project=new QFProject(fn, this, logFileProjectWidget, prgMainProgress);
    prgMainProgress->reset();
    prgMainProgress->setRange(0,1);
    prgMainProgress->setValue(0);
    connect(project, SIGNAL(propertiesChanged()), this, SLOT(readProjectProperties()));
    readProjectProperties();
    connect(project, SIGNAL(wasChanged(bool)), this, SLOT(setWindowModified(bool)));
    connect(project, SIGNAL(modelReset()), this, SLOT(modelReset()));
    if (project->error()) {
        QMessageBox::critical(this, tr("QuickFit 3.0"), project->errorDescription());
        logFileProjectWidget->log_error(project->errorDescription()+"\n");
    }
    tvMain->setModel(project->getTreeModel());
    connect(project->getTreeModel(), SIGNAL(modelReset()), tvMain, SLOT(expandAll()));
    tvMain->expandToDepth(2);

    QApplication::restoreOverrideCursor();

    setCurrentProject(fileName);
    statusBar()->showMessage(tr("Project file '%1' loaded!").arg(fileName), 2000);
    logFileProjectWidget->log_text(tr("loading project file '%1' ... DONE!\n").arg(fileName));
    logFileProjectWidget->log_line();
}

bool MainWindow::saveProject(const QString &fileName) {
    logFileMainWidget->log_text(tr("saving project to file '%1' ...\n").arg(fileName));
    tabLogs->setCurrentWidget(logFileProjectWidget);
    statusBar()->showMessage(tr("saving project file '%1' ...").arg(fileName), 2000);
    logFileProjectWidget->log_text(tr("saving project file '%1' ...\n").arg(fileName));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (project) {
        tvMain->setModel(NULL);
        for (int i=0; i<rawDataPropEditors.size(); i++) {
            if (rawDataPropEditors[i]) rawDataPropEditors[i]->writeSettings();
        }
        for (int i=0; i<evaluationPropEditors.size(); i++) {
            if (evaluationPropEditors[i]) evaluationPropEditors[i]->writeSettings();
        }
        logFileProjectWidget->open_logfile(tr("%1.log").arg(fileName), true);
        logFileProjectWidget->clearLogStore();
        project->writeXML(fileName);
        readProjectProperties();
        if (project->error()) {
            QMessageBox::critical(this, tr("QuickFit 3.0"), project->errorDescription());
            logFileProjectWidget->log_error(project->errorDescription()+"\n");
        }
        tvMain->setModel(project->getTreeModel());
        connect(project->getTreeModel(), SIGNAL(modelReset()), tvMain, SLOT(expandAll()));
        tvMain->expandAll();
    }
    QApplication::restoreOverrideCursor();

    setCurrentProject(fileName);
    statusBar()->showMessage(tr("Project file '%1' saved!").arg(fileName), 2000);
    logFileProjectWidget->log_text(tr("Project file '%1' saved!\n").arg(fileName));
    return true;
}

void MainWindow::setCurrentProject(const QString &fileName) {
    curFile = fileName;
    setWindowModified(false);
    tvMain->expandToDepth(2);

    QString shownName;
    if (curFile.isEmpty())
        shownName = tr("untitled.qfp");
    else
        shownName = strippedName(curFile);

    setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("QuickFit 3.0")));
}

QString MainWindow::strippedName(const QString &fullFileName) {
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::projectElementDoubleClicked ( const QModelIndex & index ) {
    if (project) {
        QFProjectTreeModel::nodeType nt=project->getTreeModel()->classifyIndex(tvMain->selectionModel()->currentIndex());
        if (nt==QFProjectTreeModel::qfpntRawDataRecord) {
            QFRawDataRecord* rec=project->getTreeModel()->getRawDataByIndex(index);
            if (rec) {
                QFRawDataPropertyEditor* edt=new QFRawDataPropertyEditor(settings, rec, rawDataPropEditors.size(), this, Qt::Dialog|Qt::WindowMaximizeButtonHint);
                edt->setAttribute(Qt::WA_DeleteOnClose);
                rawDataPropEditors.append(edt);
                edt->show();
            }
        } else if (nt==QFProjectTreeModel::qfpntEvaluationRecord) {
            QFEvaluationItem* rec=project->getTreeModel()->getEvaluationByIndex(tvMain->selectionModel()->currentIndex());
            if (rec) {
                QFEvaluationPropertyEditor* edt=new QFEvaluationPropertyEditor(settings, rec, evaluationPropEditors.size(), this, Qt::Dialog|Qt::WindowMaximizeButtonHint);
                //QFEvaluationPropertyEditor* edt=rec->createPropertyEditor(settings, evaluationPropEditors.size(), this, Qt::Dialog|Qt::WindowMaximizeButtonHint);
                edt->setAttribute(Qt::WA_DeleteOnClose);
                evaluationPropEditors.append(edt);
                edt->show();
            }
        }
    }
}

void MainWindow::projectNameChanged(const QString& text) {
    if (project) {
        project->setName(text);
    }
}

void MainWindow::projectCreatorChanged(const QString& text) {
    if (project) {
        project->setCreator(text);
    }
}

void MainWindow::projectDescriptionChanged() {
    if (project) {
        project->setDescription(pteDescription->toPlainText());
    }
}

void MainWindow::readProjectProperties() {
    if (project) {
        if (edtName->text()!=project->getName()) edtName->setText(project->getName());
        if (edtCreator->text()!=project->getCreator()) edtCreator->setText(project->getCreator());
        if (pteDescription->toPlainText()!=project->getDescription()) pteDescription->setPlainText(project->getDescription());
        labFile->setText(project->getFile());
    }
}



void MainWindow::deleteItem() {
    if (project) {
        QFProjectTreeModel::nodeType nt=project->getTreeModel()->classifyIndex(tvMain->selectionModel()->currentIndex());
        if (nt==QFProjectTreeModel::qfpntRawDataRecord) {
            QFRawDataRecord* rec=project->getTreeModel()->getRawDataByIndex(tvMain->selectionModel()->currentIndex());
            if (rec) {
                project->deleteRawData(rec->getID());
                tvMain->expandToDepth(2);
            }
        } else if (nt==QFProjectTreeModel::qfpntEvaluationRecord) {
            QFEvaluationItem* rec=project->getTreeModel()->getEvaluationByIndex(tvMain->selectionModel()->currentIndex());
            if (rec) {
                project->deleteEvaluation(rec->getID());
                tvMain->expandToDepth(2);
            }
        }
    }
}

void MainWindow::modelReset() {
    tvMain->expandToDepth(2);
}

