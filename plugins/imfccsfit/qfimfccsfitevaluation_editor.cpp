#include "qfimfccsfitevaluation_editor.h"
#include "qfrawdatarecord.h"
#include "qfevaluationitem.h"
#include "qfimfccsfitevaluation_item.h"
#include "ui_qfimfccsfitevaluation_editor.h"
#include "qffitalgorithm.h"
#include "qffitalgorithmmanager.h"
#include "qfimfccsparameterinputdelegate.h"
#include <QtGui>
#include <QtCore>
#include "tools.h"
#include "dlgqfprogressdialog.h"

QFImFCCSFitEvaluationEditor::QFImFCCSFitEvaluationEditor(QFPluginServices* services,  QFEvaluationPropertyEditor *propEditor, QWidget* parent):
    QFEvaluationEditor(services, propEditor, parent),
    ui(new Ui::QFImFCCSFitEvaluationEditor)
{
    updatingData=true;
    
    currentSaveDirectory="";
    

    // setup fit progress dialog
    dlgFitProgress = new dlgQFFitAlgorithmProgressDialog(this);
    dlgFitProgressReporter=new dlgQFFitAlgorithmProgressDialogReporter(dlgFitProgress);

    // setup widgets
    ui->setupUi(this);
    ui->pltOverview->setRunSelectWidgetActive(true);
    connect(ui->pltOverview, SIGNAL(currentRunChanged(int)), this, SLOT(setCurrentRun(int)));
    ui->splitter->setChildrenCollapsible(false);
    ui->splitter->setStretchFactor(0,3);
    ui->splitter->setStretchFactor(1,1);
    QList<int> l;
    l<<3*ui->splitter->height()/4;
    l<<ui->splitter->height()/4;
    ui->splitter->setSizes(l);
    ui->datacut->set_allowCopyToAll(false);
    ui->datacut->set_copyToFilesEnabled(false);
    ui->datacut->set_copyToRunsEnabled(false);
    ui->datacut->set_runsName(tr("pixels"));

    ui->cmbDisplayData->addUsedSymbol(JKQTPfilledCircle);
    ui->cmbDisplayData->addUsedSymbol(JKQTPdot);




    ui->tableView->setModel(NULL);
    ui->tableView->setItemDelegate(new QFImFCCSParameterInputDelegate(ui->tableView));
    ui->tableView->setTextElideMode(Qt::ElideMiddle);





    // set correlation/residual plots:
    ui->pltData->get_plotter()->set_gridPrinting(true);
    ui->pltData->get_plotter()->addGridPrintingPlotter(0,1,ui->pltResiduals->get_plotter());
    ui->pltData->set_displayToolbar(true);
    ui->pltData->set_toolbarAlwaysOn(true);
    ui->pltData->getXAxis()->set_logAxis(true);

    ui->pltResiduals->set_displayToolbar(false);
    ui->pltResiduals->getXAxis()->set_axisLabel(tr("lag time $\\tau$ [seconds]"));
    ui->pltResiduals->getXAxis()->set_labelFontSize(11);
    ui->pltResiduals->getXAxis()->set_tickLabelFontSize(10);
    ui->pltResiduals->getXAxis()->set_logAxis(true);
    ui->pltResiduals->getYAxis()->set_axisLabel(tr("residuals"));
    ui->pltResiduals->getYAxis()->set_labelFontSize(11);
    ui->pltResiduals->getYAxis()->set_tickLabelFontSize(10);
    ui->pltData->getXAxis()->set_axisLabel("");
    ui->pltData->getYAxis()->set_axisLabel(tr("correlation function $g(\\tau)$"));
    ui->pltData->getYAxis()->set_labelFontSize(11);
    ui->pltData->getYAxis()->set_tickLabelFontSize(10);
    ui->pltData->getXAxis()->set_drawMode1(JKQTPCADMticks);
    ui->pltData->getXAxis()->set_tickLabelFontSize(10);
    ui->pltResiduals->getXAxis()->set_drawMode1(JKQTPCADMcomplete);
    ui->pltResiduals->get_plotter()->setBorder(1,1,1,1);
    ui->pltData->get_plotter()->setBorder(1,1,1,1);
    ui->pltResiduals->synchronizeToMaster(ui->pltData, true, false);
    ui->pltData->get_plotter()->set_useAntiAliasingForSystem(true);
    ui->pltData->get_plotter()->set_useAntiAliasingForGraphs(true);
    ui->pltResiduals->get_plotter()->set_useAntiAliasingForSystem(true);
    ui->pltResiduals->get_plotter()->set_useAntiAliasingForGraphs(true);
    ui->pltResiduals->set_displayMousePosition(false);
    ui->pltData->set_displayMousePosition(false);
    ui->pltData->get_plotter()->set_keyFontSize(9);
    ui->pltData->get_plotter()->set_keyXMargin(2);
    ui->pltData->get_plotter()->set_keyYMargin(2);
    ui->pltResiduals->get_plotter()->set_keyFontSize(9);
    ui->pltResiduals->get_plotter()->set_keyXMargin(2);
    ui->pltResiduals->get_plotter()->set_keyYMargin(2);
    ui->pltResiduals->useExternalDatastore(ui->pltData->getDatastore());
    ui->pltResiduals->setMinimumHeight(75);
    ui->pltData->setMinimumHeight(75);
    connect(ui->pltData, SIGNAL(plotMouseMove(double,double)), this, SLOT(plotMouseMoved(double,double)));
    connect(ui->pltResiduals, SIGNAL(plotMouseMove(double,double)), this, SLOT(plotMouseMoved(double,double)));
    connect(ui->pltData->get_plotter()->get_actZoomAll(), SIGNAL(triggered()), ui->pltResiduals, SLOT(zoomToFit()));


    menuEvaluation=propEditor->addOrFindMenu(tr("&Evaluation"), 0);
    menuImFCCSFit=propEditor->addOrFindMenu(tr("&Tools"), 0);

    actFitCurrent=new QAction(QIcon(":/imfccsfit/fit_fitcurrent.png"), tr("Fit &Current"), this);
    connect(actFitCurrent, SIGNAL(triggered()), this, SLOT(fitCurrent()));
    ui->btnEvaluateCurrent->setDefaultAction(actFitCurrent);
    menuEvaluation->addAction(actFitCurrent);

    actFitAllPixelsMT=new QAction(QIcon(":/imfccsfit/fit_fitallruns.png"), tr("Fit &All Pixels (MT)"), this);
    connect(actFitAllPixelsMT, SIGNAL(triggered()), this, SLOT(fitAllPixelsThreaded()));
    actFitAllPixelsMT->setEnabled(false);
    ui->btnEvaluateCurrentAllRuns->addAction(actFitAllPixelsMT);
    menuEvaluation->addAction(actFitAllPixelsMT);

    actFitAllPixels=new QAction(QIcon(":/imfccsfit/fit_fitallruns.png"), tr("Fit &All Pixels"), this);
    connect(actFitAllPixels, SIGNAL(triggered()), this, SLOT(fitAllPixels()));
    ui->btnEvaluateCurrentAllRuns->addAction(actFitAllPixels);
    ui->btnEvaluateCurrentAllRuns->setDefaultAction(actFitAllPixels);
    menuEvaluation->addAction(actFitAllPixels);

    actResetCurrent=new QAction(tr("&Reset Current"), this);
    actResetCurrent->setToolTip(tr("reset the currently displayed file (and pixel) to the initial parameters\nThis deletes all fit results stored for the current file."));
    connect(actResetCurrent, SIGNAL(triggered()), this, SLOT(resetCurrent()));
    ui->btnClearCurrent->setDefaultAction(actResetCurrent);
    menuEvaluation->addSeparator();
    menuEvaluation->addAction(actResetCurrent);

    actResetAllPixels=new QAction(tr("Reset All &Pixels"), this);
    actResetAllPixels->setToolTip(tr("reset all pixels to the initial parameters in the current file.\nThis deletes all fit results stored for all runs in the current file."));
    connect(actResetAllPixels, SIGNAL(triggered()), this, SLOT(resetAllPixels()));
    ui->btnClearAllPixels->setDefaultAction(actResetAllPixels);
    menuEvaluation->addAction(actResetAllPixels);

    actCopyToInitial=new QAction(tr("Copy to &Initial"), this);
    actCopyToInitial->setToolTip(tr("copy the currently displayed fit parameters to the set of initial parameters,\n so they are used by files/runs that were not fit yet."));
    connect(actCopyToInitial, SIGNAL(triggered()), this, SLOT(copyToInitial()));
    ui->btnCopyToInitial->setDefaultAction(actCopyToInitial);
    menuEvaluation->addAction(actCopyToInitial);


    actSaveReport=new QAction(QIcon(":/imfccsfit/fit_savereport.png"), tr("&Save Report"), this);
    connect(actSaveReport, SIGNAL(triggered()), this, SLOT(saveReport()));
    ui->btnSaveReport->setDefaultAction(actSaveReport);
    menuEvaluation->addSeparator();
    menuEvaluation->addAction(actSaveReport);

    actPrintReport=new QAction(QIcon(":/imfccsfit/fit_printreport.png"), tr("&Print Report"), this);
    connect(actPrintReport, SIGNAL(triggered()), this, SLOT(printReport()));
    ui->btnPrintReport->setDefaultAction(actPrintReport);
    menuEvaluation->addAction(actPrintReport);

    QMenu* m=menuImFCCSFit->addMenu(tr("configure evaluation for ..."));

    actConfigureForSPIMFCCS=new QAction(tr("SPIM-FCCS: normal diffusion"), this);
    connect(actConfigureForSPIMFCCS, SIGNAL(triggered()), this, SLOT(configureForSPIMFCCS()));
    m->addAction(actConfigureForSPIMFCCS);
    actConfigureForASPIMFCCS=new QAction(tr("SPIM-FCCS: anomalous diffusion"), this);
    connect(actConfigureForASPIMFCCS, SIGNAL(triggered()), this, SLOT(configureForASPIMFCCS()));
    m->addAction(actConfigureForASPIMFCCS);

    menuImFCCSFit->addSeparator();

    
    
    // connect widgets 
    connect(ui->pltData, SIGNAL(zoomChangedLocally(double,double,double,double,JKQtPlotter*)), this, SLOT(zoomChangedLocally(double,double,double,double,JKQtPlotter*)));
    connect(ui->pltResiduals, SIGNAL(zoomChangedLocally(double,double,double,double,JKQtPlotter*)), this, SLOT(zoomChangedLocally(double,double,double,double,JKQtPlotter*)));
/*    connect(ui->datacut, SIGNAL(copyUserMinToAll(int)), this, SLOT(copyUserMinToAll(int)));
    connect(ui->datacut, SIGNAL(copyUserMaxToAll(int)), this, SLOT(copyUserMaxToAll(int)));
    connect(ui->datacut, SIGNAL(copyUserMinMaxToAll(int,int)), this, SLOT(copyUserMinMaxToAll(int,int)));
    connect(ui->datacut, SIGNAL(copyUserMinToAllRuns(int)), this, SLOT(copyUserMinToAllRuns(int)));
    connect(ui->datacut, SIGNAL(copyUserMaxToAllRuns(int)), this, SLOT(copyUserMaxToAllRuns(int)));
    connect(ui->datacut, SIGNAL(copyUserMinMaxToAllRuns(int,int)), this, SLOT(copyUserMinMaxToAllRuns(int,int)));*/

    updatingData=false;
}

