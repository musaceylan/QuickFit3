#include "qfimfccsrelativeccfdialog.h"
#include "ui_qfimfccsrelativeccfdialog.h"
#include "qfmatchrdrfunctor.h"
#include "qfimfccsfitevaluation_item.h"
#include "qfpluginservices.h"
#include <QtGui>
#include "qfimfccsfitevaluation_item.h"
#include "qffcstools.h"
#include "optionswidget.h"
#include "qfimfccsrelativeccfdialog.h"
#include "qfrdrimagetoruninterface.h"
#include <stdint.h>
#include "qfmathtools.h"
#include "programoptions.h"
#include "qftools.h"

QFImFCCSRelativeCCFDialog::QFImFCCSRelativeCCFDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QFImFCCSRelativeCCFDialog)
{

    plt=NULL;
    ui->setupUi(this);
    matchFunctor=new QFImFCCSMatchRDRFunctor();
    QFProject* project=QFPluginServices::getInstance()->getCurrentProject();
    QList<QPointer<QFRawDataRecord> > lst=matchFunctor->getFilteredList(project);
    ui->cmbACF->init(project, matchFunctor);
    ui->cmbCCF->init(project, matchFunctor);

    ui->widOverviewACF->setRDR(NULL);
    ui->widOverviewCCF->setRDR(NULL);


    bool okACF=false;
    bool okCCF=false;
    for (int i=0; i<lst.size(); i++) {
        if (lst[i]) {
            if (!okACF && isACF(lst[i])) {
                ui->cmbACF->setCurrentRDR(lst[i]);
                okACF=true;
            }
            if (!okCCF && isCCF(lst[i])) {
                ui->cmbCCF->setCurrentRDR(lst[i]);
                okCCF=true;
            }
        }
        if (okACF&&okCCF) break;
    }

    loadWidgetGeometry(*(ProgramOptions::getInstance()->getQSettings()), this, "ImFCSCalibrationWizard");
}

QFImFCCSRelativeCCFDialog::~QFImFCCSRelativeCCFDialog()
{
    saveWidgetGeometry(*(ProgramOptions::getInstance()->getQSettings()), this, "ImFCSCalibrationWizard");
    delete ui;
    delete matchFunctor;
}

QFRawDataRecord *QFImFCCSRelativeCCFDialog::getACF() const
{
    return ui->cmbACF->currentRDR();
}

QFRawDataRecord *QFImFCCSRelativeCCFDialog::getCCF() const
{
    return ui->cmbCCF->currentRDR();
}

