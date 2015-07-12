#ifndef QFRDRIMAGINGFCSWIZARD_H
#define QFRDRIMAGINGFCSWIZARD_H

#include <QWidget>
#include "qfwizard.h"
#include "qfselectfileslistwidget.h"
#include "qfimageplot.h"
#include "qfrdrimagingfcscorrelationjobthread.h"


class QFRDRImagingFCSWizard : public QFWizard
{
    public:
        QFRDRImagingFCSWizard(QDialog* parent=NULL);
        ~QFRDRImagingFCSWizard();
    protected:
        QFSelectFilesListWizardPage* wizSelfiles;
        QFImagePlotWizardPage* wizLSAnalysisImgPreview;
        QStringList imageFilters;
        QStringList imageFormatNames;
};

#endif // QFRDRIMAGINGFCSWIZARD_H