QFImFCCSFitEvaluationEditor::~QFImFCCSFitEvaluationEditor()
{
    delete ui;
}

int QFImFCCSFitEvaluationEditor::getUserMin(int index)
{
    QFImFCCSFitEvaluationItem* item=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!current) return 0;
    return getUserMin(item->getFitFile(0), index);
}

int QFImFCCSFitEvaluationEditor::getUserMax(int index)
{
    QFImFCCSFitEvaluationItem* item=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!current) return 0;
    return getUserMax(item->getFitFile(0), index);
}

int QFImFCCSFitEvaluationEditor::getUserMin(QFRawDataRecord *rec, int index)
{
    return getUserMin(rec, index, ui->datacut->get_userMin());
}

int QFImFCCSFitEvaluationEditor::getUserMax(QFRawDataRecord *rec, int index)
{
    return getUserMax(rec, index, ui->datacut->get_userMax());
}

void QFImFCCSFitEvaluationEditor::zoomChangedLocally(double newxmin, double newxmax, double newymin, double newymax, JKQtPlotter *sender) {
    if (sender==ui->pltData) {
        ui->pltResiduals->setX(newxmin, newxmax);
        ui->pltResiduals->update_plot();
    }
}

void QFImFCCSFitEvaluationEditor::connectWidgets(QFEvaluationItem* current, QFEvaluationItem* old) {
    // called when this widget should be connected to a new QFEvaluationItem

    QFImFCCSFitEvaluationItem* item=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    QFImFCCSFitEvaluationItem* item_old=qobject_cast<QFImFCCSFitEvaluationItem*>(old);
    if (ui->tableView->model()) {
        disconnect(ui->tableView->model(), 0, this, 0);
    }
    ui->tableView->setModel(NULL);
    ui->pltOverview->setRDR(0);
    ui->lstFileSets->setModel(NULL);

    if (old!=NULL && item_old!=NULL) {
        /* disconnect item_old and clear all widgets here */
        disconnect(item_old, SIGNAL(highlightingChanged(QFRawDataRecord*, QFRawDataRecord*)), this, SLOT(highlightingChanged(QFRawDataRecord*, QFRawDataRecord*)));
        disconnect(ui->btnAddFile, SIGNAL(clicked()), item_old, SLOT(addFitFile()));
        disconnect(ui->btnRemoveFile, SIGNAL(clicked()), item_old, SLOT(removeFitFile()));
        disconnect(item, SIGNAL(fileChanged(int,QFRawDataRecord*)), this, SLOT(fileChanged(int,QFRawDataRecord*)));
        disconnect(ui->datacut, SIGNAL(slidersChanged(int, int, int, int)), this, SLOT(slidersChanged(int, int, int, int)));
        disconnect(item_old->getParameterInputTableModel(), SIGNAL(fitParamChanged()), this, SLOT(displayEvaluation()));
        disconnect(item_old->getParameterInputTableModel(), SIGNAL(modelRebuilt()), this, SLOT(setParameterTableSpans()));
    }



    if (item) {
        updatingData=true;

        ui->cmbFitAlgorithm->setCurrentAlgorithm(item->getFitAlgorithm()->id());
        ui->cmbWeight->setCurrentWeight(item->getFitDataWeighting());
        ui->cmbDisplayData->setCurrentIndex(item->getProperty("imFCCSFit/datadisplay", 1).toInt());
        ui->cmbErrorDisplay->setCurrentIndex(item->getProperty("imFCCSFit/errordisplay", 0).toInt());
        ui->chkGrid->setChecked(item->getProperty("imFCCSFit/grid", true).toBool());
        ui->chkKey->setChecked(item->getProperty("imFCCSFit/key", true).toBool());
        ui->chkSaveStrings->setChecked(!item->getProperty("dontSaveFitResultMessage", true).toBool());
        ui->lstFileSets->setModel(item->getFileSetsModel());

        ui->tableView->setModel(item->getParameterInputTableModel());
        connect(item->getParameterInputTableModel(), SIGNAL(modelRebuilt()), this, SLOT(ensureCorrectParamaterModelDisplay()));
        setParameterTableSpans();

        /* connect widgets and fill with data from item here */
        connect(ui->btnAddFile, SIGNAL(clicked()), item, SLOT(addFitFile()));
        connect(ui->btnRemoveFile, SIGNAL(clicked()), item, SLOT(removeFitFile()));
        connect(item, SIGNAL(fileChanged(int,QFRawDataRecord*)), this, SLOT(fileChanged(int,QFRawDataRecord*)));
        connect(ui->datacut, SIGNAL(slidersChanged(int, int, int, int)), this, SLOT(slidersChanged(int, int, int, int)));
        connect(item->getParameterInputTableModel(), SIGNAL(fitParamChanged()), this, SLOT(displayEvaluation()));
        connect(ui->lstFileSets, SIGNAL(clicked(QModelIndex)), this, SLOT(filesSetActivated(QModelIndex)));
        connect(item->getParameterInputTableModel(), SIGNAL(modelRebuilt()), this, SLOT(setParameterTableSpans()));
        ui->pltOverview->setRDR(item->getFitFile(0));
        updatingData=false;
    }

    ensureCorrectParamaterModelDisplay();
    displayEvaluation();
}

