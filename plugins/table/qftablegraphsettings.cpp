/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>),
    German Cancer Research Center/University Heidelberg



    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License (LGPL) as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "qftablegraphsettings.h"
#include "ui_qftablegraphsettings.h"
#include "programoptions.h"
#include "qfrdrtable.h"
#include "qftools.h"
#include "qfdoubleedit.h"
#include "qfmathparserxfunctionlinegraph.h"
#include "qffitfunctionvalueinputdelegate.h"


QFTableGraphSettings::QFTableGraphSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QFTableGraphSettings)
{
    m_connected=false;
    headerModel=new QFTableModelColumnHeaderModel(NULL, this);
    //headerModel=new QStringListModel(this);
    functionRef=new QFFunctionReferenceTool(NULL);
    functionRef->setCompleterFile(ProgramOptions::getInstance()->getConfigFileDirectory()+"/completers/table/table_expression.txt");
    functionRef->setDefaultWordsMathExpression();

    actFit=new QFActionWithNoMenuRole(QIcon(":/table/fit.png"), tr("least squares fit"), this);
    connect(actFit, SIGNAL(triggered()), this, SLOT(doFit()));
    actRegression=new QFActionWithNoMenuRole(QIcon(":/table/regression.png"), tr("regression analysis"), this);
    connect(actRegression, SIGNAL(triggered()), this, SLOT(doRegression()));




    updating=true;

    ui->setupUi(this);

    ui->btnRefit->setVisible(true);

    ui->tabFunctionParameters->setItemDelegate(new QFFitFunctionValueInputDelegate(ui->tabFunctionParameters));
    fitfuncValuesTable=new QFFitFunctionValueInputTable(this);
    fitfuncValuesTable->setEditErrors(false);
    fitfuncValuesTable->setEditFix(false);
    fitfuncValuesTable->setEditRanges(false);
    ui->tabFunctionParameters->setModel(fitfuncValuesTable);

    ui->btnFit->addAction(actRegression);
    ui->btnFit->addAction(actFit);
    ui->btnFit->setDefaultAction(actFit);
    ui->btnFit->setVisible(false);
    functionRef->registerEditor(ui->edtFunction);
    ui->edtImageHeight->setCheckBounds(true, false);
    ui->edtImageHeight->setRange(0, 1e10);
    ui->edtImageMax->setCheckBounds(false, false);
    ui->edtImageMin->setCheckBounds(false, false);
    ui->edtImageWidth->setCheckBounds(true, false);
    ui->edtImageWidth->setRange(0, 1e10);
    ui->edtImageX->setCheckBounds(true, false);
    ui->edtImageY->setCheckBounds(true, false);
    ui->cmbGraphType->setCurrentIndex(0);
    ui->edtSelectDataValue->setCheckBounds(false, false);
    ui->edtSelectDataValue2->setCheckBounds(false, false);




    ui->cmbGraphType->clear();
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_pointslines.png"), tr("lines/scatter plot"), QFRDRTable::gtLines);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_pscatter.png"), tr("parametr. scatter plot"), QFRDRTable::gtParametrizedScatter);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_vimpulses.png"), tr("vertical impulses"), QFRDRTable::gtImpulsesVertical);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_himpulses.png"), tr("horizontal impulses"), QFRDRTable::gtImpulsesHorizontal);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_xfilledcurve.png"), tr("illed curve X"), QFRDRTable::gtFilledCurveX);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_yfilledcurve.png"), tr("filles curve Y"), QFRDRTable::gtFilledCurveY);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_vsteps.png"), tr("vertical steps"), QFRDRTable::gtStepsVertical);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_hsteps.png"), tr("horizontal steps"), QFRDRTable::gtStepsHorizontal);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_vbars.png"), tr("vertical bars"), QFRDRTable::gtBarsVertical);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_hbars.png"), tr("horizontal bars"), QFRDRTable::gtBarsHorizontal);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_boxplotsx.png"), tr("boxplot X"), QFRDRTable::gtBoxplotX);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_boxplotsy.png"), tr("boxplot Y"), QFRDRTable::gtBoxplotY);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_image.png"), tr("image"), QFRDRTable::gtImage);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_rgbimage.png"), tr("RGB image"), QFRDRTable::gtRGBImage);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_maskimage.png"), tr("mask image"), QFRDRTable::gtMaskImage);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_function.png"), tr("function (lines)"), QFRDRTable::gtFunction);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_hrange.png"), tr("horizontal range"), QFRDRTable::gtHorizontalRange);
    ui->cmbGraphType->addItem(QIcon(":/table/icons/plot_vrange.png"), tr("vertical range"), QFRDRTable::gtVerticalRange);



    current=NULL;
    this->plot=-1;
    updating=false;
    if (ProgramOptions::getInstance() && ProgramOptions::getInstance()->getQSettings()) readSettings(*(ProgramOptions::getInstance()->getQSettings()), "table/QFRDRTablePlotWidget/");
    updatePlotWidgetVisibility();

}

QFTableGraphSettings::~QFTableGraphSettings()
{
    updating=true;
    if (ProgramOptions::getInstance() && ProgramOptions::getInstance()->getQSettings()) writeSettings(*(ProgramOptions::getInstance()->getQSettings()), "table/QFRDRTablePlotWidget/");
    delete ui;
}

void QFTableGraphSettings::setRecord(QFRDRTable *record, int plot)
{
    if (current) {
        disconnect(current->model(), SIGNAL(modelReset()), this, SLOT(updateComboboxes()));
        disconnect(current->model(), SIGNAL(layoutChanged()), this, SLOT(updateComboboxes()));
        disconnect(current->model(), SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this, SLOT(updateComboboxes()));
    }
    current=record;
    this->plot=plot;
    updating=true;
    disconnectWidgets();


    headerModel->setHasNone(true);
    headerModel->setModel(current->model());

    if (current) {
        connect(current->model(), SIGNAL(modelReset()), this, SLOT(updateComboboxes()));
        connect(current->model(), SIGNAL(layoutChanged()), this, SLOT(updateComboboxes()));
        connect(current->model(), SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this, SLOT(updateComboboxes()));
    }
    updateComboboxes();
    updating=false;
    connectWidgets();

    //updateGraph();
}

void QFTableGraphSettings::updateComboboxes()
{
    reloadColumns(ui->cmbLinesXData);
    reloadColumns(ui->cmbLinesXError);
    reloadColumns(ui->cmbLinesXError2);
    reloadColumns(ui->cmbLinesYData);
    reloadColumns(ui->cmbLinesYError);
    reloadColumns(ui->cmbLinesYError2);
    reloadColumns(ui->cmbLinesMax);
    reloadColumns(ui->cmbLinesMean);
    reloadColumns(ui->cmbLinesQ75);
    reloadColumns(ui->cmbSelectDataColumn);
    reloadColumns(ui->cmbSelectDataColumn_2);
    reloadColumns(ui->cmbSelectDataColumn_3);
}

void QFTableGraphSettings::fitFunctionChanged()
{
    cmbFunctionTypeCurrentIndexChanged(ui->cmbFunctionType->currentIndex());
}

void QFTableGraphSettings::rawDataChanged()
{
    if (updating) return;
    //disconnectWidgets();
    /*reloadColumns(ui->cmbLinesXData);
    reloadColumns(ui->cmbLinesXError);
    reloadColumns(ui->cmbLinesYData);
    reloadColumns(ui->cmbLinesYError);*/

    //connectWidgets();
    //updateGraph();
    cmbFunctionTypeCurrentIndexChanged(ui->cmbFunctionType->currentIndex());
}

void QFTableGraphSettings::readSettings(QSettings &/*settings*/, const QString &/*prefix*/)
{
}

void QFTableGraphSettings::writeSettings(QSettings &/*settings*/, const QString &/*prefix*/)
{
}

QIcon QFTableGraphSettings::getGraphIcon(int i) const
{
    return ui->cmbGraphType->itemIcon(i);
}



void QFTableGraphSettings::writeGraphData()
{
    //QWidget* wid=focusWidget();
    emit graphDataChanged();
    //QApplication::processEvents();
    //QApplication::processEvents();
    QApplication::processEvents();
    //if (wid) qDebug()<<wid<<wid->isVisible();
    //if (wid) wid->setFocus();
}

void QFTableGraphSettings::reloadColumns(QComboBox *combo)
{
    bool updt=updating;
    updating=true;
    int idx=combo->currentIndex();
    if (current) {
        current->loadColumnToComboBox(combo);
    } else {
        combo->clear();
        combo->addItem(tr("--- none ---"));
        idx=0;
    }
    updating=updt;
    combo->setCurrentIndex(idx);
}