bool QFImFCCSRelativeCCFDialog::calculateRelCCF(QFRawDataRecord *acf, QFRawDataRecord *ccf, double **rel_out, double **rel_error_out, int&w, int& h, int avgCount, bool showErrorMessage)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QFRDRFCSDataInterface* acfFCS=dynamic_cast<QFRDRFCSDataInterface*>(acf);
    QFRDRImageToRunInterface* acfRUNIMAGE=dynamic_cast<QFRDRImageToRunInterface*>(acf);
    QFRDRFCSDataInterface* ccfFCS=dynamic_cast<QFRDRFCSDataInterface*>(ccf);
    QFRDRImageToRunInterface* ccfRUNIMAGE=dynamic_cast<QFRDRImageToRunInterface*>(ccf);
    if (acfFCS && acfRUNIMAGE && ccfFCS && ccfRUNIMAGE) {
        if (   acfRUNIMAGE->getImageFromRunsWidth()>0 && acfRUNIMAGE->getImageFromRunsWidth()==ccfRUNIMAGE->getImageFromRunsWidth()
            && acfRUNIMAGE->getImageFromRunsHeight()>0 && acfRUNIMAGE->getImageFromRunsHeight()==ccfRUNIMAGE->getImageFromRunsHeight() )
        {
            w=acfRUNIMAGE->getImageFromRunsWidth();
            h=acfRUNIMAGE->getImageFromRunsHeight();

            double* acfD=acfFCS->getCorrelation();
            double* acfS=acfFCS->getCorrelationStdDev();
            int acfN=acfFCS->getCorrelationN();
            double* ccfD=ccfFCS->getCorrelation();
            double* ccfS=ccfFCS->getCorrelationStdDev();
            int ccfN=ccfFCS->getCorrelationN();

            if (acfD && acfS && ccfD && ccfS) {
                double* rel=(double*)malloc(w*h*sizeof(double));
                double* rel_error=(double*)malloc(w*h*sizeof(double));
                uint64_t idxA=0;
                uint64_t idxC=0;
                for (int i=0; i<w*h; i++) {

                    double D=ccfD[idxC];
                    double A=acfD[idxA];
                    double eD=ccfS[idxC];
                    double eA=acfS[idxA];

                    if (avgCount>1) {
                        int avgA=qMin(avgCount, acfN);
                        int avgC=qMin(avgCount, ccfN);
                        for (int j=1; j<avgA; j++) {
                            A=A+acfD[idxA+j];
                            eA=eA+acfS[idxA+j];
                        }
                        A=A/double(avgA);
                        eA=eA/double(avgA);
                        for (int j=1; j<avgC; j++) {
                            D=D+ccfD[idxC+j];
                            eD=eD+ccfS[idxC+j];
                        }
                        D=D/double(avgC);
                        eD=eD/double(avgC);
                    }

                    rel[i]=D/A;
                    rel_error[i]=0;
                    if (acfS && ccfS) rel_error[i]=sqrt(qfSqr(eA*D/qfSqr(A)) + qfSqr(eD/A));
                    if (!QFFloatIsOK(rel_error[i])) rel_error[i]=0;
                    idxA=idxA+acfN;
                    idxC=idxC+ccfN;
                    //qDebug()<<i<<A<<D<<rel[i]<<rel_error[i];
                }
                if (rel_out) *rel_out=rel;
                else free(rel);
                if (rel_error_out) *rel_error_out=rel_error;
                else free(rel_error);

                QApplication::restoreOverrideCursor();
                return true;
            } else {
                if (showErrorMessage) QMessageBox::critical(NULL, tr("Calculate Relative CCF"), tr("Can't calculate relative CCFs: Memory access error."));
                QApplication::restoreOverrideCursor();
                return false;
            }

        } else {
            if (showErrorMessage) QMessageBox::critical(NULL, tr("Calculate Relative CCF"), tr("Can't calculate relative CCFs: The two selected files contain images of different size."));
            QApplication::restoreOverrideCursor();
            return false;
        }

    } else {
        if (showErrorMessage) QMessageBox::critical(NULL, tr("Calculate Relative CCF"), tr("No files selected."));
        QApplication::restoreOverrideCursor();
        return false;
    }
    QApplication::restoreOverrideCursor();
    return true;
}

void QFImFCCSRelativeCCFDialog::on_cmbCCF_currentIndexChanged(int index)
{
    replotImages();
}

void QFImFCCSRelativeCCFDialog::on_cmbACF_currentIndexChanged(int /*index*/)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QFRawDataRecord* acf=ui->cmbACF->currentRDR();
    QFProject* project=QFPluginServices::getInstance()->getCurrentProject();
    if (acf && project) {
        QString arole=acf->getRole().toUpper();
        bool ok=true;
        if (!(arole.startsWith("ACF") || arole.isEmpty() || acf->getName().toLower().contains("acf"))) {
            ok=QMessageBox::warning(this, windowTitle(), tr("the ACF file you selected seems to be no ACF file (role is '%1').\nClicking IGNORE, this function will still try to find a fitting CCF file").arg(arole), QMessageBox::Ok|QMessageBox::Ignore, QMessageBox::Ignore)==QMessageBox::Ignore;
        }
        if (ok) {
            QString adir=acf->getFolder();
            QString ainput=acf->getFileForType("input");
            QFRawDataRecord* guess1=NULL;
            QFRawDataRecord* guess2=NULL;
            QFRawDataRecord* guess3=NULL;
            //QFRawDataRecord* guess3=NULL;
            QList<QPointer<QFRawDataRecord> > lst=matchFunctor->getFilteredList(project);
            for (int i=0; i<lst.size(); i++) {
                if (lst[i] && lst[i]!=acf) {
                    //QString crole=lst[i]->getRole().toUpper();
                    QString cdir=lst[i]->getFolder();
                    QString cinput=lst[i]->getFileForType("input");
                    if (isCCF(lst[i])) {
                        if (! (ainput.isEmpty() || cinput.isEmpty())) {
                            if ((QFileInfo(cinput)==QFileInfo(ainput))) {
                                guess2=lst[i];
                            }
                        }
                        if (! (adir.isEmpty() || cdir.isEmpty())) {
                            if (adir==cdir) {
                                guess3=lst[i];
                            }
                        }
                        if (acf->getGroup()==lst[i]->getGroup()) {
                            guess1=lst[i];
                        }
                    }
                }
            }
            if (guess1) ui->cmbCCF->setCurrentRDR(guess1);
            else if (guess2) ui->cmbCCF->setCurrentRDR(guess2);
            else if (guess3) ui->cmbCCF->setCurrentRDR(guess3);
            else qDebug()<<"no guess found";
        }
    }
    replotImages();
    QApplication::restoreOverrideCursor();
}