void QFImFCCSFitEvaluationEditor::resultsChanged() {
    /* some other evaluation or the user changed the results stored in the current raw data record,
       so redisplay */
    ensureCorrectParamaterModelDisplay();
    displayEvaluation();
}

void QFImFCCSFitEvaluationEditor::readSettings() {
    // read widget settings
    if (!settings) return;
    currentSaveDirectory=settings->getQSettings()->value(QString("imfccs_fit/editor/lastSaveDirectory"), currentSaveDirectory).toString();
    if (current) {
        loadSplitter(*(settings->getQSettings()), ui->splitter, QString("QFImFCCSFitEvaluationEditor%1/splitter_1").arg(current->getID()));
        loadSplitter(*(settings->getQSettings()), ui->splitter_2, QString("QFImFCCSFitEvaluationEditor%1/splitter_2").arg(current->getID()));
        loadSplitter(*(settings->getQSettings()), ui->splitter_3, QString("QFImFCCSFitEvaluationEditor%1/splitter_3").arg(current->getID()));
    }
}

void QFImFCCSFitEvaluationEditor::writeSettings() {
    // write widget settings
    if (!settings) return;
    settings->getQSettings()->setValue(QString("imfccs_fit/editor/lastSaveDirectory"), currentSaveDirectory);
    if (current) {
        saveSplitter(*(settings->getQSettings()), ui->splitter, QString("QFImFCCSFitEvaluationEditor%1/splitter_1").arg(current->getID()));
        saveSplitter(*(settings->getQSettings()), ui->splitter_2, QString("QFImFCCSFitEvaluationEditor%1/splitter_2").arg(current->getID()));
        saveSplitter(*(settings->getQSettings()), ui->splitter_3, QString("QFImFCCSFitEvaluationEditor%1/splitter_3").arg(current->getID()));
    }
}

void QFImFCCSFitEvaluationEditor::showFitAlgorithmHelp()
{
    if (!current) return;
    QFImFCCSFitEvaluationItem* data=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    QString pid=ui->cmbFitAlgorithm->currentFitAlgorithmID();
    int ppid=services->getFitAlgorithmManager()->getPluginForID(pid);
    QFFitAlgorithm* algorithm=data->getFitAlgorithm(pid);
    QString help=services->getFitAlgorithmManager()->getPluginHelp(ppid, pid);
    if (QFile::exists(help) && algorithm) {
        QFPluginServices::getInstance()->displayHelpWindow(help);
    } else {
        QMessageBox::information(this, tr("imFCCS Fit"), tr("No Online-Help for this fit algorithm available."));
    }
}

void QFImFCCSFitEvaluationEditor::configFitAlgorithm()
{
    if (!current) return;
    QFImFCCSFitEvaluationItem* data=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!data) return;
    QString pid=ui->cmbFitAlgorithm->currentFitAlgorithmID();
    QFFitAlgorithm* algorithm=data->getFitAlgorithm(pid);
    if (algorithm) {
        data->restoreQFFitAlgorithmParameters(algorithm);
        if (algorithm->displayConfig()) {
            data->storeQFFitAlgorithmParameters(algorithm);
        }
    }
}

void QFImFCCSFitEvaluationEditor::fitAlgorithmChanged(int model)
{
    if (!current) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QString pid=ui->cmbFitAlgorithm->currentFitAlgorithmID();
    QFImFCCSFitEvaluationItem* data=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    data->setFitAlgorithm(pid);
    /*populateFitButtons(data->getFitAlgorithm()->isThreadSafe());
    actFitAllThreaded->setEnabled(data->getFitAlgorithm()->isThreadSafe());
    actFitAllFilesThreaded->setEnabled(data->getFitAlgorithm()->isThreadSafe());
    actFitAllRunsThreaded->setEnabled(data->getFitAlgorithm()->isThreadSafe());*/
    QApplication::restoreOverrideCursor();
}

void QFImFCCSFitEvaluationEditor::displayOptionsChanged()
{
    if (!current) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QString pid=ui->cmbFitAlgorithm->currentFitAlgorithmID();

    current->setQFProperty("imFCCSFit/datadisplay", ui->cmbDisplayData->currentIndex());
    current->setQFProperty("imFCCSFit/errordisplay",ui->cmbErrorDisplay->currentIndex());
    current->setQFProperty("imFCCSFit/grid",ui->chkGrid->isChecked());
    current->setQFProperty("imFCCSFit/key",ui->chkKey->isChecked());

    displayData();
    QApplication::restoreOverrideCursor();
}