void QFTableGraphSettings::writeGraphData(QFRDRTable::GraphInfo& graph)
{
    //qDebug()<<"writeGraphData()";
    //qDebug()<<"QFTableGraphSettings::graphDataChanged    updating="<<updating;
    if (updating) return;
    if (current) {
        updating=true;
        if (this->plot<0 || this->plot>=current->getPlotCount()) return;

        graph.title=ui->edtGraphTitle->text();
        graph.titleShow=ui->chkShowTitle->isChecked();

        graph.type=(QFRDRTable::GraphType)ui->cmbGraphType->itemData(ui->cmbGraphType->currentIndex()).toInt();

        updatePlotWidgetVisibility();



        graph.xcolumn=qMax(-2, ui->cmbLinesXData->currentData().toInt());
        graph.xerrorcolumn=qMax(-2, ui->cmbLinesXError->currentData().toInt());
        graph.xerrorcolumnlower=qMax(-2, ui->cmbLinesXError2->currentData().toInt());
        graph.ycolumn=qMax(-2, ui->cmbLinesYData->currentData().toInt());
        graph.yerrorcolumn=qMax(-2, ui->cmbLinesYError->currentData().toInt());
        graph.yerrorcolumnlower=qMax(-2, ui->cmbLinesYError2->currentData().toInt());
        graph.meancolumn=qMax(-2, ui->cmbLinesMean->currentData().toInt());
        graph.maxcolumn=qMax(-2, ui->cmbLinesMax->currentData().toInt());
        graph.q75column=qMax(-2, ui->cmbLinesQ75->currentData().toInt());

        graph.xerrorsymmetric=ui->chkXErrSym->isChecked();
        graph.yerrorsymmetric=ui->chkYErrSym->isChecked();

        graph.width=ui->edtWidth->value()/100.0;
        graph.shift=ui->edtShift->value()/100.0;
        QColor oldColor=graph.color;
        QColor oldRangeColor=graph.rangeCenterColor;
        QColor oldDefaultErrorColor=oldColor.darker();
        QColor oldDefaultFillColor=oldColor.lighter();
        graph.color=ui->cmbLineColor->currentColor();

        graph.centerColorAuto=ui->chkCenterColAuto->isChecked();
        graph.fillColorAuto=ui->chkFillColAuto->isChecked();
        graph.errorColorAuto=ui->chkErrorColAuto->isChecked();




        if (!ui->chkErrorColAuto->isChecked()) {
            graph.errorColor=ui->cmbErrorColor->currentColor();
        } else {
            graph.errorColor=graph.color.darker();
            ui->cmbErrorColor->setCurrentColor(graph.errorColor);
        }

        if (!ui->chkFillColAuto->isChecked()) {
            graph.fillColor=ui->cmbFillColor->currentColor();
        } else {
            graph.fillColor=graph.color.lighter();
            ui->cmbFillColor->setCurrentColor(graph.fillColor);
        }

        if (!ui->chkCenterColAuto->isChecked()) {
            graph.rangeCenterColor=ui->cmbRangeCenterColor->currentColor();
        } else {
            graph.rangeCenterColor=graph.color.lighter();
            ui->cmbRangeCenterColor->setCurrentColor(graph.rangeCenterColor);
        }
        graph.errorStyle=ui->cmbErrorStyle->getErrorStyle();
        graph.symbol=ui->cmbSymbol->getSymbol();
        graph.style=ui->cmbLineStyle->currentLineStyle();
        graph.whiskerStyle=ui->cmbWhiskerStyle->currentLineStyle();
        graph.fillStyle=ui->cmbFillStyle->currentFillStyle();
        graph.linewidth=ui->spinLineWidth->value();
        graph.symbolSize=ui->spinSymbolSize->value();
        graph.symbolLineWidth=ui->spinSymbolLineWidth->value();
        graph.drawLine=ui->chkDrawLine->isChecked();
        graph.colorTransparent=double(ui->sliderPlotTransparent->value())/255.0;
        graph.errorColorTransparent=double(ui->sliderErrorTransparent->value())/255.0;
        graph.fillColorTransparent=double(ui->sliderFillTransparent->value())/255.0;
        graph.imageFalseTransparent=double(ui->sliderImageFalseColor->value())/255.0;
        graph.imageTrueTransparent=double(ui->sliderImageTrueColor->value())/255.0;
        graph.imageFalseColor=ui->cmbImageFalseColor->currentColor();
        graph.imageTrueColor=ui->cmbImageTrueColor->currentColor();
        graph.imageHeight=ui->edtImageHeight->value();
        graph.imageWidth=ui->edtImageWidth->value();
        graph.imageMax=ui->edtImageMax->value();
        graph.imageMin=ui->edtImageMin->value();
        graph.imageX=ui->edtImageX->value();
        graph.imageY=ui->edtImageY->value();
        graph.imagePixelWidth=ui->spinImageWidth->value();
        graph.imagePalette=ui->cmbColormap->colorPalette();
        graph.imageAutoRange=ui->chkImageAutorange->isChecked();
        graph.imageColorbarRight=ui->chkImageColorbarRight->isChecked();
        graph.imageColorbarTop=ui->chkImageColorbarTop->isChecked();
        graph.imageLegend=ui->edtColorbarLabel->text();
        graph.imageLegendG=ui->edtColorbarLabelG->text();
        graph.imageLegendB=ui->edtColorbarLabelB->text();
        graph.imageLegendMod=ui->edtColorbarLabelMod->text();
        graph.dataSortOrder=ui->cmbSort->currentIndex();



        graph.imageColorbarTicklength=ui->spinColorbarTickLength->value();
        graph.imageColorbarFontsize=ui->spinColorbarFontsize->value();
        graph.imageTicks=ui->spinColorbarTicks->value();
        graph.imageModTicks=ui->spinColorbarModTicks->value();
        graph.imageColorbarLabelType=ui->cmbColorbarLabelType->getLabelType();
        graph.imageColorbarLabelDigits=ui->spinColorbarLabelDigits->value();

        graph.colorbarWidth=ui->spinColorbarWidth->value();
        graph.colorbarRelativeHeight=ui->spinColorbarHeight->value()/100.0;
        graph.function=ui->edtFunction->text();
        graph.isStrided=ui->chkSTrided->isChecked();
        graph.stride=ui->spinStride->value();
        graph.strideStart=ui->spinStrideStart->value();
        graph.modifierMode=(JKQTPMathImage::ModifierMode)ui->cmbModifierMode->currentIndex();

        graph.isDataSelect=ui->chkSelectData->isChecked();
        graph.dataSelect1Column=qMax(-2, ui->cmbSelectDataColumn->currentData().toInt());
        graph.dataSelect1Operation=(QFRDRTable::DataSelectOperation)ui->cmbSelectDataCompare->currentIndex();
        graph.dataSelect1CompareValue=ui->edtSelectDataValue->value();
        graph.dataSelect1CompareValue2=ui->edtSelectDataValue2->value();

        graph.dataSelect2Column=qMax(-2, ui->cmbSelectDataColumn_2->currentData().toInt());
        graph.dataSelect2Operation=(QFRDRTable::DataSelectOperation)ui->cmbSelectDataCompare_2->currentIndex();
        graph.dataSelect2CompareValue=ui->edtSelectDataValue_2->value();
        graph.dataSelect2CompareValue2=ui->edtSelectDataValue2_2->value();

        graph.dataSelect3Column=qMax(-2, ui->cmbSelectDataColumn_3->currentData().toInt());
        graph.dataSelect3Operation=(QFRDRTable::DataSelectOperation)ui->cmbSelectDataCompare_3->currentIndex();
        graph.dataSelect3CompareValue=ui->edtSelectDataValue_3->value();
        graph.dataSelect3CompareValue2=ui->edtSelectDataValue2_3->value();

        graph.dataSelectLogic12=(QFRDRTable::DataSelectLogicOperation)ui->cmbSelectDataLogic12->currentIndex();
        graph.dataSelectLogic23=(QFRDRTable::DataSelectLogicOperation)ui->cmbSelectDataLogic23->currentIndex();

        graph.offset=ui->edtOffset->value();

        graph.functionType=(QFRDRTable::GTFunctionType)ui->cmbFunctionType->currentIndex();
        if (graph.functionType==QFRDRTable::gtfQFFunction) {
            graph.function=ui->cmbQFFitFunction->currentFitFunctionID();
            graph.subfunction=ui->cmbQFFitFunctionSubplot->currentIndex()-1;
            graph.showallsubfunctions=ui->chkQFFitFunctionSubplotAll->isChecked();
        }
        graph.functionParameters=fitfuncValues;


        graph.rangeCenter=ui->edtRangeCenter->value();
        graph.rangeStart=ui->edtRangeStart->value();
        graph.rangeEnd=ui->edtRangeEnd->value();
        graph.rangeCenterWidth=ui->spinRangeCenterWidth->value();
        graph.rangeMode=(QFRDRTable::RangeGraphMode)ui->cmbRangeMode->currentIndex();
        graph.rangeCenterColor=ui->cmbRangeCenterColor->currentColor();
        graph.rangeCenterColorTransparent=double(ui->sliderRangeCenterTransparency->value())/255.0;
        graph.rangeCenterStyle=ui->cmbRangeCenterStyle->currentLineStyle();
        graph.rangeFill=ui->chkRangeFillRange->isChecked();
        graph.rangeInverted=ui->chkRangeInverted->isChecked();
        graph.rangeDrawCenter=ui->chkRangeDrawCenter->isChecked();


        graph.errorLineStyle=ui->cmbErrorLineStyle->currentLineStyle();
        graph.errorBarSize=ui->spinErrorBarWidth->value();
        graph.errorWidth=ui->spinErrorLineWidth->value();



        updating=false;
    }
}