void QFImFCCSRelativeCCFDialog::replotImages()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ui->widOverviewACF->setRDR(ui->cmbACF->currentRDR());
    ui->widOverviewCCF->setRDR(ui->cmbCCF->currentRDR());

    ui->pltData->set_doDrawing(false);
    ui->pltData->clearGraphs();
    JKQTPdatastore* ds=ui->pltData->getDatastore();
    ds->clear();
    double* rel=NULL;
    double* rel_error=NULL;
    int w=0, h=0;
    if (calculateRelCCF(ui->cmbACF->currentRDR(), ui->cmbCCF->currentRDR(), &rel, &rel_error, w, h, ui->spinAvg->value(), false)) {
        size_t col=ds->addCopiedColumn(rel, w*h, tr("rel. CCF amplitude"));
        size_t cole=ds->addCopiedColumn(rel_error, w*h, tr("rel. CCF amplitude error"));
        plt=new JKQTPColumnMathImage(ui->pltData->get_plotter());
        plt->set_palette(JKQTPMathImage::MATLAB);
        plt->set_imageColumn(col);
        plt->set_Nx(w);
        plt->set_Ny(h);
        plt->set_x(0);
        plt->set_y(0);
        plt->set_width(w);
        plt->set_height(h);
        plt->set_showColorBar(true);
        plt->set_autoImageRange(false);
        plt->set_imageMin(0);
        plt->set_imageMax(1);
        bool showTop=w>h;
        plt->set_colorBarTopVisible(showTop);
        plt->set_colorBarRightVisible(!showTop);
        plt->set_title("");
        ui->pltData->get_plotter()->set_showKey(false);
        ui->pltData->get_plotter()->set_axisAspectRatio(double(w)/double(h));
        ui->pltData->get_plotter()->set_aspectRatio(double(w)/double(h));
        ui->pltData->get_plotter()->set_maintainAspectRatio(true);
        ui->pltData->get_plotter()->set_maintainAxisAspectRatio(true);
        ui->pltData->addGraph(plt);
        ui->pltData->setAbsoluteXY(0,w,0,h);
        ui->pltData->setXY(0,w,0,h);
    }
    if (rel) free(rel);
    if (rel_error) free(rel_error);
    //ui->pltData->zoomToFit();
    ui->pltData->set_doDrawing(true);
    ui->pltData->update_plot();
    QApplication::restoreOverrideCursor();
}