void QFImFCCSFitEvaluationEditor::setCurrentRun(int run)
{
    if (!current) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QFImFCCSFitEvaluationItem* data=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!data) return;
    data->setCurrentIndex(run);

    QFRawDataRecord* currentRecord=data->getFitFile(0);

    if (currentRecord) {
        ui->datacut->disableSliderSignals();
        ui->datacut->set_min(getUserRangeMin(currentRecord, data->getCurrentIndex()));
        ui->datacut->set_max(getUserRangeMax(currentRecord, data->getCurrentIndex()));
        ui->datacut->set_userMin(getUserMin(currentRecord, data->getCurrentIndex(), getUserRangeMin(currentRecord, data->getCurrentIndex())));
        ui->datacut->set_userMax(getUserMax(currentRecord, data->getCurrentIndex(), getUserRangeMax(currentRecord, data->getCurrentIndex())));
        ui->datacut->enableSliderSignals();
    }

    displayEvaluation();
    QApplication::restoreOverrideCursor();
}

void QFImFCCSFitEvaluationEditor::configureForSPIMFCCS() {
    QFImFCCSFitEvaluationItem* data=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!data) return;

    if (data->getFitFileCount()<3) {
        while (data->getFitFileCount()<3) {
            data->addFitFile();
        }
    } else if (data->getFitFileCount()>3) {
        while (data->getFitFileCount()>3) {
            data->removeFitFile();
        }
    }

    data->setFitFunction(0, "fccs_spim_fw_diff2coloracfg");
    data->setFitFunction(1, "fccs_spim_fw_diff2coloracfr");
    data->setFitFunction(2, "fccs_spim_fw_diff2colorccf");
    data->clearLinkParameters();

    QStringList globals;
    globals<<"concentration_a"<<"concentration_b"<<"concentration_ab"
          <<"diff_coeff_a"  <<"diff_coeff_b"  <<"diff_coeff_ab"
         <<"crosstalk"<<"focus_distance_x"  <<"focus_distance_y"  <<"focus_distance_z"
        <<"focus_width1"  <<"focus_width2"  <<"focus_height1"  <<"focus_height2"
       <<"pixel_width"<<"count_rate1"<<"count_rate2"<<"background1"<<"background2";

    for (int g=0; g<globals.size(); g++) {
        data->setLinkParameter(0, globals[g], g);
        data->setLinkParameter(1, globals[g], g);
        data->setLinkParameter(2, globals[g], g);
    }

}

void QFImFCCSFitEvaluationEditor::configureForASPIMFCCS() {
    QFImFCCSFitEvaluationItem* data=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!data) return;

    if (data->getFitFileCount()<3) {
        while (data->getFitFileCount()<3) {
            data->addFitFile();
        }
    } else if (data->getFitFileCount()>3) {
        while (data->getFitFileCount()>3) {
            data->removeFitFile();
        }
    }

    data->setFitFunction(0, "fccs_spim_fw_adiff2coloracfg");
    data->setFitFunction(1, "fccs_spim_fw_adiff2coloracfr");
    data->setFitFunction(2, "fccs_spim_fw_adiff2colorccf");
    data->clearLinkParameters();

    QStringList globals;
    globals<<"concentration_a"<<"concentration_b"<<"concentration_ab"
          <<"diff_acoeff_a"  <<"diff_acoeff_b"  <<"diff_acoeff_ab"
         <<"diff_alpha_a"  <<"diff_alpha_b"  <<"diff_alpha_ab"
         <<"crosstalk"<<"focus_distance_x"  <<"focus_distance_y"  <<"focus_distance_z"
        <<"focus_width1"  <<"focus_width2"  <<"focus_height1"  <<"focus_height2"
       <<"pixel_width"<<"count_rate1"<<"count_rate2"<<"background1"<<"background2";

    for (int g=0; g<globals.size(); g++) {
        data->setLinkParameter(0, globals[g], g);
        data->setLinkParameter(1, globals[g], g);
        data->setLinkParameter(2, globals[g], g);
    }

}

void QFImFCCSFitEvaluationEditor::filesSetActivated(const QModelIndex &idx)
{
    QFImFCCSFitEvaluationItem* data=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!data) return;
    data->setFitFileSet(idx.row());
}

void QFImFCCSFitEvaluationEditor::ensureCorrectParamaterModelDisplay()
{
    QElapsedTimer t;
    t.start();
    //qDebug()<<"ensureCorrectParamaterModelDisplay";
    QFImFCCSFitEvaluationItem* eval=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    setUpdatesEnabled(false);
    ui->tableView->setEditTriggers(QAbstractItemView::DoubleClicked| QAbstractItemView::SelectedClicked| QAbstractItemView::EditKeyPressed| QAbstractItemView::EditKeyPressed|QAbstractItemView::CurrentChanged);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QHeaderView* h=ui->tableView->verticalHeader();
    if (h) {
        h->setResizeMode(QHeaderView::Fixed);
        QFontMetrics fm(h->font());
        int height=qMax(16, (int)ceil(double(fm.height())*1.5));
        //qDebug()<<"resizing verticals to "<<height;
        for (int i=0; i<h->count(); i++) {
            h->resizeSection(i, height);
        }
    }
    h=ui->tableView->horizontalHeader();
   if (h) {
       int width=qMax(200, ui->tableView->width()/4);
       if (eval) width=qMax(width, ui->tableView->width()/eval->getParameterInputTableModel()->columnCount());
       //ui->tableView->resizeColumnsToContents();
       for (int i=0; i<h->count(); i++) {
           if (i==0) {
               //qDebug()<<"  resizing h"<<i<<" to contents "<<t.elapsed()<<"ms";
               ui->tableView->resizeColumnToContents(i);
               h->setResizeMode(i, QHeaderView::Interactive);
               //qDebug()<<"  resizing h"<<i<<" to contents done "<<t.elapsed()<<"ms";
           }
           //h->setResizeMode(i, QHeaderView::Interactive);
           if (i>0 && eval) {
               int ii=(i-1)%(eval->getParameterInputTableModel()->getColsPerRDR());
               //qDebug()<<"  resizing h"<<i<<" to contents "<<t.elapsed()<<"ms";
               if (ii==0) h->resizeSection(i, 125);
               else if (ii==1) h->resizeSection(i, 50);
               else if (ii==2) h->resizeSection(i, 50);
               else if (ii==3) h->resizeSection(i, 24);
               else if (ii==4) h->resizeSection(i, 75);
               //qDebug()<<"  resizing h"<<i<<" to contents done "<<t.elapsed()<<"ms";
               //h->setResizeMode(i, QHeaderView::Interactive);
               /*
               if (ii==3)  h->resizeSection(i, 20);
               else if (ii==0 && ui->tableView->columnWidth(i)>2*ui->tableView->width()/3) h->resizeSection(i, qMax(200, 2*ui->tableView->width()/3));
               else if (ii==0 && ui->tableView->columnWidth(i)<200) h->resizeSection(i, 200);*/

               //qDebug()<<"resizeH: ii="<<i<<" width="<<width<<"  files="<<eval->getNumberOfFitFiles();
           }
       }
   }
   setUpdatesEnabled(true);
   //qDebug()<<"ensureCorrectParamaterModelDisplay done "<<t.elapsed()<<"ms";

}