void QFTableGraphSettings::loadGraphData(const QFRDRTable::GraphInfo &graph)
{
    //qDebug()<<"loadGraphData()";
    updating=true;
    ui->edtGraphTitle->setText(graph.title);
    ui->chkShowTitle->setChecked(graph.titleShow);

    QString fit_type=graph.moreProperties.value("FIT_TYPE", "NONE").toString().toUpper().trimmed();
    isFitResult=((fit_type=="LEAST_SQUARES")||(fit_type=="FIT")||(fit_type=="REGRESSION"));

    ui->cmbLinesXData->setCurrentData(graph.xcolumn);
    ui->cmbLinesXError->setCurrentData(graph.xerrorcolumn);
    ui->cmbLinesXError2->setCurrentData(graph.xerrorcolumnlower);
    ui->cmbLinesYData->setCurrentData(graph.ycolumn);
    ui->cmbLinesYError->setCurrentData(graph.yerrorcolumn);
    ui->cmbLinesYError2->setCurrentData(graph.yerrorcolumnlower);
    ui->cmbLinesMax->setCurrentData(graph.maxcolumn);
    ui->cmbLinesMean->setCurrentData(graph.meancolumn);
    ui->cmbLinesQ75->setCurrentData(graph.q75column);

    ui->chkXErrSym->setChecked(graph.xerrorsymmetric);
    ui->chkYErrSym->setChecked(graph.yerrorsymmetric);

    ui->cmbGraphType->setCurrentIndex(ui->cmbGraphType->findData((int)graph.type));

    ui->edtWidth->setValue(graph.width*100.0);
    ui->edtShift->setValue(graph.shift*100.0);
    ui->edtOffset->setValue(graph.offset);

    ui->cmbSort->setCurrentIndex(graph.dataSortOrder);


    ui->cmbErrorColor->setCurrentColor(graph.errorColor);
    ui->cmbLineColor->setCurrentColor(graph.color);
    ui->cmbFillColor->setCurrentColor(graph.fillColor);
    ui->cmbErrorStyle->setSymbol(graph.errorStyle);
    ui->cmbSymbol->setSymbol(graph.symbol);
    ui->cmbLineStyle->setCurrentLineStyle(graph.style);
    ui->cmbFillStyle->setCurrentFillStyle(graph.fillStyle);
    ui->spinLineWidth->setValue(graph.linewidth);
    ui->spinSymbolSize->setValue(graph.symbolSize);
    ui->spinSymbolLineWidth->setValue(graph.symbolLineWidth);
    ui->chkDrawLine->setChecked(graph.drawLine);
    ui->sliderPlotTransparent->setValue(graph.colorTransparent*255.0);
    ui->spinPlotTransparent->setValue(ui->sliderPlotTransparent->value());

    ui->sliderErrorTransparent->setValue(graph.errorColorTransparent*255.0);
    ui->spinErrorTransparent->setValue(ui->sliderErrorTransparent->value());
    ui->sliderFillTransparent->setValue(graph.fillColorTransparent*255.0);
    ui->spinFillTransparent->setValue(ui->sliderFillTransparent->value());
    ui->cmbImageFalseColor->setCurrentColor(graph.imageFalseColor);
    ui->sliderImageFalseColor->setValue(graph.imageFalseTransparent*255.0);
    ui->spinImageFalseColor->setValue(ui->sliderImageFalseColor->value());
    ui->cmbImageTrueColor->setCurrentColor(graph.imageTrueColor);
    ui->sliderImageTrueColor->setValue(graph.imageTrueTransparent*255.0);
    ui->spinImageTrueColor->setValue(ui->sliderImageTrueColor->value());
    ui->edtImageHeight->setValue(graph.imageHeight);
    ui->edtImageWidth->setValue(graph.imageWidth);
    ui->edtImageMax->setValue(graph.imageMax);
    ui->edtImageMin->setValue(graph.imageMin);
    ui->edtImageX->setValue(graph.imageX);
    ui->edtImageY->setValue(graph.imageY);
    ui->spinImageWidth->setValue(graph.imagePixelWidth);
    ui->cmbColormap->setColorPalette(graph.imagePalette);
    ui->chkImageAutorange->setChecked(graph.imageAutoRange);
    ui->cmbWhiskerStyle->setCurrentLineStyle(graph.whiskerStyle);

    ui->cmbColorbarLabelType->setLabelType(graph.imageColorbarLabelType);
    ui->spinColorbarLabelDigits->setValue(graph.imageColorbarLabelDigits);

    ui->spinColorbarTickLength->setValue(graph.imageColorbarTicklength);
    ui->spinColorbarFontsize->setValue(graph.imageColorbarFontsize);
    ui->spinColorbarTicks->setValue(graph.imageTicks);
    ui->spinColorbarModTicks->setValue(graph.imageModTicks);

    ui->chkImageColorbarRight->setChecked(graph.imageColorbarRight);
    ui->chkImageColorbarTop->setChecked(graph.imageColorbarTop);
    ui->edtColorbarLabel->setText(graph.imageLegend);
    ui->edtColorbarLabelG->setText(graph.imageLegendG);
    ui->edtColorbarLabelB->setText(graph.imageLegendB);
    ui->edtColorbarLabelMod->setText(graph.imageLegendMod);
    ui->spinColorbarWidth->setValue(graph.colorbarWidth);
    ui->spinColorbarHeight->setValue(graph.colorbarRelativeHeight*100.0);
    ui->edtFunction->setText(graph.function);
    ui->chkSTrided->setChecked(graph.isStrided);
    ui->spinStride->setValue(graph.stride);
    ui->spinStrideStart->setValue(graph.strideStart);
    ui->cmbModifierMode->setCurrentIndex(graph.modifierMode);


    ui->chkSelectData->setChecked(graph.isDataSelect);
    ui->cmbSelectDataColumn->setCurrentData(graph.dataSelect1Column);
    ui->cmbSelectDataCompare->setCurrentIndex(graph.dataSelect1Operation);
    ui->edtSelectDataValue->setValue(graph.dataSelect1CompareValue);
    ui->edtSelectDataValue2->setValue(graph.dataSelect1CompareValue2);

    ui->cmbSelectDataColumn_2->setCurrentData(graph.dataSelect2Column);
    ui->cmbSelectDataCompare_2->setCurrentIndex(graph.dataSelect2Operation);
    ui->edtSelectDataValue_2->setValue(graph.dataSelect2CompareValue);
    ui->edtSelectDataValue2_2->setValue(graph.dataSelect2CompareValue2);

    ui->cmbSelectDataColumn_3->setCurrentData(graph.dataSelect3Column);
    ui->cmbSelectDataCompare_3->setCurrentIndex(graph.dataSelect3Operation);
    ui->edtSelectDataValue_3->setValue(graph.dataSelect3CompareValue);
    ui->edtSelectDataValue2_3->setValue(graph.dataSelect3CompareValue2);

    ui->cmbSelectDataLogic12->setCurrentIndex(graph.dataSelectLogic12);
    ui->cmbSelectDataLogic23->setCurrentIndex(graph.dataSelectLogic23);

    ui->cmbFunctionType->setCurrentIndex((int)graph.functionType);
    if (graph.functionType==QFRDRTable::gtfQFFunction) {
        ui->cmbQFFitFunction->setCurrentFitFunction(graph.function);
        ui->cmbQFFitFunctionSubplot->setCurrentIndex(graph.subfunction+1);
        ui->chkQFFitFunctionSubplotAll->setChecked(graph.showallsubfunctions);
    }
    fitfuncValues=graph.functionParameters;
    fitfuncErrors.clear();
    if (graph.moreProperties.contains("FIT_ERRORPARAMS")) {
        QList<QVariant> el=graph.moreProperties["FIT_ERRORPARAMS"].toList();
        //qDebug()<<"FIT_ERRORPARAMS="<<graph.moreProperties["FIT_ERRORPARAMS"];
        //qDebug()<<"FIT_ERRORPARAMS.toList="<<el;
        //qDebug()<<"fitfuncValues="<<fitfuncValues;
        //qDebug()<<"FIT_ERRORPARAMS.size="<<el.size()<<"   fitfuncValues.size()="<<fitfuncValues.size();
        if (el.size()==fitfuncValues.size()) {
            for (int i=0; i<fitfuncValues.size(); i++) {
                bool ok=false;
                fitfuncErrors.append(el[i].toDouble(&ok));
                if (!ok) {
                    fitfuncErrors.clear();
                    //qDebug()<<"   !!! NOT OK AT "<<i;
                    break;
                }
            }
            //qDebug()<<"fitfuncErrors="<<fitfuncErrors;
        }
    }
    cmbFunctionTypeCurrentIndexChanged(ui->cmbFunctionType->currentIndex());
    ui->tabFunctionParameters->resizeColumnsToContents();
    ui->tabFunctionParameters->resizeRowsToContents();


    ui->edtRangeCenter->setValue(graph.rangeCenter);
    ui->edtRangeStart->setValue(graph.rangeStart);
    ui->edtRangeEnd->setValue(graph.rangeEnd);
    ui->spinRangeCenterWidth->setValue(graph.rangeCenterWidth);
    ui->cmbRangeMode->setCurrentIndex((int)graph.rangeMode);
    ui->cmbRangeCenterColor->setCurrentColor(graph.rangeCenterColor);
    ui->sliderRangeCenterTransparency->setValue(graph.rangeCenterColorTransparent*255.0);
    ui->spinRangeCenterTransparency->setValue(ui->sliderRangeCenterTransparency->value());
    ui->cmbRangeCenterStyle->setCurrentLineStyle(graph.rangeCenterStyle);
    ui->chkRangeFillRange->setChecked(graph.rangeFill);
    ui->chkRangeInverted->setChecked(graph.rangeInverted);
    ui->chkRangeDrawCenter->setChecked(graph.rangeDrawCenter);

    ui->cmbErrorLineStyle->setCurrentLineStyle(graph.errorLineStyle);
    ui->spinErrorBarWidth->setValue(graph.errorBarSize);
    ui->spinErrorLineWidth->setValue(graph.errorWidth);

    ui->chkCenterColAuto->setChecked(graph.centerColorAuto);
    ui->chkFillColAuto->setChecked(graph.fillColorAuto);
    ui->chkErrorColAuto->setChecked(graph.errorColorAuto);


    updateSelectDataEnabled();
    updating=false;
}

