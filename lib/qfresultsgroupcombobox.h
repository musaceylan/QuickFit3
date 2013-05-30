#ifndef QFRESULTSGROUPCOMBOBOX_H
#define QFRESULTSGROUPCOMBOBOX_H

#include <QComboBox>
#include "qfrawdatarecord.h"
#include "lib_imexport.h"
#include <QRegExp>


/*! \brief combobox that displays a list of all revaluation groups of a specified QFRawDataRecord
    \ingroup qf3lib_tools

  */
class QFLIB_EXPORT QFResultsGroupComboBox : public QComboBox {
        Q_OBJECT
    public:
        explicit QFResultsGroupComboBox(QWidget *parent = 0);

        QString currentEvaluationGroup() const;
        void setGroupFilter(bool enabled, const QRegExp& filter=QRegExp());
        void setContainedParam(const QString& param);
    public slots:
        void setCurrentEvaluationGroup(const QString& group);
        void setRDR(QFRawDataRecord *rdr);
    signals:
        void currentEvaluationGroupChanged(QString egroup);
    protected slots:
        void myCurrentIndexChanged(int i);
        void refill();
    protected:
        QPointer<QFRawDataRecord> rdr;
        QStringList egroups;
        QStringList egnames;
        QRegExp rxFilter;
        bool doFilter;
        QString containsParamInGroup;
};

#endif // QFRESULTSGROUPCOMBOBOX_H