void QFImFCCSFitEvaluationEditor::fileChanged(int num, QFRawDataRecord *file)
{
    QFImFCCSFitEvaluationItem* eval=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (num==0) ui->pltOverview->setRDR(file);
    if (!eval) return;

    ui->datacut->disableSliderSignals();
    ui->datacut->set_min(0);
    ui->datacut->set_max(getUserRangeMax(file, eval->getCurrentIndex()));
    ui->datacut->set_userMin(getUserMin(0));
    ui->datacut->set_userMax(getUserMax(getUserRangeMax(file, eval->getCurrentIndex())));
    ui->datacut->enableSliderSignals();
    displayData();
}

void QFImFCCSFitEvaluationEditor::plotMouseMoved(double x, double y)
{
    ui->labPosition->setText(tr("current position: (%1s, %2)").arg(floattohtmlstr(x, 3, true, 1e-18).c_str()).arg(y));
}

void QFImFCCSFitEvaluationEditor::displayEvaluation() {
    if (!current) return;
    QFImFCCSFitEvaluationItem* eval=qobject_cast<QFImFCCSFitEvaluationItem*>(current);

    displayData();
    eval->getParameterInputTableModel()->rebuildModel();

}

void QFImFCCSFitEvaluationEditor::displayData() {
    if (!current) return;
    QFImFCCSFitEvaluationItem* eval=qobject_cast<QFImFCCSFitEvaluationItem*>(current);


    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    JKQTPdatastore* ds=ui->pltData->getDatastore();
    ui->pltData->set_doDrawing(false);
    ui->pltResiduals->set_doDrawing(false);
    ui->pltData->clearGraphs();
    ui->pltResiduals->clearGraphs();
    ui->pltData->get_plotter()->set_showKey(ui->chkKey->isChecked());
    ui->pltResiduals->get_plotter()->set_showKey(ui->chkKey->isChecked());
    ui->pltData->get_plotter()->get_xAxis()->set_drawGrid(ui->chkGrid->isChecked());
    ui->pltData->get_plotter()->get_yAxis()->set_drawGrid(ui->chkGrid->isChecked());
    ui->pltResiduals->get_plotter()->get_xAxis()->set_drawGrid(ui->chkGrid->isChecked());
    ui->pltResiduals->get_plotter()->get_yAxis()->set_drawGrid(ui->chkGrid->isChecked());
    ds->clear();
    QList<QColor> cols;
    cols<<QColor("darkgreen")<<QColor("red")<<QColor("blue")<<QColor("darkorange")<<QColor("purple")<<QColor("maroon");
    //qDebug()<<"\n\n\n\n ### displayData() eval="<<eval<<" ###";
    if (eval) {
        for (int file=0; file<eval->getFitFileCount(); file++) {
            QFFitFunction* ff=eval->getFitFunction(file);
            QFRawDataRecord* rec=eval->getFitFile(file);

            QFRDRFCSDataInterface* fcs=dynamic_cast<QFRDRFCSDataInterface*>(rec);
            QFRDRImageToRunInterface* runintf=dynamic_cast<QFRDRImageToRunInterface*>(rec);

            double* data=fcs->getCorrelationRun(eval->getCurrentIndex());
            double* tau=fcs->getCorrelationT();
            bool wOK=false;
            double* sigma=eval->allocWeights(&wOK, rec, eval->getCurrentIndex(), ui->datacut->get_userMin(), ui->datacut->get_userMax());//fcs->getCorrelationRunError(eval->getCurrentIndex());
            if (!wOK && sigma) {
                free(sigma);
                sigma=NULL;
            }
            long N=fcs->getCorrelationN();
            if (data && tau) {
                size_t c_tau=ds->addCopiedColumn(tau, N, tr("file%1: tau [s]").arg(file+1));
                size_t c_data=ds->addCopiedColumn(data, N, tr("file%1: g(tau)").arg(file+1));
                size_t c_error=0;
                if (sigma) c_error=ds->addCopiedColumn(sigma, N, tr("file%1: errors").arg(file+1));
                //qDebug()<<c_tau<<c_data<<c_error;
                JKQTPxyLineErrorGraph* g=new JKQTPxyLineErrorGraph(ui->pltData->get_plotter());
                QString plotname=QString("\\verb{")+rec->getName()+QString(": ")+fcs->getCorrelationRunName(eval->getCurrentIndex())+QString("}");
                g->set_title(plotname);
                g->set_xColumn(c_tau);
                g->set_yColumn(c_data);
                g->set_xErrorColumn(-1);
                if (sigma) g->set_yErrorColumn(c_error);
                else g->set_yErrorColumn(-1);
                g->set_color(cols.value(file, g->get_color()));
                QColor ec=g->get_color().lighter();
                g->set_errorColor(ec);
                ec.setAlphaF(0.5);
                g->set_errorFillColor(ec);
                g->set_symbol(ui->cmbDisplayData->getSymbol());
                g->set_drawLine(ui->cmbDisplayData->getDrawLine());
                g->set_symbolSize(5);
                g->set_lineWidth(1);
                g->set_yErrorStyle(ui->cmbErrorDisplay->getErrorStyle());
                g->set_datarange_start(ui->datacut->get_userMin());
                g->set_datarange_end(ui->datacut->get_userMax());
                ui->pltData->get_plotter()->addGraph(g);

                //qDebug()<<ff<<ff->name();

                double* params=eval->allocFillParameters(rec, eval->getCurrentIndex(), ff);
                double* err=eval->allocFillParameterErrors(rec, eval->getCurrentIndex(), ff);
                bool* fix=eval->allocFillFix(rec, eval->getCurrentIndex(), ff);
                QVector<double> paramsV;
                for (int i=0; i<ff->paramCount(); i++) {
                    paramsV<<params[i];
                    //qDebug()<<ff->getParameterID(i)<<" = "<<params[i];
                }
                JKQTPxQFFitFunctionLineGraph* g_fit=new JKQTPxQFFitFunctionLineGraph();
                g_fit->set_title(tr("fit: %1").arg(plotname));
                g_fit->set_fitFunction(ff, false);
                g_fit->set_paramsVector(paramsV);
                g_fit->set_color(g->get_color());
                g_fit->set_style(Qt::DashLine);
                g_fit->set_lineWidth(2);
                g_fit->set_minSamples(30);
                g_fit->set_maxRefinementDegree(4);
                g_fit->set_plotRefinement(true);
                ui->pltData->get_plotter()->addGraph(g_fit);

                QFFitStatistics fstat=ff->calcFitStatistics(N, tau, data, sigma, ui->datacut->get_userMin(), ui->datacut->get_userMax(), params, err, fix, 3, 100);

                size_t c_resid=ds->addCopiedColumn(fstat.residuals, N, tr("file%1: residuals").arg(file+1));
                size_t c_residw=ds->addCopiedColumn(fstat.residuals_weighted, N, tr("file%1: weighted residuals").arg(file+1));
                size_t c_residat=ds->addCopiedColumn(fstat.tau_runavg, fstat.runAvgN, tr("file%1: tau for avg. res.").arg(file+1));
                size_t c_resida=ds->addCopiedColumn(fstat.residuals_runavg, N, tr("file%1: avg. residuals").arg(file+1));
                size_t c_residaw=ds->addCopiedColumn(fstat.residuals_runavg_weighted, N, tr("file%1: weighted avg. residuals").arg(file+1));
                JKQTPxyLineGraph* g_res=new JKQTPxyLineGraph();
                g_res->set_xColumn(c_tau);
                g_res->set_yColumn(c_resid);
                g_res->set_color(g->get_color());
                g_res->set_symbol(ui->cmbDisplayData->getSymbol());
                g_res->set_symbolSize(5);
                g_res->set_lineWidth(1);
                g_res->set_drawLine(ui->cmbDisplayData->getDrawLine());
                g_res->set_datarange_start(ui->datacut->get_userMin());
                g_res->set_datarange_end(ui->datacut->get_userMax());
                ui->pltResiduals->get_plotter()->addGraph(g_res);
                JKQTPxyLineGraph* g_resa=new JKQTPxyLineGraph();
                g_resa->set_xColumn(c_residat);
                g_resa->set_yColumn(c_resida);
                g_resa->set_color(g->get_color());
                g_resa->set_drawLine(true);
                g_resa->set_symbol(JKQTPnoSymbol);
                ui->pltResiduals->get_plotter()->addGraph(g_resa);
                fstat.free();




                free(params);
                free(err);
                free(fix);
            }
            if (sigma) free(sigma);

            ui->pltData->zoomToFit();
            ui->pltResiduals->zoomToFit();
        }

    }
    ui->pltData->set_doDrawing(true);
    ui->pltResiduals->set_doDrawing(true);
    ui->pltData->update_plot();
    ui->pltResiduals->update_plot();
    QApplication::restoreOverrideCursor();
}