void QFTableGraphSettings::updatePlotWidgetVisibility()
{
    if (current) {
        QWidget* widFocus=focusWidget();
        if (this->plot<0 || this->plot>=current->getPlotCount()) return;


        bool updt=updatesEnabled();
        if (isVisible()) setUpdatesEnabled(false);

        updateSelectDataEnabled();

        this->setVisible(true);

        on_cmbRangeMode_currentIndexChanged(ui->cmbRangeMode->currentIndex());

        ui->widErrorStyle->setVisible(true);
        ui->widFillColor->setVisible(true);
        ui->widImage->setVisible(true);
        ui->widLineStyle->setVisible(true);
        ui->widSymbol->setVisible(true);
        ui->labErrorStyle->setVisible(true);
        ui->labDataX->setVisible(true);
        ui->labDataY->setVisible(true);
        ui->labErrorX->setVisible(true);
        ui->labErrorY->setVisible(true);
        ui->labFillColor->setVisible(true);
        ui->labImage->setVisible(true);
        ui->labLinestyle->setVisible(true);
        ui->labSymbol->setVisible(true);
        ui->labTitle->setVisible(true);
        ui->labType->setVisible(true);
        ui->chkDrawLine->setVisible(true);
        ui->cmbLineStyle->setVisible(true);
        ui->cmbLinesXData->setVisible(true);
        ui->btnAutoX->setVisible(true);
        ui->widErrorX->setVisible(true);
        ui->widErrorX2->setVisible(true);
        ui->cmbLinesYData->setVisible(true);
        ui->btnAutoY->setVisible(true);
        ui->widErrorY->setVisible(true);
        ui->widErrorY2->setVisible(true);
        ui->widFunction->setVisible(false);
        ui->labFuction->setVisible(false);
        ui->chkSTrided->setVisible(true);
        ui->widStride->setVisible(true);
        ui->chkSelectData->setVisible(true);
        ui->widDataSelect->setVisible(true);
        ui->cmbWhiskerStyle->setVisible(false);
        ui->labWhisker->setVisible(false);

        ui->btnClearLinesXData->setVisible(true);
        ui->widErrorX->setVisible(true);
        ui->btnClearLinesYData->setVisible(true);
        ui->widErrorY->setVisible(true);
        ui->btnClearLinesMax->setVisible(false);
        ui->btnClearLinesMean->setVisible(false);
        ui->btnClearLinesQ75->setVisible(false);
        ui->labSort->setVisible(true);
        ui->cmbSort->setVisible(true);

        ui->labDataX->setText(tr("X data col.:"));
        ui->labDataY->setText(tr("Y data col.:"));
        ui->labErrorX->setText(tr("X error col.:"));
        ui->labErrorY->setText(tr("Y error col.:"));
        ui->labMax->setVisible(false);
        ui->labMean->setVisible(false);
        ui->labQ75->setVisible(false);
        ui->labMax->setText(tr("max col.:"));
        ui->labMean->setText(tr("mean col.:"));
        ui->labQ75->setText(tr("Q75% col.:"));
        ui->labImage->setText(tr("image map:"));

        ui->cmbLinesMax->setVisible(false);
        ui->cmbLinesMean->setVisible(false);
        ui->cmbLinesQ75->setVisible(false);

        ui->labTransparencyFalse->setVisible(false);
        ui->widFalseTransparency->setVisible(false);
        ui->labTransparencyTrue->setVisible(false);
        ui->widTrueTransparency->setVisible(false);
        ui->labTrueColor->setVisible(false);
        ui->cmbImageTrueColor->setVisible(false);
        ui->labFalseColor->setVisible(false);
        ui->cmbImageFalseColor->setVisible(false);
        ui->labColormap->setVisible(false);
        ui->cmbColormap->setVisible(false);
        ui->labColorbarLabel->setVisible(false);
        ui->edtColorbarLabel->setVisible(false);
        ui->labModifierMode->setVisible(false);
        ui->cmbModifierMode->setVisible(false);
        ui->labColorbarLabelMod->setVisible(false);
        ui->edtColorbarLabelMod->setVisible(false);
        ui->labColorbarLabelG->setVisible(false);
        ui->edtColorbarLabelG->setVisible(false);
        ui->labColorbarLabelB->setVisible(false);
        ui->edtColorbarLabelB->setVisible(false);
        ui->labColorbarLabel->setText(tr("bar label:"));
        ui->btnFit->setVisible(true);
        ui->labImageHeight->setVisible(true);
        ui->labImageWidth->setVisible(true);
        ui->labImageWidthPixels->setVisible(true);
        ui->spinImageWidth->setVisible(true);
        ui->edtImageWidth->setVisible(true);
        ui->edtImageHeight->setVisible(true);
        ui->edtImageX->setVisible(true);
        ui->edtImageY->setVisible(true);
        ui->labImageX->setVisible(true);
        ui->labImageY->setVisible(true);
        ui->btnAutoImageSizes->setVisible(true);

        ui->labQFFitFuction->setVisible(false);
        ui->labQFFitFuction2->setVisible(false);
        ui->widQFFitFunctionSubplot->setVisible(false);
        ui->labFuctionType->setVisible(false);
        ui->labFuctionParameters->setVisible(false);
        ui->cmbFunctionType->setVisible(false);
        ui->cmbQFFitFunction->setVisible(false);
        ui->tabFunctionParameters->setVisible(false);
        ui->labWidth->setVisible(false);
        ui->labShift->setVisible(false);
        ui->edtWidth->setVisible(false);
        ui->edtShift->setVisible(false);
        ui->widWidth->setVisible(false);
        ui->labWidth->setText(tr("Width:"));

        ui->labRangeData->setVisible(false);
        ui->labRangeStyle->setVisible(false);
        ui->widRangeData->setVisible(false);
        ui->widRangeStyle->setVisible(false);
        ui->btnRefit->setVisible(false);
        ui->labOffset->setVisible(false);
        ui->edtOffset->setVisible(false);

        switch(ui->cmbGraphType->itemData(ui->cmbGraphType->currentIndex()).toInt()) {


            case QFRDRTable::gtImpulsesVertical:
                //graph.type=QFRDRTable::gtImpulsesVertical;
                ui->widErrorX->setVisible(false);
                ui->labErrorX->setVisible(false);
                ui->chkDrawLine->setVisible(false);
                ui->cmbLineStyle->setVisible(false);
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->widErrorX2->setVisible(false);
                ui->labOffset->setVisible(true);
                ui->edtOffset->setVisible(true);

                break;
            case QFRDRTable::gtImpulsesHorizontal:
                //graph.type=QFRDRTable::gtImpulsesHorizontal;
                ui->widErrorX->setVisible(false);
                ui->labErrorX->setVisible(false);
                ui->chkDrawLine->setVisible(false);
                ui->cmbLineStyle->setVisible(false);
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->btnFit->setVisible(false);
                ui->widErrorX2->setVisible(false);
                ui->labOffset->setVisible(true);
                ui->edtOffset->setVisible(true);



                break;
            case QFRDRTable::gtFilledCurveX:
                //graph.type=QFRDRTable::gtFilledCurveX;
                ui->widErrorX->setVisible(false);
                ui->labErrorX->setVisible(false);
                ui->labSymbol->setVisible(false);
                ui->widSymbol->setVisible(false);
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->widErrorX2->setVisible(false);
                ui->labOffset->setVisible(true);
                ui->edtOffset->setVisible(true);


                break;
            case QFRDRTable::gtFilledCurveY:
                //graph.type=QFRDRTable::gtFilledCurveY;
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->widErrorX->setVisible(false);
                ui->labErrorX->setVisible(false);
                ui->labSymbol->setVisible(false);
                ui->widSymbol->setVisible(false);
                ui->btnFit->setVisible(false);
                ui->widErrorX2->setVisible(false);
                ui->labOffset->setVisible(true);
                ui->edtOffset->setVisible(true);


                break;
            case QFRDRTable::gtStepsVertical:
                //graph.type=QFRDRTable::gtStepsVertical;
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->widErrorX->setVisible(false);
                ui->labErrorX->setVisible(false);
                ui->widErrorY->setVisible(false);
                ui->labErrorY->setVisible(false);
                ui->labSymbol->setVisible(false);
                ui->widSymbol->setVisible(false);
                ui->widErrorStyle->setVisible(false);
                ui->labErrorStyle->setVisible(false);
                ui->widErrorX2->setVisible(false);
                ui->widErrorY2->setVisible(false);

                /*ui->cmbErrorColor->setVisible(false);
                ui->cmbErrorStyle->setVisible(false);
                ui->cmbLinesXError->setVisible(false);
                ui->cmbLinesYError->setVisible(false);
                ui->cmbSymbol->setVisible(false);
                ui->spinSymbolSize->setVisible(false);
                ui->sliderErrorTransparent->setVisible(false);*/
                break;
            case QFRDRTable::gtStepsHorizontal:
                //graph.type=QFRDRTable::gtStepsHorizontal;
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->widErrorX->setVisible(false);
                ui->labErrorX->setVisible(false);
                ui->widErrorY->setVisible(false);
                ui->labErrorY->setVisible(false);
                ui->labSymbol->setVisible(false);
                ui->widSymbol->setVisible(false);
                ui->widErrorStyle->setVisible(false);
                ui->labErrorStyle->setVisible(false);
                ui->btnFit->setVisible(false);
                ui->widErrorX2->setVisible(false);
                ui->widErrorY2->setVisible(false);
/*                    ui->cmbErrorColor->setVisible(false);
                ui->cmbErrorStyle->setVisible(false);
                ui->cmbLinesXError->setVisible(false);
                ui->cmbLinesYError->setVisible(false);
                ui->cmbSymbol->setVisible(false);
                ui->spinSymbolSize->setVisible(false);
                ui->sliderErrorTransparent->setVisible(false);*/
                break;
            case QFRDRTable::gtBarsVertical:
                //graph.type=QFRDRTable::gtbarsVertical;
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->widErrorX->setVisible(false);
                ui->labErrorX->setVisible(false);
                ui->widErrorY->setVisible(true);
                ui->labErrorY->setVisible(true);
                ui->labSymbol->setVisible(false);
                ui->widSymbol->setVisible(false);
                //ui->widErrorStyle->setVisible(false);
                //ui->labErrorStyle->setVisible(false);
                ui->chkDrawLine->setVisible(false);
                ui->labWidth->setVisible(true);
                ui->labShift->setVisible(true);
                ui->edtWidth->setVisible(true);
                ui->edtShift->setVisible(true);
                ui->widWidth->setVisible(true);
                ui->widErrorX2->setVisible(true);
                ui->widErrorY2->setVisible(true);
                ui->labOffset->setVisible(true);
                ui->edtOffset->setVisible(true);

/*                    ui->cmbErrorColor->setVisible(false);
                ui->cmbErrorStyle->setVisible(false);
                ui->cmbLinesXError->setVisible(false);
                ui->cmbLinesYError->setVisible(false);
                ui->chkDrawLine->setVisible(false);
                ui->cmbSymbol->setVisible(false);
                ui->spinSymbolSize->setVisible(false);
                ui->sliderErrorTransparent->setVisible(false);*/
                break;
            case QFRDRTable::gtBarsHorizontal:
                //graph.type=QFRDRTable::gtbarsHorizontal;
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->widErrorX->setVisible(true);
                ui->labErrorX->setVisible(true);
                ui->widErrorY->setVisible(false);
                ui->labErrorY->setVisible(false);
                ui->labSymbol->setVisible(false);
                ui->widSymbol->setVisible(false);
                //ui->widErrorStyle->setVisible(false);
                //ui->labErrorStyle->setVisible(false);
                ui->chkDrawLine->setVisible(false);
                ui->btnFit->setVisible(false);
                ui->labWidth->setVisible(true);
                ui->labShift->setVisible(true);
                ui->edtWidth->setVisible(true);
                ui->edtShift->setVisible(true);
                ui->widWidth->setVisible(true);
                ui->widErrorX2->setVisible(true);
                ui->widErrorY2->setVisible(true);
                ui->labOffset->setVisible(true);
                ui->edtOffset->setVisible(true);
                break;
            case QFRDRTable::gtParametrizedScatter:
                ui->labImage->setVisible(true);
                ui->labImage->setText(tr("colorbar:"));
                ui->widImage->setVisible(true);
                ui->labMean->setVisible(true);
                ui->labQ75->setVisible(true);
                ui->labMean->setText(tr("color col.:"));
                ui->labQ75->setText(tr("size col.:"));
                ui->cmbLinesMean->setVisible(true);
                ui->cmbLinesQ75->setVisible(true);
                ui->btnClearLinesMean->setVisible(true);
                ui->btnClearLinesQ75->setVisible(true);


                ui->labTransparencyFalse->setVisible(false);
                ui->widFalseTransparency->setVisible(false);
                ui->labTransparencyTrue->setVisible(false);
                ui->widTrueTransparency->setVisible(false);
                ui->labTrueColor->setVisible(false);
                ui->cmbImageTrueColor->setVisible(false);
                ui->labFalseColor->setVisible(false);
                ui->cmbImageFalseColor->setVisible(false);
                ui->labColormap->setVisible(true);
                ui->cmbColormap->setVisible(true);
                ui->labColorbarLabel->setVisible(true);
                ui->edtColorbarLabel->setVisible(true);
                ui->labModifierMode->setVisible(false);
                ui->cmbModifierMode->setVisible(false);
                ui->labColorbarLabelMod->setVisible(false);
                ui->edtColorbarLabelMod->setVisible(false);

                ui->labImageHeight->setVisible(false);
                ui->labImageWidth->setVisible(false);
                ui->labImageWidthPixels->setVisible(false);
                ui->spinImageWidth->setVisible(false);
                ui->edtImageWidth->setVisible(false);
                ui->edtImageHeight->setVisible(false);
                ui->edtImageX->setVisible(false);
                ui->edtImageY->setVisible(false);
                ui->labImageX->setVisible(false);
                ui->labImageY->setVisible(false);
                ui->btnAutoImageSizes->setVisible(false);



                break;
            case QFRDRTable::gtBoxplotX:
            case QFRDRTable::gtBoxplotY:
                //graph.type=QFRDRTable::gtBoxplotX, gtBoxplotY;
                ui->labMax->setVisible(true);
                ui->labMean->setVisible(true);
                ui->labQ75->setVisible(true);
                ui->labDataX->setText(tr("position col.:"));
                ui->labErrorX->setText(tr("minimum col.:"));
                ui->labDataY->setText(tr("Q25%  col.:"));
                ui->labErrorY->setText(tr("median col.:"));
                ui->labMax->setText(tr("max col.:"));
                ui->labMean->setText(tr("mean col.:"));
                ui->labQ75->setText(tr("Q75%  col.:"));
                ui->cmbLinesXError->setVisible(true);
                ui->cmbLinesYData->setVisible(true);
                ui->cmbLinesYError->setVisible(true);
                ui->cmbLinesMax->setVisible(true);
                ui->cmbLinesMean->setVisible(true);
                ui->cmbLinesQ75->setVisible(true);
                ui->widErrorY->setVisible(true);
                ui->widErrorX->setVisible(true);
                ui->widErrorY2->setVisible(false);
                ui->widErrorX2->setVisible(false);

                ui->btnClearLinesMax->setVisible(true);
                ui->btnClearLinesMean->setVisible(true);
                ui->btnClearLinesQ75->setVisible(true);

                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->labSymbol->setVisible(true);
                ui->widSymbol->setVisible(true);
                ui->widErrorStyle->setVisible(false);
                ui->labErrorStyle->setVisible(false);
                ui->chkDrawLine->setVisible(false);
                ui->btnFit->setVisible(false);
                ui->labWidth->setVisible(true);
                ui->labWidth->setText(tr("boxplot width:"));
                ui->labShift->setVisible(false);
                ui->edtWidth->setVisible(true);
                ui->edtShift->setVisible(false);
                ui->widWidth->setVisible(true);
                ui->cmbWhiskerStyle->setVisible(true);
                ui->labWhisker->setVisible(true);
                break;
            case QFRDRTable::gtImage:
                //graph.type=QFRDRTable::gtImage;
                ui->btnFit->setVisible(false);
                ui->widErrorX->setVisible(false);
                ui->labErrorX->setVisible(false);
                ui->cmbLinesYData->setVisible(true);
                ui->labDataY->setVisible(true);
                ui->widErrorY->setVisible(false);
                ui->labErrorY->setVisible(false);
                ui->labSymbol->setVisible(false);
                ui->widSymbol->setVisible(false);
                ui->widErrorStyle->setVisible(false);
                ui->labErrorStyle->setVisible(false);
                ui->widColor->setVisible(false);
                ui->labColor->setVisible(false);
                ui->widFillColor->setVisible(false);
                ui->labFillColor->setVisible(false);
                ui->widLineStyle->setVisible(false);
                ui->labLinestyle->setVisible(false);
                ui->labDataX->setText(tr("image col.:"));
                ui->labDataY->setText(tr("modifier col.:"));
                ui->widErrorX2->setVisible(false);
                ui->btnClearLinesYData->setVisible(true);
                ui->widErrorY2->setVisible(false);
                ui->chkSelectData->setVisible(false);
                ui->widDataSelect->setVisible(false);
                ui->chkSTrided->setVisible(false);
                ui->widStride->setVisible(false);
                ui->labSort->setVisible(false);
                ui->cmbSort->setVisible(false);

                ui->labTransparencyFalse->setVisible(false);
                ui->widFalseTransparency->setVisible(false);
                ui->labTransparencyTrue->setVisible(false);
                ui->widTrueTransparency->setVisible(false);
                ui->labTrueColor->setVisible(false);
                ui->cmbImageTrueColor->setVisible(false);
                ui->labFalseColor->setVisible(false);
                ui->cmbImageFalseColor->setVisible(false);
                ui->labColormap->setVisible(true);
                ui->cmbColormap->setVisible(true);
                ui->labColorbarLabel->setVisible(true);
                ui->edtColorbarLabel->setVisible(true);
                ui->labModifierMode->setVisible(true);
                ui->cmbModifierMode->setVisible(true);
                ui->labColorbarLabelMod->setVisible(true);
                ui->edtColorbarLabelMod->setVisible(true);

                break;
            case QFRDRTable::gtRGBImage:
                //graph.type=QFRDRTable::gtRGBImage;
                ui->btnFit->setVisible(false);
                ui->widErrorX->setVisible(true);
                ui->labErrorX->setVisible(true);
                ui->cmbLinesYData->setVisible(true);
                ui->labDataY->setVisible(true);
                ui->widErrorY2->setVisible(false);
                ui->widErrorX2->setVisible(false);
                ui->btnClearLinesYData->setVisible(true);
                ui->widErrorY->setVisible(true);
                ui->labErrorY->setVisible(true);
                ui->labSymbol->setVisible(false);
                ui->widSymbol->setVisible(false);
                ui->widErrorStyle->setVisible(false);
                ui->labErrorStyle->setVisible(false);
                ui->widColor->setVisible(false);
                ui->labColor->setVisible(false);
                ui->widFillColor->setVisible(false);
                ui->labFillColor->setVisible(false);
                ui->widLineStyle->setVisible(false);
                ui->labLinestyle->setVisible(false);
                ui->labDataX->setText(tr("R col.:"));
                ui->labErrorX->setText(tr("G col.:"));
                ui->labDataY->setText(tr("B col.:"));
                ui->labErrorY->setText(tr("modifier col.:"));
                ui->chkSelectData->setVisible(false);
                ui->widDataSelect->setVisible(false);
                ui->chkSTrided->setVisible(false);
                ui->widStride->setVisible(false);
                ui->labSort->setVisible(false);
                ui->cmbSort->setVisible(false);

                ui->labTransparencyFalse->setVisible(false);
                ui->widFalseTransparency->setVisible(false);
                ui->labTransparencyTrue->setVisible(false);
                ui->widTrueTransparency->setVisible(false);
                ui->labTrueColor->setVisible(false);
                ui->cmbImageTrueColor->setVisible(false);
                ui->labFalseColor->setVisible(false);
                ui->cmbImageFalseColor->setVisible(false);
                ui->labColormap->setVisible(false);
                ui->cmbColormap->setVisible(false);
                ui->labColorbarLabel->setVisible(true);
                ui->edtColorbarLabel->setVisible(true);
                ui->labModifierMode->setVisible(true);
                ui->cmbModifierMode->setVisible(true);
                ui->labColorbarLabelMod->setVisible(false);
                ui->edtColorbarLabelMod->setVisible(false);
                ui->labColorbarLabelG->setVisible(true);
                ui->edtColorbarLabelG->setVisible(true);
                ui->labColorbarLabelB->setVisible(true);
                ui->edtColorbarLabelB->setVisible(true);
                ui->labColorbarLabel->setText(tr("R bar label:"));

                break;
            case QFRDRTable::gtMaskImage:
                //graph.type=QFRDRTable::gtMaskImage;
                ui->btnFit->setVisible(false);
                ui->labDataX->setText(tr("mask col.:"));
                ui->widErrorX->setVisible(false);
                ui->labErrorX->setVisible(false);
                ui->cmbLinesYData->setVisible(false);
                ui->btnAutoY->setVisible(false);
                ui->labDataY->setVisible(false);
                ui->widErrorY2->setVisible(false);
                ui->widErrorX2->setVisible(false);
                ui->btnClearLinesYData->setVisible(false);
                ui->widErrorY->setVisible(false);
                ui->chkSelectData->setVisible(false);
                ui->widDataSelect->setVisible(false);
                ui->chkSTrided->setVisible(false);
                ui->widStride->setVisible(false);
                ui->labSort->setVisible(false);
                ui->cmbSort->setVisible(false);

                ui->labErrorY->setVisible(false);
                ui->labSymbol->setVisible(false);
                ui->widSymbol->setVisible(false);
                ui->widErrorStyle->setVisible(false);
                ui->labErrorStyle->setVisible(false);
                ui->widColor->setVisible(false);
                ui->labColor->setVisible(false);
                ui->widFillColor->setVisible(false);
                ui->labFillColor->setVisible(false);
                ui->widLineStyle->setVisible(false);
                ui->labLinestyle->setVisible(false);
                ui->cmbModifierMode->setVisible(false);
                ui->labModifierMode->setVisible(false);

                ui->labTransparencyFalse->setVisible(true);
                ui->widFalseTransparency->setVisible(true);
                ui->labTransparencyTrue->setVisible(true);
                ui->widTrueTransparency->setVisible(true);
                ui->labTrueColor->setVisible(true);
                ui->cmbImageTrueColor->setVisible(true);
                ui->labFalseColor->setVisible(true);
                ui->cmbImageFalseColor->setVisible(true);
                ui->labColormap->setVisible(false);
                ui->cmbColormap->setVisible(false);
                ui->labColorbarLabel->setVisible(false);
                ui->edtColorbarLabel->setVisible(false);
                ui->labModifierMode->setVisible(false);
                ui->cmbModifierMode->setVisible(false);
                ui->labColorbarLabelMod->setVisible(false);
                ui->edtColorbarLabelMod->setVisible(false);

                break;
            case QFRDRTable::gtFunction:
                //graph.type=QFRDRTable::gtFunction;
                ui->btnFit->setVisible(false);
                ui->labDataY->setText(tr("func. param. col."));
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->widErrorX->setVisible(false);
                ui->widErrorY->setVisible(false);
                ui->labErrorX->setVisible(false);
                ui->labErrorY->setVisible(false);
                ui->cmbLinesXData->setVisible(false);
                ui->btnAutoX->setVisible(false);
                ui->labDataX->setVisible(false);
                ui->widErrorY2->setVisible(false);
                ui->widErrorX2->setVisible(false);
                ui->btnClearLinesYData->setVisible(false);
                ui->widErrorY->setVisible(false);
                ui->btnClearLinesQ75->setVisible(false);
                ui->btnClearLinesMean->setVisible(false);
                ui->btnClearLinesMax->setVisible(false);
                ui->labErrorY->setVisible(false);
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->labSymbol->setVisible(false);
                ui->widSymbol->setVisible(false);
                ui->chkSelectData->setVisible(false);
                ui->widDataSelect->setVisible(false);
                ui->chkSTrided->setVisible(false);
                ui->widStride->setVisible(false);
                ui->labSort->setVisible(false);
                ui->cmbSort->setVisible(false);

                ui->btnRefit->setVisible(isFitResult);

                ui->labFuctionType->setVisible(true);
                ui->cmbFunctionType->setVisible(true);

                if (ui->cmbFunctionType->currentIndex()==0) {
                    ui->widFunction->setVisible(true);
                    ui->labFuction->setVisible(true);
                } else if (ui->cmbFunctionType->currentIndex()==ui->cmbFunctionType->count()-1) {
                    ui->labQFFitFuction->setVisible(true);
                    ui->labQFFitFuction2->setVisible(true);
                    ui->widQFFitFunctionSubplot->setVisible(true);
                    ui->cmbQFFitFunction->setVisible(true);
                    if (ui->cmbLinesYData->currentIndex()==0) {
                        ui->labFuctionParameters->setVisible(true);
                        ui->tabFunctionParameters->setVisible(true);
                    }
                } else {
                    if (ui->cmbLinesYData->currentIndex()==0) {
                        ui->labFuctionParameters->setVisible(true);
                        ui->tabFunctionParameters->setVisible(true);
                    }
                }
                break;

            case QFRDRTable::gtHorizontalRange: case QFRDRTable::gtVerticalRange: // horicontal/vertical range
                ui->btnFit->setVisible(false);
                ui->labRangeData->setVisible(true);
                ui->labRangeStyle->setVisible(true);
                ui->widRangeData->setVisible(true);
                ui->widRangeStyle->setVisible(true);
                ui->labSort->setVisible(false);
                ui->cmbSort->setVisible(false);

                ui->widErrorX->setVisible(false);
                ui->widErrorX2->setVisible(false);
                ui->labErrorX->setVisible(false);
                ui->labSymbol->setVisible(false);
                ui->widSymbol->setVisible(false);
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                ui->btnAutoX->setVisible(false);

                ui->widErrorStyle->setVisible(false);
                ui->widFillColor->setVisible(true);
                ui->widImage->setVisible(false);
                ui->widSymbol->setVisible(false);
                ui->labErrorStyle->setVisible(false);
                ui->labDataX->setVisible(false);
                ui->labDataY->setVisible(false);
                ui->labErrorX->setVisible(false);
                ui->labErrorY->setVisible(false);
                ui->labImage->setVisible(false);
                ui->labSymbol->setVisible(false);
                ui->labTitle->setVisible(true);
                ui->labType->setVisible(true);
                ui->chkDrawLine->setVisible(true);
                ui->cmbLineStyle->setVisible(true);
                ui->cmbLinesXData->setVisible(false);
                ui->btnAutoX->setVisible(false);
                ui->widErrorX->setVisible(false);
                ui->cmbLinesYData->setVisible(false);
                ui->btnAutoY->setVisible(false);
                ui->widErrorY->setVisible(false);
                ui->widFunction->setVisible(false);
                ui->labFuction->setVisible(false);
                ui->chkSelectData->setVisible(false);
                ui->widDataSelect->setVisible(false);
                ui->chkSTrided->setVisible(false);
                ui->widStride->setVisible(false);
                ui->btnClearLinesXData->setVisible(false);
                ui->widErrorX->setVisible(false);
                ui->btnClearLinesYData->setVisible(false);
                ui->widErrorY2->setVisible(false);
                ui->btnClearLinesMax->setVisible(false);
                ui->btnClearLinesMean->setVisible(false);
                ui->btnClearLinesQ75->setVisible(false);




                break;

            case QFRDRTable::gtLines:
            default:
                //graph.type=QFRDRTable::gtLines;
                ui->labImage->setVisible(false);
                ui->widImage->setVisible(false);
                break;
        }

//        bool isConn=m_connected;
//        disconnectWidgets();
//        if (widFocus && widFocus->isVisible() && focusWidget()!=widFocus) widFocus->setFocus();
        if (isVisible()) setUpdatesEnabled(updt);
//        if (isConn) connectWidgets();


    }
}

