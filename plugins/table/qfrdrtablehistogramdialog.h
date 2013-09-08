#ifndef QFRDRTABLEHISTOGRAMDIALOG_H
#define QFRDRTABLEHISTOGRAMDIALOG_H

#include <QDialog>
#include "qfproperties.h"
#include <QMap>
#include <QList>
#include <QStringList>
#include "qftablepluginmodel.h"

namespace Ui {
    class QFRDRTableHistogramDialog;
}

class QFRDRTableHistogramDialog : public QDialog
{
        Q_OBJECT
        
    public:
        explicit QFRDRTableHistogramDialog(QWidget *parent = 0);
        ~QFRDRTableHistogramDialog();

        void setTable(QFTablePluginModel* table);
        void setSelectedColumn(int col);
        void init();


        QMap<int, QVector<double> > getHistograms() const;
        QMap<int, QString> getHeaderNames() const;
    protected slots:
        void updateHistograms(bool estimateBinWidth=false);
        void on_edtRangeMax_valueChanged(double value);
        void on_edtRangeMin_valueChanged(double value);
        void on_edtBinWidth_valueChanged(double value);
        void on_spinBins_valueChanged(int value);
        void on_cmbMethod_currentIndexChanged(int current);
        void on_btnAutoBinwidth_clicked();
        void on_btnAutoBandwidth_clicked();
        void on_btnAutoBins_clicked();
        void calcAutoBinwidth();
        void calcAutoBandwidth();
        void calcAutoBins();
    private:
        Ui::QFRDRTableHistogramDialog *ui;

        QMap<int, QVector<double> > histograms;
        QMap<int, QString> headerNames;
        struct colStatistics {
            double min;
            double max;
            double q25;
            double q75;
            double mean;
            double median;
            double var;
            int values;
        };
        colStatistics stat;
        //QMap<int, colStatistics> columnStatistics;
        int selected;
        QFTablePluginModel* table;
};

#endif // QFRDRTABLEHISTOGRAMDIALOG_H