void QFImFCCSFitEvaluationEditor::fitCurrent() {
    if (!current) return;
    QFImFCCSFitEvaluationItem* eval=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!eval) return;
    QFFitAlgorithm* falg=eval->getFitAlgorithm();
    if (!falg) return;
    QList<QFRawDataRecord*> records=eval->getFitFiles();
    if (records.size()<=0) return;

    falg->setReporter(dlgFitProgressReporter);
    QString runname=tr("average");
    if (eval->getCurrentIndex()>=0) runname=QString::number(eval->getCurrentIndex());
    dlgFitProgress->reportSuperStatus(tr("fit run %1<br>using algorithm '%2' \n").arg(runname).arg(falg->name()));
    dlgFitProgress->reportStatus("");
    dlgFitProgress->setProgressMax(100);
    dlgFitProgress->setSuperProgressMax(100);
    dlgFitProgress->setProgress(0);
    dlgFitProgress->setSuperProgress(0);
    dlgFitProgress->setAllowCancel(true);
    dlgFitProgress->display();
    QApplication::processEvents();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    eval->doFit(records, eval->getCurrentIndex(), getUserMin(records[0], eval->getCurrentIndex(), ui->datacut->get_userMin()), getUserMax(records[0], eval->getCurrentIndex(), ui->datacut->get_userMax()), dlgFitProgressReporter, ProgramOptions::getConfigValue(eval->getType()+"/log", false).toBool());
    for (int i=0; i<records.size(); i++) {
        records[i]->enableEmitResultsChanged(true);
    }

    dlgFitProgress->reportSuperStatus(tr("fit done ... updating user interface\n"));
    dlgFitProgress->reportStatus("");
    dlgFitProgress->setProgressMax(100);
    dlgFitProgress->setSuperProgressMax(100);
    displayEvaluation();
    QApplication::restoreOverrideCursor();
    dlgFitProgress->done();
    falg->setReporter(NULL);
    QApplication::processEvents();
    current->emitResultsChanged();
    eval->getParameterInputTableModel()->rebuildModel();
}

void QFImFCCSFitEvaluationEditor::fitAllPixelsThreaded()
{
    if (!current) return;
    QFImFCCSFitEvaluationItem* eval=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!eval) return;
    QFFitAlgorithm* falg=eval->getFitAlgorithm();
    if (!falg->isThreadSafe()) {
        fitAllPixels();
        return;
    }
    if (!falg) return;

    dlgQFProgressDialog* dlgTFitProgress=new dlgQFProgressDialog(this);
    dlgTFitProgress->reportTask(tr("fit all runs in the current file<br>using algorithm '%1' \n").arg(falg->name()));
    dlgTFitProgress->setProgressMax(100);
    dlgTFitProgress->setProgress(0);
    dlgTFitProgress->setAllowCancel(true);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    dlgTFitProgress->display();


    int items=0;
    int thread=0;
    dlgTFitProgress->reportStatus("creating thread objects ...");
    QApplication::processEvents();
    QList<QPointer<QFRawDataRecord> > recs=eval->getApplicableRecords();
    //QList<QFFitResultsByIndexEvaluationFitThread*> threads;
    int threadcount=qMax(2,ProgramOptions::getInstance()->getMaxThreads());
    if (ProgramOptions::getConfigValue(eval->getType()+"/overrule_threads", false).toBool()) {
        threadcount=qMax(2,ProgramOptions::getConfigValue(eval->getType()+"/threads", 1).toInt());
    }
    /*for (int i=0; i<threadcount; i++) {
        threads.append(new QFFitResultsByIndexEvaluationFitThread(true, this));
    }


    // enqueue the jobs in the several threads and finally make sure they know when to stop
    dlgTFitProgress->reportStatus("distributing jobs ...");
    QApplication::processEvents();
    //for (int i=0; i<recs.size(); i++) {
        QFRawDataRecord* record=eval->getHighlightedRecord();
        QFRDRRunSelectionsInterface* rsel=qobject_cast<QFRDRRunSelectionsInterface*>(record);

        if (record ) {
            record->disableEmitResultsChanged();
            int runmax=eval->getIndexMax(record);
            int runmin=eval->getIndexMin(record);
            items=items+runmax-runmin+1;
            for (int run=runmin; run<=runmax; run++) {
                bool doall=!current->getProperty("LEAVEOUTMASKED", false).toBool();
                //qDebug()<<doall;
                if (run<=runmax && (doall || (!doall && rsel && !rsel->leaveoutRun(run)))) {
                    threads[thread]->addJob(eval, record, run, getUserMin(record, run, datacut->get_userMin()), getUserMax(record, run, datacut->get_userMax()));
                    thread++;
                    if (thread>=threadcount) thread=0;
                }
            }
        }
    //}
    dlgTFitProgress->setProgressMax(items+5);


    // start all threads and wait for them to finish
    for (int i=0; i<threadcount; i++) {
        threads[i]->start();
    }
    bool finished=false;
    bool canceled=false;
    QTime time;
    time.start();
    while (!finished) {


        // check whether all threads have finished and collect progress info
        finished=true;
        int jobsDone=0;
        for (int i=0; i<threadcount; i++) {
            finished=finished&&threads[i]->isFinished();
            jobsDone=jobsDone+threads[i]->getJobsDone();
        }
        dlgTFitProgress->setProgress(jobsDone);
        if (!canceled) {
            double runtime=double(time.elapsed())/1.0e3;
            double timeperfit=runtime/double(jobsDone);
            double estimatedRuntime=double(items)*timeperfit;
            double remaining=estimatedRuntime-runtime;
            dlgTFitProgress->reportStatus(tr("processing fits in %3 threads ... %1/%2 done\nruntime: %4:%5       remaining: %6:%7 [min:secs]       %8 fits/sec").arg(jobsDone).arg(items).arg(threadcount).arg(uint(int(runtime)/60),2,10,QChar('0')).arg(uint(int(runtime)%60),2,10,QChar('0')).arg(uint(int(remaining)/60),2,10,QChar('0')).arg(uint(int(remaining)%60),2,10,QChar('0')).arg(1.0/timeperfit,5,'f',2));
        }
        QApplication::processEvents();

        // check for user canceled
        if (!canceled && dlgTFitProgress->isCanceled()) {
            dlgTFitProgress->reportStatus("sending all threads the CANCEL signal");
            for (int i=0; i<threadcount; i++) {
                threads[i]->cancel(false);
            }
            canceled=true;
        }
    }


    // free memory
    for (int i=0; i<threadcount; i++) {
        delete threads[i];
    }*/

    dlgTFitProgress->reportStatus(tr("fit done ... updating user interface\n"));
    dlgTFitProgress->setProgress(items+2);
    QApplication::processEvents();

    /*for (int i=0; i<recs.size(); i++) {
        QFRawDataRecord* record=recs[i];
        if (record ) {
            record->enableEmitResultsChanged(true);
        }
    }*/
    current->emitResultsChanged();

    dlgTFitProgress->setProgress(items+3);
    QApplication::processEvents();
    displayEvaluation();
    dlgTFitProgress->setProgress(items+5);
    QApplication::processEvents();
    QApplication::restoreOverrideCursor();
    dlgTFitProgress->done();
    delete dlgTFitProgress;
    eval->getParameterInputTableModel()->rebuildModel();
}

