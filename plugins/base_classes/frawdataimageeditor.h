/*
    Copyright (c) 2008-2014 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center

    last modification: $LastChangedDate$  (revision $Rev$)

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef FRAWDATAIMAGEEDITOR_H
#define FRAWDATAIMAGEEDITOR_H

#include "qfrawdataeditor.h"
#include <QLabel>
#include "qfplotter.h"
#include <QCheckBox>
#include <QPushButton>
#include <QListWidget>
#include "qvisiblehandlesplitter.h"
#include <QSplitter>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QSpinBox>
#include <QAbstractTableModel>
#include <QTabWidget>
#include "datacutslider.h"
#include <QSet>
#include "qfdoubleedit.h"
#include "qtriple.h"
#include "qfhtmldelegate.h"
#include <QTableView>
#include "qffitfunction.h"
#include "qenhancedtableview.h"
#include "qftablemodel.h"
#include "qfhistogramview.h"
#include "jkqtptools.h"
#include <QFrame>
#include <QFile>
#include <QToolBar>
#include "jkverticalscrollarea.h"
#include "qfcorrelationmasktools.h"
#include "qfrdrtableinterface.h"
#include "qfrdrcolumngraphsinterface.h"
#include "qfplottercopytotabledialog.h"
#include "qfimageplotter.h"
#include "qfimageparametergroupbox.h"
#include "qfoverlaystylecombobox.h"
#include "qfparametercorrelationview.h"


class FRawDataImageEditor : public QFRawDataEditor {
        Q_OBJECT
    public:
        enum ImageTransforms {
            itNone=0,
            itAbs=1,
            itLog=2,
            itReciprocal=3,
            itSqrt=4
        };


        FRawDataImageEditor(const QString &configprefix, QFPluginServices* services, QFRawDataPropertyEditor* propEditor, QWidget* parent);

        virtual ~FRawDataImageEditor();

    protected:
        /** �brief create additional widgets here */
        virtual void createAdditionalWidgets() {}
        /** \brief read the additional settings */
        virtual void readAdditionalSettings() {}
        /** \brief write the additional settings */
        virtual void writeAdditionalSettings() {}

        virtual void createReportDoc(QTextDocument* document);

        /** \brief returns true, if a given run/pixel is inside the second DualView channel */
        virtual bool indexIsDualView2(int32_t sel);



        /** \brief return the currently selected evalGroup (or an empty string, if none selected) */
        virtual QString currentEvalGroup() const;
        /** \brief return the currently selected fit parameter (or an empty string, if none selected, or no evalGroup selected) */
        virtual QString currentFitParameter() const;
        /** \brief return the currently selected goodnes of fit parameter (or an empty string, if none selected, or no evalGroup selected) */
        virtual QString currentFit2Parameter() const;
        /** \brief return the currently selected fit parameter transformation */
        virtual ImageTransforms currentFitParameterTransfrom() const;
        /** \brief return the currently selected goodnes of fit parameter transformation */
        virtual ImageTransforms currentFit2ParameterTransfrom() const;

        virtual QString formatTransformAndParameter(QComboBox* cmbParameter, QComboBox* cmbTransform);


        void selectedInsert(int idx);
        void selectedRemove(int idx);


        /** \brief read the settings */
        void readSettings();
        /** \brief write the settings */
        void writeSettings();


        void excludeByImage(double *imageIn);

        void getCurrentResultNamesAndLabels(QStringList& names, QStringList& labels);


        /** \brief create a parameter image with the given evalGroup and fitParam */
        void readParameterImage(double* image, uint32_t width, uint32_t height, QString evalGroup, QString fitParam, ImageTransforms tranFitParam, bool thisRDR=true, const QString& otherRDRRole=QString(""), const QString& otherRDRevalGroup=QString(""));

        /** \brief apply the given transformation to the image */
        void transformImage(double *image, uint32_t width, uint32_t height, FRawDataImageEditor::ImageTransforms tranFitParam);

        /*! \brief evaluate the fit function (with parameters) as defined by the pair evalGroup and evaluation

            \param tau lag times where to evaluate the fit function
            \param[out] fit the evaluated wit function is written here, this is only filled if both \a tau and \a fit are non-NULL
            \param N size of tau and fit
            \param[out] names is filled with the names of the parameters
            \param[out] namelabels is filled with the name labels of the parameters
            \param[out] values is filled with the values of the parameters
            \param[out] errors is filled with the errors of the parameters
            \param[out] fix is filled with the fix-states of the parameters
            \param[out] units is filled with the units of the parameters
            \param[out] unitlabels is filled with the unit labels of the parameters
            \param evaluation specifies the parameters to use
            \param index for which run should we print
            \return If not all fit needed parameters are available, this function retuns \c false and does
                    not output data in fit. If the function is well defined it is evaluated for every given
                    tau. The result is saved in fit.
         */
        bool evaluateFitFunction(QFRawDataRecord* current, const double* tau, double* fit, uint32_t N, QStringList& names, QStringList& namelabels, QList<double>& values, QList<double>& errors, QList<bool>& fix, QStringList& units, QStringList& unitlabels, QString evaluation, int index);

        /*! \brief fill a QComboBox with the names (and IDs as data) of all plotable fit parameters */
        void fillParameterComboBox(QComboBox* cmbParameter, QFRawDataRecord* m, const QString& egroup, const QStringList &param1Default, const QString& otherParameter=QString(), const QString& indexPropertyName=QString(""), const QString& parameterDefault=QString("fitparam_n_particle"), int idxDefault=0);

        /*! \brief returns the RDR from which to read fit parameter 2 */
        QFRawDataRecord* getRDRForParameter2(const QString& otherRDRRole) const;


        QString selectionToString();

        void loadSelectionFromString(QString sel);


    protected slots:

        /** \brief switch plots on and off according to selected visible plots */
        void showHidePlots();
        /** \brief move color bars to appropriate position */
        void moveColorbarsAuto();

        /** \brief connected to the rawDataChanged() signal of the current record */
        virtual void rawDataChanged();

        virtual void maskChanged();
        /** \brief update selection etz. after click on image */
        void updateAfterClick();
        /** \brief connect widgets to current data record */
        virtual void connectWidgets(QFRawDataRecord* current, QFRawDataRecord* old);


        /** \brief replot histogram */
        void replotHistogram();

        /** \brief recalculate histogram over all pixels */
        void updateHistogram();

        /** \brief fill the given histogram widget with data */
        void updateHistogram(QFHistogramView *histogram, QFRDRRunSelectionsInterface *m, double *plteImageData, int32_t plteImageSize, bool excludeExcluded, bool dv, bool selHistogram);
        /** \brief fill the given correlation widget with data */
        void updateCorrelation(QFParameterCorrelationView *corrView, QFRawDataRecord *m, double *data1, double *data2, int32_t plteImageSize, bool excludeExcluded, bool dv, bool selHistogram, int mode, int channel=0, const QString& label1=QString(), const QString label2=QString(), int width=0, int height=0);


        /** \brief recalculate histogram over selected pixels */
        void updateSelectionHistogram(bool replot=false);


        /** \brief displays the data from the current data element in the plotter widget
         *
         * the parameter dummy has no function it is only there so this function may be used
         * as a slot for a signal with an integer parameter.
         */
        void replotData();


        /** \brief draw overview plot */
        void replotImage();

        /** \brief replot the selection displays only */
        void replotSelection(bool replot=true);

        /** \brief fill the selection/expluded arrays for the overlays, does not plot */
        void updateSelectionArrays();

        /** \brief draw overview plot */
        void replotOverview();
        /** \brief draw mask plot */
        void replotMask();
        /** \brief excludes the currently selected items from the evaluation and recalculates the average */
        void excludeRuns();
        /** \brief includes the currently selected items from the evaluation and recalculates the average */
        void includeRuns();
        /** \brief exclude runs by intensity in the overview image */
        void excludeByIntensity();
        /** \brief exclude runs by intensity in the parameter image */
        void excludeByParamIntensity();
        /** \brief exclude runs by intensity in the parameter image 2 */
        void excludeByParan2Intensity();

        /** \brief save selection to harddisk */
        void saveSelection();
        /** \brief load a selection from harddisk */
        void loadSelection();
        /** \brief copy selection to clipboard */
        void copySelection();
        /** \brief paste a selection from clipboard */
        void pasteSelection();

        /** \brief delete mask */
        void includeAll();
        /** \brief invert mask */
        void invertMask();
        void slidersChanged(int userMin, int userMax, int min, int max);
        void imageClicked(double x, double y, Qt::KeyboardModifiers modifiers);
        void imageScribbled(double x, double y, Qt::KeyboardModifiers modifiers, bool first, bool last);
        void imageMouseMoved(double x, double y);
        void imageRectangleFinished(double x, double y, double width, double height, Qt::KeyboardModifiers modifiers);
        void imageLineFinished(double x1, double y1, double x2, double y2, Qt::KeyboardModifiers modifiers);
        void imageCircleFinished(double x, double y, double radius, Qt::KeyboardModifiers modifiers);
        void imageEllipseFinished(double x, double y, double radiusX, double radiusY, Qt::KeyboardModifiers modifiers);
        void setImageEditMode();


        /** \brief activated when the user selects a new parameter set/evaluation group,  fills cmbParameters with all available parameters in the selected result group */
        void parameterSetChanged();
        /** \brief activated when the user selects a new parameter*/
        void parameterChanged();
        /** \brief called when the user selects a new transformation */
        void transformChanged();
        /** \brief clear the parameter image */
        void clearImage();
        /** \brief fills cmbResultGroup with all available result groups */
        void fillParameterSet();
        /** \brief connect/disconnect cmbResultGroups, cmbParameters, ... to their slots */
        void connectParameterWidgets(bool connectTo=true);
        /** \brief connect/disconnect widgets for image settings to their slots */
        void connectImageWidgets(bool connectTo=true);
        /** \brief called when the user selects a new palette */
        void ovrPaletteChanged();
        void histogramSettingsChanged();
        /** \brief called when the user selects to display overlays or not */
        void updateOverlaySettings();

        /** \brief save the image plot settings to the raw data record properties */
        void saveImageSettings();
        /** \brief load the image plot settings from the raw data record properties */
        void loadImageSettings();

        void imageZoomChangedLocally(double newxmin, double newxmax, double newymin, double newymax, JKQtPlotter* sender);


        /*! \brief called when the current's record's fit results change

            This function updates the contents of the image setting comboboxes, but tries to preserve the current selection!
         */
        void resultsChanged(const QString &evalName=QString(""), const QString &resultName=QString(""), bool deleted=false);

        void debugInfo();

        /** \brief save a report of the evaluation results */
        void saveReport();
        /** \brief save the displayed images as data files (TIFF, CSV, ...) */
        void saveData();
        /** \brief copy displayed images to Matlab */
        void copyToMatlab();
        /** \brief copy the shown images as columns of values */
        void copyDataAsColumns();
        /** \brief print a report of the evaluation results */
        void printReport();

        /** \brief copy image data into datastores of the image plots, so they can be copied to other applications. */
        void setCopyableData();

        void updateSelectionCombobox();

        void selectedSelectionInCombo(int index);
        void storeSelection();
        void deleteSelection();
        void selectionEdited();


        void copyFitResultStatistics();
        void copyMaskToAll();


    protected:
        /** \brief store a list of default fit parameters for image 1 in this */
        QStringList param1Default;
        /** \brief store a list of default fit parameters for image 1 in this */
        QStringList param2Default;

        /** \brief set this variable to a value, like "imfcs" to separate the config from other plugins using this widget */
        QString configprefix;


        /** \brief map with all available fit functions */
        QMap<QString, QFFitFunction*> m_fitFunctions;

        int connectParameterWidgetsCounter;
        int connectImageWidgetsCounter;

        /** \brief indicates whether the overview plot is visible */
        QCheckBox* chkOverviewVisible;
        /** \brief indicates whether the goodnes of fit plot is visible */
        QCheckBox* chkParamImage2Visible;
        /** \brief indicates whether the mask plot is visible */
        QCheckBox* chkMaskVisible;
        /** \brief checkbox to select whether to display selected/excluded pixels overlay on image plot */
        QCheckBox* chkDisplayImageOverlay;

        /** \brief a combobox to select how to display the selected pixel */
        QFOverlayStyleCombobox* cmbSelectionStyle;


        /** \brief splitter between top plots */
        QVisibleHandleSplitter* splitterTop;
        /** \brief splitter between bottom plot and parameter table */
        QVisibleHandleSplitter* splitterBot;
        /** \brief splitter between top plots and bottom plots */
        QVisibleHandleSplitter* splitterTopBot;


        QFImageParameterGroupBox* grpImage;
        QFImageParameterGroupBox* grpImage2;


        /** \brief label over the parameter image plot */
        QLabel* labParamImage;

        /** \brief  plotter for overview image */
        QFPlotter* pltOverview;
        /** \brief plot for the overview image in pltOverview */
        JKQTPMathImage* plteOverview;
        JKQTPRGBMathImage* plteOverviewRGB;
        /** \brief plot for the selected runs in pltOverview, plot plteOverviewSelectedData */
        JKQTPOverlayImageEnhanced* plteOverviewSelected;
        /** \brief plot for the excluded runs in pltOverview, plot plteOverviewSelectedData */
        JKQTPOverlayImageEnhanced* plteOverviewExcluded;
        /** \brief data in plteOverviewSelected */
        bool* plteOverviewSelectedData;
        /** \brief data in plteOverviewExcluded */
        bool* plteOverviewExcludedData;
        /** \brief size of plteOverviewSelectedData */
        int plteOverviewSize;

        /** \brief  plotter for mask image */
        QFPlotter* pltMask;
        /** \brief plot for the mask image in pltMask */
        JKQTPOverlayImage* plteMask;
        /** \brief plot for the selected runs in pltOverview, plot plteOverviewSelectedData */
        JKQTPOverlayImageEnhanced* plteMaskSelected;

        /** \brief  plotter for parameter image */
        QFImagePlotter* pltImage;

        int32_t plteImageSize;



        /** \brief  plotter for goodnes of fit image */
        QFImagePlotter* pltParamImage2;



        /** \brief combobox for the color bar of overview image */
        JKQTPMathImageColorPaletteComboBox* cmbColorbarOverview;
        QCheckBox* chkAutorangeOverview;
        QFDoubleEdit* edtOvrMin;
        QFDoubleEdit* edtOvrMax;
        QFDoubleEdit* edtOvr2Min;
        QFDoubleEdit* edtOvr2Max;




        /** \brief toolbar to work with parameter images */
        QToolBar* tbParameterImage;
        QLabel* labImagePositionDisplay;
        QLabel* labImageAvg;
        QActionGroup* agImageSelectionActions;

        QAction* actImagesZoom;
        QAction* actImagesDrawPoints;
        QAction* actImagesDrawRectangle;
        QAction* actImagesDrawCircle;
        QAction* actImagesDrawEllipse;
        QAction* actImagesDrawLine;
        QAction* actImagesScribble;
        QAction* actInsertSelectedCorrelationsAsFCSRDR;
        QAction* actCopyFitResultStatistics;
        QComboBox* cmbMaskEditMode;
        QAction* actCopyMaskToAll;

        /** \brief combobox to select a result group */
        QComboBox* cmbResultGroup;


        /** \brief combobox to select a parameter from the result group */
        QComboBox* cmbParameter;
        QLabel* labParameter;
        QLabel* labParameterTransform;
        QComboBox* cmbParameterTransform;
        /** \brief combobox to select a goodnes of fit parameter from the result group */
        QComboBox* cmbParameter2;
        QLabel* labParameter2;
        QLabel* labParameter2Transform;
        QComboBox* cmbParameter2Transform;

        /** \brief table for the fit params */
        QEnhancedTableView* tvParams;

        QFTableModel* tabFitvals;


        /** \brief combobox to select a stored selection */
        QComboBox* cmbStoredSelections;
        QToolButton* btnSaveSelectionToStored;
        QToolButton* btnDeleteStoredSelection;
        QAction* actSaveSelectionToStored;
        QAction* actDeleteStoredSelection;

        QToolButton* btnCopySelection;
        QToolButton* btnPasteSelection;
        QToolButton* btnCopyMask;
        QToolButton* btnPasteMask;
        QAction* actCopySelection;
        QAction* actPasteSelection;



        /** \brief plotter widget for the parameter histogram */
        QGridLayout* histLay;
        QFHistogramView* histogram;
        QCheckBox* chkExcludeExcludedRunsFromHistogram;
        QFHistogramView* histogram2;
        QCheckBox* chkExcludeExcludedRunsFromHistogram2;

        QComboBox* cmbCorrelationDisplayMode;
        QSpinBox* spinCorrelationChannel;
        QFParameterCorrelationView* corrView;

        QColor selectionColor;
        QColor excludedColor;


        QCheckBox* chkOtherFileP2;
        QComboBox* cmbOtherFilesResultGroup;
        QComboBox* cmbOtherFileRole;




        /** \brief tab widget to select whether to display ACF, histogram, ... */
        QTabWidget* tabDisplay;



        /** \brief set which contains all currently selected runs */
        QSet<int32_t> selected;


        QString lastSavePath;

        QTimer* timUpdateAfterClick;

        QString lastMaskDir;








        /** \brief a button which excludes the selected runs from the evaluation */
        QToolButton* btnDontUse;
        /** \brief a button which includes the selected runs to the evaluation */
        QToolButton* btnUse;
        /** \brief a button which includes all runs to the evaluation */
        QToolButton* btnUseAll;
        /** \brief a button which inverts the mask */
        QToolButton* btnInvertMask;
        /** \brief button to mask pixels by intensity */
        QToolButton* btnMaskByIntensity;
        /** \brief button to mask pixels by parameter image 2 */
        QToolButton* btnMaskByParam2Intensity;
        /** \brief button to mask pixels by parameter image */
        QToolButton* btnMaskByParamIntensity;

        QToolButton* btnLoadSelection;
        QToolButton* btnLoadMask;
        QToolButton* btnSaveSelection;
        QToolButton* btnSaveMask;
        /** \brief action to print a report */
        QToolButton* btnPrintReport;

        /** \brief action to save a report */
        QToolButton* btnSaveReport;
        /** \brief action to save image data */
        QToolButton* btnSaveData;
        /** \brief action to copy image data to Matlab */
        QToolButton* btnCopyDataToMatlab;
        /** \brief action to copy image data as column table (CSV) */
        QToolButton* btnCopyDataAsColumns;






        /** \brief a button which excludes the selected runs from the evaluation */
        QAction* actDontUse;
        /** \brief a button which includes the selected runs to the evaluation */
        QAction* actUse;
        /** \brief a button which includes all runs to the evaluation */
        QAction* actClearMask;
        /** \brief a button which inverts the mask */
        QAction* actInvertMask;
        /** \brief button to mask pixels by intensity */
        QAction* actMaskByIntensity;
        /** \brief button to mask pixels by parameter image 2 */
        QAction* actMaskByParam2Intensity;
        /** \brief button to mask pixels by parameter image */
        QAction* actMaskByParamIntensity;

        QAction* actLoadSelection;
        QAction* actSaveSelection;
        /** \brief action to print a report */
        QAction* actPrintReport;

        /** \brief action to save a report */
        QAction* actSaveReport;
        /** \brief action to save image data */
        QAction* actSaveData;
        /** \brief action to copy image data to Matlab */
        QAction* actCopyDataToMatlab;
        /** \brief action to copy image data to Matlab */
        QAction* actCopyDataAsColumns;


        QMenu* menuData;
        QMenu* menuMask;
        QMenu* menuSelection;

        /** \brief this should be set by the user !!! */
        QFRDRImageMaskEditTools* correlationMaskTools;

    private:
        /** \brief create widgets on object creation */
        void createWidgets();
};

#endif // FRAWDATAIMAGEEDITOR_H