void QFImFCCSRelativeCCFDialog::addResult()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    double* rel;
    double* rel_error;
    int w=0, h=0;
    QFRawDataRecord* acf=getACF();
    QFRawDataRecord* ccf=getCCF();
    if (acf&&ccf&&calculateRelCCF(acf, ccf, &rel, &rel_error, w, h, ui->spinAvg->value(), true)) {

        QString evalName=QString("CalcRelCCF_ACF%1_CCF%2").arg(acf->getID()).arg(ccf->getID());
        QString group="results";
        QString egroup=evalName;
        QString acfName=acf->getRole();
        if (acfName.isEmpty()) acfName=acf->getName();
        QString ccfName=ccf->getRole();
        if (ccfName.isEmpty()) ccfName=ccf->getName();
        QString egrouplabel=tr("rel. CCF amlitude of CCF/ACF: '%2'/'%1'").arg(acfName).arg(ccfName);
        QString rn="relative_ccf_amplitude";

        ccf->resultsSetEvaluationGroup(evalName, egroup);
        ccf->resultsSetEvaluationGroupLabel(egroup, egrouplabel);
        ccf->resultsSetEvaluationGroupIndex(evalName, -1);
        ccf->resultsSetNumberErrorList(evalName, rn, rel, rel_error, w*h);
        ccf->resultsSetLabel(evalName, rn, tr("relative CCF amplitude"));
        ccf->resultsSetGroup(evalName, rn, group);
        ccf->resultsSetInteger(evalName, "acf_file_id", acf->getID());
        ccf->resultsSetString(evalName, "acf_file_role", acf->getRole());
        ccf->resultsSetString(evalName, "acf_file_name", acf->getName());
        ccf->resultsSetInteger(evalName, "ccf_file_id", ccf->getID());
        ccf->resultsSetString(evalName, "ccf_file_role", ccf->getRole());
        ccf->resultsSetString(evalName, "ccf_file_name", ccf->getName());



        if (rel) free(rel);
        if (rel_error) free(rel_error);
    }
    QApplication::restoreOverrideCursor();
}

void QFImFCCSRelativeCCFDialog::on_btnNextACF_clicked()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    nextACF();
    QApplication::restoreOverrideCursor();

}

void QFImFCCSRelativeCCFDialog::on_btnNextFileSameRole_clicked()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    nextFileSameRole();
    QApplication::restoreOverrideCursor();
}


bool QFImFCCSRelativeCCFDialog::isACF(const QFRawDataRecord *rec)
{
    if (!rec) return false;
    return rec->getRole().toUpper().startsWith("ACF") || rec->getRole().toUpper().startsWith("FCS") || (rec->getRole().isEmpty() && (rec->getName().toLower().contains("acf") || rec->getName().toLower().contains("fcs")) );
}

bool QFImFCCSRelativeCCFDialog::isCCF(const QFRawDataRecord *rec)
{
    if (!rec) return false;
    return rec->getRole().toUpper().contains("FCCS") || rec->getRole().toUpper().contains("DCCF")|| (rec->getRole().isEmpty() && (rec->getName().toLower().contains("fccs") || rec->getName().toLower().contains("dccf")) );
}


bool QFImFCCSRelativeCCFDialog::nextACF()
{
    QFProject* project=QFPluginServices::getInstance()->getCurrentProject();
    if (!(matchFunctor && project&&getACF()) ) return false;
    QList<QPointer<QFRawDataRecord> > lst=matchFunctor->getFilteredList(project);
    int current=lst.indexOf(getACF());
    if (current>=0 && current+1<lst.size()) {
        QFRawDataRecord* guess1=NULL;
        QFRawDataRecord* guess2=NULL;
        QFRawDataRecord* guess3=NULL;
        QString adir=getACF()->getFolder();
        QString ainput=getACF()->getFileForType("input");
        for (int i=current+1; i<lst.size(); i++) {
            if (isACF(lst[i])) {
                QString binput=lst[i]->getFileForType("input");
                QString bdir=getACF()->getFolder();
                if (getACF()->getGroup()==lst[i]->getGroup()) {
                    guess1=lst[i];
                    break;
                } else if (!(binput.isEmpty() || ainput.isEmpty()) && ((QFileInfo(binput)==QFileInfo(ainput)) )) {
                    if (!guess2) guess2=lst[i];
                } else if (adir.trimmed().toLower()==bdir.trimmed().toLower()) {
                    if (!guess2) guess2=lst[i];
                }
            }
        }
        if (guess1) { ui->cmbACF->setCurrentRDR(guess1); return true; }
        else if (guess2) { ui->cmbACF->setCurrentRDR(guess2); return true; }
        else if (guess3) { ui->cmbACF->setCurrentRDR(guess3); return true; }
    }
    return false;


}