void QFImFCCSFitEvaluationEditor::fitAllPixels()
{
    if (!current) return;
    QFImFCCSFitEvaluationItem* eval=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!eval) return;
    QFFitAlgorithm* falg=eval->getFitAlgorithm();
    if (!falg) return;
    QList<QFRawDataRecord*> records=eval->getFitFiles();
    if (records.size()<=0) return;
    QFRDRRunSelectionsInterface* rsel=qobject_cast<QFRDRRunSelectionsInterface*>(records[0]);

    falg->setReporter(dlgFitProgressReporter);
    int runmax=eval->getIndexMax(records[0]);
    int runmin=eval->getIndexMin(records[0]);
    QString runname=tr("average");
    if (eval->getCurrentIndex()>=0) runname=QString::number(eval->getCurrentIndex());
    dlgFitProgress->reportSuperStatus(tr("fit run %1<br>using algorithm '%2' \n").arg(runname).arg(falg->name()));
    dlgFitProgress->reportStatus("");
    dlgFitProgress->setProgressMax(100);
    dlgFitProgress->setSuperProgressMax(runmax-runmin);
    dlgFitProgress->setProgress(0);
    dlgFitProgress->setSuperProgress(0);
    dlgFitProgress->setAllowCancel(true);
    dlgFitProgress->display();
    QApplication::processEvents();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));


    QTime time;
    time.start();
    for (int run=runmin; run<=runmax; run++) {
        bool doall=true;//!current->getProperty("leaveoutMasked", false).toBool();
        if (doall || (!doall && rsel && !rsel->leaveoutRun(run))) {
            falg->setReporter(dlgFitProgressReporter);
            QString runname=tr("average");
            if (run>=0) runname=QString::number(run);
            double runtime=double(time.elapsed())/1.0e3;
            double timeperfit=runtime/double(run-runmin);
            double estimatedRuntime=double(runmax-runmin)*timeperfit;
            double remaining=estimatedRuntime-runtime;
            dlgFitProgress->reportSuperStatus(tr("fit run %1<br>using algorithm '%2' \nruntime: %3:%4       remaining: %5:%6 [min:secs]       %9 fits/sec").arg(runname).arg(falg->name()).arg(uint(int(runtime)/60),2,10,QChar('0')).arg(uint(int(runtime)%60),2,10,QChar('0')).arg(uint(int(remaining)/60),2,10,QChar('0')).arg(uint(int(remaining)%60),2,10,QChar('0')).arg(1.0/timeperfit,5,'f',2));

            //doFit(record, run);
            eval->doFit(records, run, getUserMin(records[0], run, ui->datacut->get_userMin()), getUserMax(records[0], run, ui->datacut->get_userMax()), dlgFitProgressReporter, ProgramOptions::getConfigValue(eval->getType()+"/log", false).toBool());

            dlgFitProgress->incSuperProgress();
            QApplication::processEvents();
            falg->setReporter(NULL);
            if (dlgFitProgress->isCanceled()) break;
        }
    }
    for (int i=0; i<records.size(); i++) {
        records[i]->enableEmitResultsChanged(true);
    }

    dlgFitProgress->reportSuperStatus(tr("fit done ... updating user interface\n"));
    dlgFitProgress->reportStatus("");
    dlgFitProgress->setProgressMax(100);
    dlgFitProgress->setSuperProgressMax(100);
    displayEvaluation();
    QApplication::restoreOverrideCursor();
    dlgFitProgress->done();
    falg->setReporter(NULL);
    QApplication::processEvents();
    current->emitResultsChanged();
    eval->getParameterInputTableModel()->rebuildModel();
}





void QFImFCCSFitEvaluationEditor::resetCurrent()
{
    if (!current) return;
    QFImFCCSFitEvaluationItem* eval=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!eval) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    for (int f=0; f<eval->getFitFileCount(); f++) {
        QFRawDataRecord* rec=eval->getFitFile(f);
        eval->resetAllFitResultsCurrent(rec);
    }
    displayEvaluation();
    QApplication::restoreOverrideCursor();
}

void QFImFCCSFitEvaluationEditor::resetAllPixels()
{
    if (!current) return;
    QFImFCCSFitEvaluationItem* eval=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!eval) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    for (int f=0; f<eval->getFitFileCount(); f++) {
        QFRawDataRecord* rec=eval->getFitFile(f);
        eval->resetAllFitResultsAllIndices(rec);
    }
    displayEvaluation();
    QApplication::restoreOverrideCursor();
}