void QFTableGraphSettings::connectWidgets()
{
    //qDebug()<<"connectWidgets";
    m_connected=true;

    connect(ui->edtFunction, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtGraphTitle, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->cmbGraphType, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbLinesXData, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbLinesXError, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbLinesXError2, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbRangeMode, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->chkShowTitle, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    connect(ui->chkXErrSym, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    connect(ui->chkYErrSym, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    connect(ui->cmbLinesYData, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbLinesYError, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));

    connect(ui->cmbLinesMax, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbLinesMean, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbLinesYError2, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbLinesQ75, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->edtWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtShift, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtOffset, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->cmbSort, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));

    connect(ui->cmbFillColor, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbErrorColor, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbErrorStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbLineColor, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbWhiskerStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbFillStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbSymbol, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->chkDrawLine, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    connect(ui->spinLineWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->spinSymbolSize, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->spinSymbolLineWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->sliderErrorTransparent, SIGNAL(valueChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->sliderFillTransparent, SIGNAL(valueChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->sliderPlotTransparent, SIGNAL(valueChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbImageFalseColor, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbImageTrueColor, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbColormap, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbModifierMode, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->spinImageWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtImageHeight, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtImageMax, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtImageMin, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtImageHeight, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtImageX, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtImageY, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->sliderImageFalseColor, SIGNAL(valueChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->sliderImageTrueColor, SIGNAL(valueChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->edtColorbarLabel, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtColorbarLabelB, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtColorbarLabelG, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->spinColorbarWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->spinColorbarHeight, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->chkImageColorbarRight, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    connect(ui->chkImageColorbarTop, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    connect(ui->chkImageAutorange, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    connect(ui->chkSTrided, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    connect(ui->spinStride, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->spinStrideStart, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->chkSelectData, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    connect(ui->cmbSelectDataColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbSelectDataCompare, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->edtSelectDataValue, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtSelectDataValue2, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->cmbSelectDataColumn_2, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbSelectDataCompare_2, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->edtSelectDataValue_2, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtSelectDataValue2_2, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->cmbSelectDataColumn_3, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbSelectDataCompare_3, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->edtSelectDataValue_3, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtSelectDataValue2_3, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->cmbSelectDataLogic12, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbSelectDataLogic23, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbFunctionType, SIGNAL(currentIndexChanged(int)), this, SLOT(cmbFunctionTypeCurrentIndexChanged(int)));
    connect(ui->cmbQFFitFunction, SIGNAL(currentIndexChanged(int)), this, SLOT(fitFunctionChanged()));
    connect(ui->cmbQFFitFunctionSubplot, SIGNAL(currentIndexChanged(int)), this, SLOT(fitFunctionChanged()));
    connect(ui->chkQFFitFunctionSubplotAll, SIGNAL(toggled(bool)), this, SLOT(fitFunctionChanged()));


    connect(ui->cmbRangeCenterColor, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->cmbRangeCenterStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->chkRangeDrawCenter, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    connect(ui->chkRangeFillRange, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    connect(ui->chkRangeInverted, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    connect(ui->spinRangeCenterWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtRangeCenter, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtRangeStart, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->edtRangeEnd, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->sliderRangeCenterTransparency, SIGNAL(valueChanged(int)), this, SLOT(writeGraphData()));
    connect(fitfuncValuesTable, SIGNAL(fitParamChanged()), this, SLOT(writeGraphData()));

    connect(ui->spinColorbarModTicks, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->spinColorbarTicks, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->spinColorbarTickLength, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->spinColorbarFontsize, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));

    connect(ui->spinErrorLineWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->spinErrorBarWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    connect(ui->cmbErrorLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));


    connect(ui->cmbColorbarLabelType, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    connect(ui->spinColorbarLabelDigits, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
}

void QFTableGraphSettings::disconnectWidgets()
{
    //qDebug()<<"disconnectWidgets";
    m_connected=false;
    disconnect(ui->edtWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtShift, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));

    disconnect(ui->edtOffset, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->cmbSort, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbRangeMode, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));

    disconnect(ui->cmbLinesQ75, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->edtFunction, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtGraphTitle, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->chkShowTitle, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbGraphType, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbLinesXData, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbLinesXError, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbLinesXError2, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->chkXErrSym, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    disconnect(ui->chkYErrSym, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbLinesYData, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbLinesYError, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbLinesMax, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbLinesMean, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbLinesYError2, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbErrorColor, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbFillColor, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbErrorStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbLineColor, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbWhiskerStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbFillStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbSymbol, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbModifierMode, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->chkDrawLine, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    disconnect(ui->spinLineWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->spinSymbolSize, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->spinSymbolLineWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->sliderErrorTransparent, SIGNAL(valueChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->sliderFillTransparent, SIGNAL(valueChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->sliderPlotTransparent, SIGNAL(valueChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbImageFalseColor, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbImageTrueColor, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbColormap, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->spinImageWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtImageHeight, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtImageMax, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtImageMin, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtImageHeight, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtImageX, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtImageY, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->sliderImageFalseColor, SIGNAL(valueChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->sliderImageTrueColor, SIGNAL(valueChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->edtColorbarLabel, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtColorbarLabelB, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtColorbarLabelG, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->spinColorbarWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->spinColorbarHeight, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->chkImageColorbarRight, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    disconnect(ui->chkImageColorbarTop, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    disconnect(ui->chkImageAutorange, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    disconnect(ui->chkSTrided, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    disconnect(ui->spinStride, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->spinStrideStart, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->chkSelectData, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbSelectDataColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbSelectDataCompare, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->edtSelectDataValue, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtSelectDataValue2, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->cmbSelectDataColumn_2, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbSelectDataCompare_2, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->edtSelectDataValue_2, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtSelectDataValue2_2, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->cmbSelectDataColumn_3, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbSelectDataCompare_3, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->edtSelectDataValue_3, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtSelectDataValue2_3, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->cmbSelectDataLogic12, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbSelectDataLogic23, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));

    disconnect(ui->cmbFunctionType, SIGNAL(currentIndexChanged(int)), this, SLOT(cmbFunctionTypeCurrentIndexChanged(int)));
    disconnect(ui->cmbQFFitFunctionSubplot, SIGNAL(currentIndexChanged(int)), this, SLOT(fitFunctionChanged()));
    disconnect(ui->chkQFFitFunctionSubplotAll, SIGNAL(toggled(bool)), this, SLOT(fitFunctionChanged()));
    disconnect(ui->cmbQFFitFunction, SIGNAL(currentIndexChanged(int)), this, SLOT(fitFunctionChanged()));
    disconnect(ui->cmbRangeCenterColor, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->cmbRangeCenterStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->chkRangeDrawCenter, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    disconnect(ui->chkRangeFillRange, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    disconnect(ui->chkRangeInverted, SIGNAL(toggled(bool)), this, SLOT(writeGraphData()));
    disconnect(ui->spinRangeCenterWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtRangeCenter, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtRangeStart, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->edtRangeEnd, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->sliderRangeCenterTransparency, SIGNAL(valueChanged(int)), this, SLOT(writeGraphData()));
    disconnect(fitfuncValuesTable, SIGNAL(fitParamChanged()), this, SLOT(writeGraphData()));
    disconnect(ui->spinColorbarModTicks, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->spinColorbarTicks, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->spinColorbarTickLength, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->spinColorbarFontsize, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));

    disconnect(ui->cmbColorbarLabelType, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
    disconnect(ui->spinColorbarLabelDigits, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));

    disconnect(ui->spinErrorLineWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->spinErrorBarWidth, SIGNAL(editingFinished()), this, SLOT(writeGraphData()));
    disconnect(ui->cmbErrorLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(writeGraphData()));
}

void QFTableGraphSettings::doFit()
{
    QFRDRTable::GraphInfo graph;
    writeGraphData(graph);
    QFRDRTable::GraphDataSelection sel;
    sel=graph;
    emit performFit(ui->cmbLinesXData->currentData().toInt(), ui->cmbLinesYData->currentData().toInt(), ui->cmbLinesYError->currentData().toInt(), plot, QString(""), sel);
}

void QFTableGraphSettings::doRefit()
{
    emit performRefit(plot);
}

void QFTableGraphSettings::doRegression()
{
    QFRDRTable::GraphInfo graph;
    writeGraphData(graph);
    QFRDRTable::GraphDataSelection sel;
    sel=graph;
    emit performRegression(ui->cmbLinesXData->currentData().toInt(), ui->cmbLinesYData->currentData().toInt(), ui->cmbLinesYError->currentData().toInt(), plot, sel);
}



void QFTableGraphSettings::initFocus()
{
    ui->edtGraphTitle->setFocus();
}

void QFTableGraphSettings::on_edtFunction_textChanged(const QString &text)
{
    ui->labFunctionOK->setText(tr("<font color=\"darkgreen\">function OK</font>"));
    QFMathParser mp; // instanciate
    QFMathParser::qfmpNode* n;
    // parse some numeric expression
    n=mp.parse(text);
    delete n;
    if (mp.hasErrorOccured()) {
        ui->labFunctionOK->setText(tr("<font color=\"red\">ERROR in function:<br>&nbsp;&nbsp;&nbsp;&nbsp;%1</font>").arg(mp.getLastErrors().join("<br>&nbsp;&nbsp;&nbsp;&nbsp;")));
    }
}

void QFTableGraphSettings::on_btnAutoX_clicked()
{
    QComboBox* cmbInput=ui->cmbLinesXData;
    QString name=cmbInput->currentText();
    QStringList searchPhrases;
    QString id;
    QString typ;
    QRegExp rxID("([^\\(\\[\\{]*)[\\[\\(\\{\\:]+([^\\]\\)\\}]*)[\\]\\)\\}]?");
    rxID.setCaseSensitivity(Qt::CaseInsensitive);

    QFRDRTable::GraphType gt=(QFRDRTable::GraphType)ui->cmbGraphType->itemData(ui->cmbGraphType->currentIndex()).toInt();
    if (gt==QFRDRTable::gtLines || gt==QFRDRTable::gtImpulsesVertical || gt==QFRDRTable::gtFilledCurveX || gt==QFRDRTable::gtStepsHorizontal
        || gt==QFRDRTable::gtBarsHorizontal || gt==QFRDRTable::gtImpulsesHorizontal || gt==QFRDRTable::gtParametrizedScatter || gt==QFRDRTable::gtFilledCurveY
        || gt==QFRDRTable::gtStepsVertical || gt==QFRDRTable::gtBarsVertical) {

        if (rxID.indexIn(name)>=0) {
            id=rxID.cap(2);
            typ=rxID.cap(1).toLower();
            //qDebug()<<typ<<id;
            if (typ.contains("mean") || typ.contains("average") || typ.contains("avg") || typ.contains("mittelwert") || typ.contains("mittel") || typ.contains("median") || typ.contains("med")) {
                searchPhrases<<"std"<<"stddev"<<"standard deviation"<<"sd"<<"s.d."<<"stabw"<<"stab"<<"iqr";
            }



            for (int step=0; step<2; step++) {
                for (int i=0; i<cmbInput->count(); i++) {
                    QString en=cmbInput->itemText(i);
                    if (en!=name) {
                        bool found=(rxID.indexIn(en)>=0);
                        bool exactmatch=(rxID.cap(2)==id);
                        //qDebug()<<step<<i<<en<<found<<exactmatch<<rxID.cap(1)<<rxID.cap(1)<<rxID.cap(2);
                        if ((exactmatch && step==0)||(found && step==1)) {
                            for (int j=0; j<searchPhrases.size(); j++) {
                                if (en.contains(searchPhrases[j], Qt::CaseInsensitive)) {
                                    ui->cmbLinesXError->setCurrentIndex(i);
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }

    }
}

void QFTableGraphSettings::on_btnAutoY_clicked()
{
    QComboBox* cmbInput=ui->cmbLinesYData;
    QString name=cmbInput->currentText();
    QStringList searchPhrases;
    QString id;
    QString typ;
    QRegExp rxID("([^\\(\\[\\{]*)[\\[\\(\\{\\:]+([^\\]\\)\\}]*)[\\]\\)\\}]?");
    rxID.setCaseSensitivity(Qt::CaseInsensitive);

    QRegExp rxIDD=rxID;

    QFRDRTable::GraphType gt=(QFRDRTable::GraphType)ui->cmbGraphType->itemData(ui->cmbGraphType->currentIndex()).toInt();
    if (gt==QFRDRTable::gtLines || gt==QFRDRTable::gtImpulsesVertical || gt==QFRDRTable::gtFilledCurveX || gt==QFRDRTable::gtStepsHorizontal
        || gt==QFRDRTable::gtBarsHorizontal || gt==QFRDRTable::gtImpulsesHorizontal || gt==QFRDRTable::gtParametrizedScatter || gt==QFRDRTable::gtFilledCurveY
        || gt==QFRDRTable::gtStepsVertical || gt==QFRDRTable::gtBarsVertical) {

        if (rxID.indexIn(name)>=0) {
            id=rxID.cap(2);
            typ=rxID.cap(1).toLower();
            //qDebug()<<typ<<id;
            if (typ.contains("mean") || typ.contains("average") || typ.contains("avg") || typ.contains("mittel") || typ.contains("mittelwert") || typ.contains("median") || typ.contains("med")) {
                searchPhrases<<"std"<<"stddev"<<"standard deviation"<<"sd"<<"s.d."<<"stabw"<<"stab";
            }

            for (int step=0; step<2; step++) {
                for (int i=0; i<cmbInput->count(); i++) {
                    QString en=cmbInput->itemText(i);
                    if (en!=name) {
                        bool found=(rxID.indexIn(en)>=0);
                        bool exactmatch=(rxID.cap(2)==id);
                        if ((exactmatch && step==0)||(found && step==1)) {
                            for (int j=0; j<searchPhrases.size(); j++) {
                                if (en.contains(searchPhrases[j], Qt::CaseInsensitive)) {
                                    ui->cmbLinesYError->setCurrentIndex(i);
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }

    } else if (gt==QFRDRTable::gtBoxplotX || gt==QFRDRTable::gtBoxplotY) {
        if (rxID.indexIn(name)>=0) {
            id=rxID.cap(2);
            typ=rxID.cap(1).toLower();
            //qDebug()<<typ<<id;
            /*if (typ.contains("mean") || typ.contains("average") || typ.contains("avg") || typ.contains("mittel") || typ.contains("median") || typ.contains("med")) {
                searchPhrases<<"std"<<"stddev"<<"standard deviation"<<"sd"<<"s.d."<<"stabw"<<"stab";
            } else if (typ.contains("mean") || typ.contains("average") || typ.contains("avg") || typ.contains("mittel") || typ.contains("median") || typ.contains("med")) {
                searchPhrases<<"std"<<"stddev"<<"standard deviation"<<"sd"<<"s.d."<<"stabw"<<"stab";
            }*/

            bool done1=false, done2=false, done3=false, done4=false, done5=false, done6=false;
            for (int step=0; step<2; step++) {
                for (int i=0; i<cmbInput->count(); i++) {
                    QString en=cmbInput->itemText(i);
                    if (en!=name) {
                        bool found=(rxID.indexIn(en)>=0);
                        bool exactmatch=found && (rxID.cap(2)==id);
                        //qDebug()<<step<<i<<en<<found<<exactmatch<<rxID.cap(1)<<rxID.cap(2);
                        if ((exactmatch && step==0)||(found && step==1)) {
                            searchPhrases.clear();
                            searchPhrases<<"minimum"<<"min";
                            for (int j=0; j<searchPhrases.size(); j++) {
                                if (!done1 && en.contains(searchPhrases[j], Qt::CaseInsensitive)) {
                                    ui->cmbLinesXError->setCurrentIndex(i);
                                    done1=true;
                                    break;
                                }
                            }
                            /*searchPhrases.clear();
                            searchPhrases<<"quantile25"<<"quant25"<<"q25"<<"0.25"<<"25";
                            for (int j=0; j<searchPhrases.size(); j++) {
                                if (!done2 && en.contains(searchPhrases[j], Qt::CaseInsensitive)) {
                                    ui->cmbLinesYData->setCurrentIndex(i);
                                    done2=true;
                                    break;
                                }
                            }*/
                            searchPhrases.clear();
                            searchPhrases<<"median"<<"med"<<"quantile50"<<"quant50"<<"q50";
                            for (int j=0; j<searchPhrases.size(); j++) {
                                if (!done3 && en.contains(searchPhrases[j], Qt::CaseInsensitive)) {
                                    ui->cmbLinesYError->setCurrentIndex(i);
                                    done3=true;
                                    break;
                                }
                            }
                            searchPhrases.clear();
                            searchPhrases<<"quantile75"<<"quant75"<<"q75"<<"0.75"<<"75";
                            for (int j=0; j<searchPhrases.size(); j++) {
                                if (!done4 && en.contains(searchPhrases[j], Qt::CaseInsensitive)) {
                                    ui->cmbLinesQ75->setCurrentIndex(i);
                                    done4=true;
                                    break;
                                }
                            }
                            searchPhrases.clear();
                            searchPhrases<<"mean"<<"average"<<"avg"<<"mittel"<<"mittelwert";
                            for (int j=0; j<searchPhrases.size(); j++) {
                                if (!done5 && en.contains(searchPhrases[j], Qt::CaseInsensitive)) {
                                    ui->cmbLinesMean->setCurrentIndex(i);
                                    done5=true;
                                    break;
                                }
                            }
                            searchPhrases.clear();
                            searchPhrases<<"maximum"<<"max";
                            for (int j=0; j<searchPhrases.size(); j++) {
                                if (!done6 && en.contains(searchPhrases[j], Qt::CaseInsensitive)) {
                                    ui->cmbLinesMax->setCurrentIndex(i);
                                    done6=true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void QFTableGraphSettings::on_btnShiftAuto_clicked()
{
    if (!current) return;
    QFRDRTable::PlotInfo p=current->getPlot(plot);
    double cntH=0;
    double cntV=0;
    for (int i=0; i<p.graphs.size(); i++) {
        QFRDRTable::GraphType gt=p.graphs[i].type;
        if (gt==QFRDRTable::gtBarsHorizontal) cntH++;
        if (gt==QFRDRTable::gtBarsVertical) cntV++;
    }

    double widthH=1.0/cntH*0.7;
    double widthV=1.0/cntV*0.7;
    double dH=0.8/(cntH);
    double dV=0.8/(cntV);
    double h=0.1+dH/2.0;
    double v=0.1+dV/2.0;
    for (int i=0; i<p.graphs.size(); i++) {
        QFRDRTable::GraphType gt=p.graphs[i].type;
        if (gt==QFRDRTable::gtBarsHorizontal) {
            if (cntH>1) {
                p.graphs[i].width=widthH;
                p.graphs[i].shift=h-0.5;
                h=h+dH;
            } else {
                p.graphs[i].width=0.9;
                p.graphs[i].shift=0.0;
            }
        }
        if (gt==QFRDRTable::gtBarsVertical) {
            if (cntV>1) {
                p.graphs[i].width=widthV;
                p.graphs[i].shift=v-0.5;
                v=v+dV;
            } else {
                p.graphs[i].width=0.9;
                p.graphs[i].shift=0.0;
            }
        }
    }
    current->setPlot(plot, p);
    emit reloadGraph();
}

void QFTableGraphSettings::on_cmbSelectDataCompare_currentIndexChanged(int index)
{
    ui->edtSelectDataValue2->setVisible(index==QFRDRTable::dsoInRange || index==QFRDRTable::dsoOutOfRange);
}

void QFTableGraphSettings::on_cmbRangeMode_currentIndexChanged(int index)
{
    switch(index) {
        case 1: //line+/-error
            ui->labRangeStart->setEnabled(true);
            ui->labRangeStart->setText(tr("error:"));
            ui->edtRangeStart->setEnabled(true);
            ui->labRangeEnd->setText(tr("end:"));
            ui->labRangeEnd->setEnabled(false);
            ui->edtRangeEnd->setEnabled(false);
            break;
        case 2: //line only
            ui->labRangeStart->setEnabled(false);
            ui->labRangeStart->setText(tr("start:"));
            ui->edtRangeStart->setEnabled(false);
            ui->labRangeEnd->setText(tr("end:"));
            ui->labRangeEnd->setEnabled(false);
            ui->edtRangeEnd->setEnabled(false);
            break;
        default:
        case 0: //range
            ui->labRangeStart->setEnabled(true);
            ui->labRangeStart->setText(tr("start:"));
            ui->edtRangeStart->setEnabled(true);
            ui->labRangeEnd->setText(tr("end:"));
            ui->labRangeEnd->setEnabled(true);
            ui->edtRangeEnd->setEnabled(true);
            break;
    }
}

void QFTableGraphSettings::on_btnClearLinesXData_clicked()
{
    ui->cmbLinesXData->setCurrentData(-1);
}

void QFTableGraphSettings::on_btnClearLinesXError_clicked()
{
    ui->cmbLinesXError->setCurrentData(-1);
}

void QFTableGraphSettings::on_btnClearLinesXError2_clicked()
{
    ui->cmbLinesXError2->setCurrentData(-1);
}

void QFTableGraphSettings::on_btnClearLinesYData_clicked()
{
    ui->cmbLinesYData->setCurrentData(-1);
}

void QFTableGraphSettings::on_btnClearLinesYError_clicked()
{
    ui->cmbLinesYError->setCurrentData(-1);
}

void QFTableGraphSettings::on_btnClearLinesYError2_clicked()
{
    ui->cmbLinesYError2->setCurrentData(-1);
}

void QFTableGraphSettings::on_btnClearLinesMax_clicked()
{
    ui->cmbLinesMax->setCurrentData(-1);
}

void QFTableGraphSettings::on_btnClearLinesMean_clicked()
{
    ui->cmbLinesMean->setCurrentData(-1);
}

void QFTableGraphSettings::on_btnClearLinesQ75_clicked()
{
    ui->cmbLinesQ75->setCurrentData(-1);
}

void QFTableGraphSettings::on_cmbSelectDataLogic12_currentIndexChanged(int /*index*/)
{
    updateSelectDataEnabled();
}

void QFTableGraphSettings::on_cmbSelectDataLogic23_currentIndexChanged(int /*index*/)
{
    updateSelectDataEnabled();
}

void QFTableGraphSettings::updateSelectDataEnabled()
{
    ui->cmbSelectDataColumn_2->setEnabled(ui->cmbSelectDataLogic12->currentIndex()>0);
    ui->cmbSelectDataCompare_2->setEnabled(ui->cmbSelectDataLogic12->currentIndex()>0);
    ui->edtSelectDataValue_2->setEnabled(ui->cmbSelectDataLogic12->currentIndex()>0);
    ui->edtSelectDataValue2_2->setEnabled(ui->cmbSelectDataLogic12->currentIndex()>0);
    ui->cmbSelectDataColumn_3->setEnabled(ui->cmbSelectDataLogic12->currentIndex()>0 && ui->cmbSelectDataLogic23->currentIndex()>0);
    ui->cmbSelectDataCompare_3->setEnabled(ui->cmbSelectDataLogic12->currentIndex()>0 && ui->cmbSelectDataLogic23->currentIndex()>0);
    ui->edtSelectDataValue_3->setEnabled(ui->cmbSelectDataLogic12->currentIndex()>0 && ui->cmbSelectDataLogic23->currentIndex()>0);
    ui->edtSelectDataValue2_3->setEnabled(ui->cmbSelectDataLogic12->currentIndex()>0 && ui->cmbSelectDataLogic23->currentIndex()>0);
    ui->cmbSelectDataLogic23->setEnabled(ui->cmbSelectDataLogic12->currentIndex()>0);


}

void QFTableGraphSettings::cmbFunctionTypeCurrentIndexChanged(int /*index*/)
{

    if (ui->cmbFunctionType->currentIndex()==0) {
        fitfuncValuesTable->setWriteTo(&fitfuncValuesDummy, QStringList());
        fitfuncValuesTable->setAuxiliaryWriteTo(&fitfuncValuesDummy, NULL, NULL, NULL);
    } else if (ui->cmbFunctionType->currentIndex()==1) {
        int vsize=11;
        if (fitfuncValues.size()>vsize) fitfuncValues.remove(fitfuncValues.size()-(fitfuncValues.size()-vsize), fitfuncValues.size()-vsize);
        while (fitfuncValues.size()<vsize) fitfuncValues.append(0);
        QStringList pn;
        pn<<tr("offset");
        for (int i=1; i<fitfuncValues.size(); i++) {
            pn.append(tr("p%1").arg(i+1));
        }
        fitfuncValuesTable->setWriteTo(&fitfuncValues, pn);
        //qDebug()<<"1: fitfuncErrors.size()="<<fitfuncErrors.size()<<"   fitfuncValues.size()="<<fitfuncValues.size();
        if (fitfuncErrors.size()==fitfuncValues.size()) {
            fitfuncValuesTable->setAuxiliaryWriteTo(&fitfuncErrors, NULL, NULL, NULL);
        }
    } else if (ui->cmbFunctionType->currentIndex()==2) {
        int vsize=3;
        if (fitfuncValues.size()>vsize) fitfuncValues.remove(fitfuncValues.size()-(fitfuncValues.size()-vsize), fitfuncValues.size()-vsize);
        while (fitfuncValues.size()<vsize) fitfuncValues.append(0);
        QStringList pn;
        pn<<tr("offset")<<tr("amplitude")<<tr("exponent_div");
        fitfuncValuesTable->setWriteTo(&fitfuncValues, pn);
        //qDebug()<<"2: fitfuncErrors.size()="<<fitfuncErrors.size()<<"   fitfuncValues.size()="<<fitfuncValues.size();
        if (fitfuncErrors.size()==fitfuncValues.size()) {
            fitfuncValuesTable->setAuxiliaryWriteTo(&fitfuncErrors, NULL, NULL, NULL);
        }

    } else if (ui->cmbFunctionType->currentIndex()==3) {
        int vsize=3;
        if (fitfuncValues.size()>vsize) {
            //qDebug()<<"deleting:  oldsize="<<fitfuncValues.size()<<"  newsize="<<vsize<<" startdel="<<fitfuncValues.size()-(vsize-fitfuncValues.size())<<" count="<<vsize-fitfuncValues.size();
            fitfuncValues.remove(fitfuncValues.size()-(fitfuncValues.size()-vsize), fitfuncValues.size()-vsize);
        }
        while (fitfuncValues.size()<vsize) fitfuncValues.append(0);
        QStringList pn;
        pn<<tr("offset")<<tr("amplitude")<<tr("exponent");
        fitfuncValuesTable->setWriteTo(&fitfuncValues, pn);
        //qDebug()<<"3: fitfuncErrors.size()="<<fitfuncErrors.size()<<"   fitfuncValues.size()="<<fitfuncValues.size();
        if (fitfuncErrors.size()==fitfuncValues.size()) {
            fitfuncValuesTable->setAuxiliaryWriteTo(&fitfuncErrors, NULL, NULL, NULL);
        }

    } else if (ui->cmbFunctionType->currentIndex()==4) {
        QFFitFunction* ff(ui->cmbQFFitFunction->createCurrentInstance());
        //fitfuncValues.clear();
        if (ff) {
            int vsize=ff->paramCount();
            if (fitfuncValues.size()>vsize) {
                fitfuncValues.remove(fitfuncValues.size()-(fitfuncValues.size()-vsize), fitfuncValues.size()-vsize);
            }
            for (int i=0; i<ff->paramCount(); i++) {
                if (i>=fitfuncValues.size()) fitfuncValues<<ff->getDescription(i).initialValue;
            }
            QVector<double> p=ff->getInitialParamValues();
            int addplots=ff->getAdditionalPlotCount(ff->getInitialParamValues());
            while (ui->cmbQFFitFunctionSubplot->count()<addplots+1) {
                ui->cmbQFFitFunctionSubplot->addItem("");
            }
            while (ui->cmbQFFitFunctionSubplot->count()>addplots+1) {
                ui->cmbQFFitFunctionSubplot->removeItem(ui->cmbQFFitFunctionSubplot->count()-1);
            }
            ui->cmbQFFitFunctionSubplot->setItemText(0, tr("function [default]"));
            for (int i=0; i< addplots; i++) {
                ui->cmbQFFitFunctionSubplot->setItemText(1+i, ff->transformParametersForAdditionalPlot(i, p));
            }
        }
        fitfuncValuesTable->setWriteTo(&fitfuncValues, ff, true);
        //qDebug()<<"4: fitfuncErrors.size()="<<fitfuncErrors.size()<<"   fitfuncValues.size()="<<fitfuncValues.size();
        if (fitfuncErrors.size()==fitfuncValues.size()) {
            fitfuncValuesTable->setAuxiliaryWriteTo(&fitfuncErrors, NULL, NULL, NULL);
        }

    }
    fitfuncValuesTable->setEditErrors(true);
    fitfuncValuesTable->setUserEditErrors(false);
    updatePlotWidgetVisibility();
    writeGraphData();
}

void QFTableGraphSettings::on_btnAutoImageSizes_clicked()
{

    if (ui->cmbGraphType->currentIndex()==11 || ui->cmbGraphType->currentIndex()==12 || ui->cmbGraphType->currentIndex()==13) {
        if (current->model()->hasColumnHeaderData(ui->cmbLinesXData->currentData().toInt(), QFRDRTable::ColumnImagePixWidth)) ui->spinImageWidth->setValue(current->model()->getColumnHeaderData(ui->cmbLinesXData->currentData().toInt(), QFRDRTable::ColumnImagePixWidth).toInt());
        if (current->model()->hasColumnHeaderData(ui->cmbLinesXData->currentData().toInt(), QFRDRTable::ColumnImageWidth)) ui->edtImageWidth->setValue(current->model()->getColumnHeaderData(ui->cmbLinesXData->currentData().toInt(), QFRDRTable::ColumnImageWidth).toDouble());
        if (current->model()->hasColumnHeaderData(ui->cmbLinesXData->currentData().toInt(), QFRDRTable::ColumnImageHeight)) ui->edtImageHeight->setValue(current->model()->getColumnHeaderData(ui->cmbLinesXData->currentData().toInt(), QFRDRTable::ColumnImageHeight).toDouble());
        if (current->model()->hasColumnHeaderData(ui->cmbLinesXData->currentData().toInt(), QFRDRTable::ColumnImageX)) ui->edtImageX->setValue(current->model()->getColumnHeaderData(ui->cmbLinesXData->currentData().toInt(), QFRDRTable::ColumnImageX).toDouble());
        if (current->model()->hasColumnHeaderData(ui->cmbLinesXData->currentData().toInt(), QFRDRTable::ColumnImageY)) ui->edtImageY->setValue(current->model()->getColumnHeaderData(ui->cmbLinesXData->currentData().toInt(), QFRDRTable::ColumnImageY).toDouble());

    }
}

void QFTableGraphSettings::on_btnFunctionHelp_clicked()
{
     if (current)
         QFPluginServices::getInstance()->displayHelpWindow(QFPluginServices::getInstance()->getPluginHelpDirectory("table")+"/mathparser.html");
}