bool QFImFCCSRelativeCCFDialog::nextFileSameRole()
{
    QFProject* project=QFPluginServices::getInstance()->getCurrentProject();
    if (!(matchFunctor && project&&getACF()) ) return false;
    QList<QPointer<QFRawDataRecord> > lst=matchFunctor->getFilteredList(project);
    int current=lst.indexOf(getACF());
    if (current>=0 && current+1<lst.size()) {
        QFRawDataRecord* guess1=NULL;
        QFRawDataRecord* guess2=NULL;
        QFRawDataRecord* guess3=NULL;
        QString adir=getACF()->getFolder();
        QString ainput=getACF()->getFileForType("input");
        for (int i=current+1; i<lst.size(); i++) {
            qDebug()<<"checking: "<<lst[i]->getName();
            if (isACF(lst[i]) && lst[i]->getRole().toUpper()==getACF()->getRole().toUpper()) {
                QString binput=lst[i]->getFileForType("input");
                QString bdir=getACF()->getFolder();
                if (getACF()->getGroup()==lst[i]->getGroup()) {
                    guess1=lst[i];
                    break;
                } else if (!(binput.isEmpty() || ainput.isEmpty()) && ((QFileInfo(binput)==QFileInfo(ainput)) )) {
                    if (!guess2) guess2=lst[i];
                } else if (adir.trimmed().toLower()==bdir.trimmed().toLower()) {
                    if (!guess2) guess2=lst[i];
                }
            }
        }
        if (guess1) { ui->cmbACF->setCurrentRDR(guess1); return true; }
        else if (guess2) { ui->cmbACF->setCurrentRDR(guess2); return true; }
        else if (guess3) { ui->cmbACF->setCurrentRDR(guess3); return true; }
    }
    return false;
}


void QFImFCCSRelativeCCFDialog::on_btnHelp_clicked()
{
    QFPluginServices::getInstance()->displayPluginHelpWindow("imfccs_fit", "calc_ccf_amplitude.html");
}






void QFImFCCSRelativeCCFDialog::on_btnStoreDataAll_clicked()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QModernProgressDialog progress(tr("calculating relative CCF amplitudes ..."), tr("&Cancel"), this);
    progress.show();
    QFRawDataRecord* rdr=ui->cmbACF->currentRDR();
    QString role="";
    QFProject* project=QFPluginServices::getInstance()->getCurrentProject();
    if (rdr&&project) {
        role=rdr->getRole().toUpper();
        QList<QPointer<QFRawDataRecord> > lst=matchFunctor->getFilteredList(project);
        for (int i=0; i<lst.size(); i++) {
            if (lst[i] && isACF(lst[i])) {
                ui->cmbACF->setCurrentRDR(lst[i]);
                break;
            }
        }

    }
    QApplication::processEvents();
    on_cmbACF_currentIndexChanged(ui->cmbACF->currentIndex());
    addResult();
    while (nextACF()) {
        on_cmbACF_currentIndexChanged(ui->cmbACF->currentIndex());
        //QApplication::processEvents();
        addResult();
        if (progress.wasCanceled()) break;
    }
    QApplication::restoreOverrideCursor();
}

void QFImFCCSRelativeCCFDialog::on_btnStoreDataAllSameRole_clicked()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QFProject* project=QFPluginServices::getInstance()->getCurrentProject();
    QModernProgressDialog progress(tr("calculating relative CCF amplitudes ..."), tr("&Cancel"), this);
    progress.show();
    QFRawDataRecord* rdr=ui->cmbACF->currentRDR();
    QString role="";
    if (project&&rdr) {
        role=rdr->getRole().toUpper();
        QList<QPointer<QFRawDataRecord> > lst=matchFunctor->getFilteredList(project);
        for (int i=0; i<lst.size(); i++) {
            if (lst[i] && lst[i]->getRole().toUpper()==role && isACF(lst[i])) {
                ui->cmbACF->setCurrentRDR(lst[i]);
                break;
            }
        }

    }
    QApplication::processEvents();
    on_cmbACF_currentIndexChanged(ui->cmbACF->currentIndex());
    addResult();
    while(nextFileSameRole()) {
        on_cmbACF_currentIndexChanged(ui->cmbACF->currentIndex());
        //QApplication::processEvents();
        addResult();
        if (progress.wasCanceled()) break;
    }
    QApplication::restoreOverrideCursor();
}