void QFImFCCSFitEvaluationEditor::copyToInitial()
{
    if (!current) return;
    QFImFCCSFitEvaluationItem* eval=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!eval) return;

    eval->set_doEmitResultsChanged(false);
    eval->set_doEmitPropertiesChanged(false);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    for (int f=0; f<eval->getFitFileCount(); f++) {
        QFRawDataRecord* rec=eval->getFitFile(f);
        QFFitFunction* ffunc=eval->getFitFunction(f);

        double* params=eval->allocFillParameters(ffunc);


        for (int i=0; i<ffunc->paramCount(); i++) {
            QString id=ffunc->getParameterID(i);
            double value=eval->getFitValue(id, rec);
            double error=eval->getFitError(id, rec);
            bool fix=eval->getFitFix(id, rec);
            if (ffunc->isParameterVisible(i, params)) {
                eval->setInitFitFix(id, fix, rec);
                eval->setInitFitValue(id, value, error, rec);
                //qDebug()<<"set("<<rec->getID()<<") "<<id<<" = "<<value<<" +/- "<<error;
            };
        }
        free(params);
    }


    eval->set_doEmitResultsChanged(true);
    eval->set_doEmitPropertiesChanged(true);
    eval->emitResultsChanged();

    QApplication::restoreOverrideCursor();
}

void QFImFCCSFitEvaluationEditor::setParameterTableSpans()
{
    for (int i=1; i<ui->tableView->model()->columnCount(); i=i+5) {
        ui->tableView->setSpan(0,i,1,5);
        ui->tableView->setSpan(1,i,1,5);
    }
}

void QFImFCCSFitEvaluationEditor::on_cmbWeight_currentWeightChanged(QFFCSWeightingTools::DataWeight weight)
{
    if (!current) return;
    if (!current->getHighlightedRecord()) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QString pid=ui->cmbFitAlgorithm->currentFitAlgorithmID();
    current->getHighlightedRecord()->setQFProperty("algorithm", pid, false, false);
    QFImFCCSFitEvaluationItem* data=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    data->setFitDataWeighting(weight);
    QApplication::restoreOverrideCursor();
}

int QFImFCCSFitEvaluationEditor::getUserRangeMin(QFRawDataRecord *rec, int index)
{
    return 0;
}

int QFImFCCSFitEvaluationEditor::getUserMin(QFRawDataRecord *rec, int index, int defaultMin)
{
    QFImFCCSFitEvaluationItem* data=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!data) return defaultMin;
    const QString resultID=data->getEvaluationResultID(-1/*index*/, rec);

    // WORKROUND FOR OLD PROPERTY NAMES
    int defaultM=rec->getProperty(QString(resultID+"_datacut_min"), defaultMin).toInt();

    return rec->getProperty(resultID+"_datacut_min", defaultM).toInt();
}

int QFImFCCSFitEvaluationEditor::getUserMax(QFRawDataRecord *rec, int index, int defaultMax)
{
    QFImFCCSFitEvaluationItem* data=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!data) return defaultMax;
    const QString resultID=data->getEvaluationResultID(-1/*index*/, rec);

    // WORKROUND FOR OLD PROPERTY NAMES
    int defaultM=rec->getProperty(QString(resultID+"_datacut_max"), defaultMax).toInt();

    return rec->getProperty(resultID+"_datacut_max", defaultM).toInt();
    //return rec->getProperty(resultID+"_datacut_max", defaultMax).toInt();
}

void QFImFCCSFitEvaluationEditor::setUserMinMax(int userMin, int userMax)
{
    if (!current) return;
    QFImFCCSFitEvaluationItem* data=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if (!data) return;
    for (int i=0; i<data->getFitFileCount(); i++) {
        QFRawDataRecord* rdr=data->getFitFile(i);
        const QString resultID=data->getEvaluationResultID(-1, rdr);
        rdr->disableEmitPropertiesChanged();
        rdr->setQFProperty(resultID+"_datacut_min", userMin, false, false);
        rdr->setQFProperty(resultID+"_datacut_max", userMax, false, false);
        rdr->enableEmitPropertiesChanged(true);
    }
}

void QFImFCCSFitEvaluationEditor::slidersChanged(int userMin, int userMax, int min, int max)
{
    if (!current) return;
    setUserMinMax(userMin, userMax);
    displayData();
}

int QFImFCCSFitEvaluationEditor::getUserRangeMax(QFRawDataRecord *rec, int index)
{
    QFRDRFCSDataInterface* data=qobject_cast<QFRDRFCSDataInterface*>(rec);
    if (data) {
        return data->getCorrelationN()-1;
    }
    return 0;
}
void QFImFCCSFitEvaluationEditor::on_chkSaveStrings_toggled(bool checked)
{
    if (!current) return;
    current->setQFProperty("dontSaveFitResultMessage", !checked, false, false);
}


void QFImFCCSFitEvaluationEditor::createReportDoc(QTextDocument* document) {
    if (!current) return;
    QFRawDataRecord* record=current->getHighlightedRecord();
    // possibly to a qobject_cast<> to the data type/interface you are working with here: QFRDRMyInterface* data=qobject_cast<QFRDRMyInterface*>(record);
    QFImFCCSFitEvaluationItem* eval=qobject_cast<QFImFCCSFitEvaluationItem*>(current);
    if ((!eval)||(!record)/*||(!data)*/) return;


    // we use this QTextCursor to write the document
    QTextCursor cursor(document);

    // here we define some generic formats
    QTextCharFormat fText=cursor.charFormat();
    fText.setFontPointSize(8);
    QTextCharFormat fTextSmall=fText;
    fTextSmall.setFontPointSize(0.85*fText.fontPointSize());
    QTextCharFormat fTextBold=fText;
    fTextBold.setFontWeight(QFont::Bold);
    QTextCharFormat fTextBoldSmall=fTextBold;
    fTextBoldSmall.setFontPointSize(0.85*fText.fontPointSize());
    QTextCharFormat fHeading1=fText;
    QTextBlockFormat bfLeft;
    bfLeft.setAlignment(Qt::AlignLeft);
    QTextBlockFormat bfRight;
    bfRight.setAlignment(Qt::AlignRight);
    QTextBlockFormat bfCenter;
    bfCenter.setAlignment(Qt::AlignHCenter);

    fHeading1.setFontPointSize(2*fText.fontPointSize());
    fHeading1.setFontWeight(QFont::Bold);


    // insert heading
    cursor.insertText(tr("imFCCS Fit Report:\n\n"), fHeading1);
    cursor.movePosition(QTextCursor::End);

    // insert table with some data
    /*QTextTableFormat tableFormat;
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 98));
    QTextTable* table = cursor.insertTable(2, 2, tableFormat);
    table->cellAt(0, 0).firstCursorPosition().insertText(tr("raw data:"), fTextBold);
    table->cellAt(0, 1).firstCursorPosition().insertText(record->getName(), fText);
    table->cellAt(1, 0).firstCursorPosition().insertText(tr("ID:"), fTextBold);
    table->cellAt(1, 1).firstCursorPosition().insertText(QString::number(record->getID()));*/
    cursor.movePosition(QTextCursor::End);

}
