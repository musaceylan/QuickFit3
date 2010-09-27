#ifndef QFPROJECTRAWDATAMODEL_H
#define QFPROJECTRAWDATAMODEL_H

#include <QAbstractListModel>
#include "qfproject.h"


/*! \brief this class implements a data model which contains all
           raw data items in the project. This is used by the QFEvaluationPropertyEditor
           to display a list of all files that may be added to the evaluation item.

    This data model is drag/drop enabled. Items may be draged out and will be exported as
    a list of their IDs (MIME: "application/qf2.ids.int.list"). If data is droped into this
    model simply nothing happens. This allows to use two widgets next to each other:
      - Drag from this model to the other allows to include items there.
      - Drag to this model allows to delete selected items from the other list.
    .

    Also this model returns in the first Qt::UserRole the ID of the raw data record in
    every row.

    \note You can use QFMultiListEvaluationPropertyEditor::QFProjectRawDataModelMultiListEvaluationProxy
          to filter out records that are not usable together with a given QFMultiListEvaluation.
 */
class QFProjectRawDataModel : public QAbstractListModel {
        Q_OBJECT
    public:
        /** Default constructor */
        QFProjectRawDataModel(QFProject* parent);
        /** Default destructor */
        virtual ~QFProjectRawDataModel();

        void setProject(QFProject* parent);

        virtual QVariant data(const QModelIndex &index, int role) const;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    protected:
        /** \brief pointer to the owning evaluation item */
        QFProject* item;
        /** \brief name of the MIME type used for Drag&Drop actions */
        QString defaultMimeType;
    protected slots:
        /** \brief connected to the propertiesChanged() slot of the owning QFEvaluationItem. Used to
         *         tell the model that something has changed */
        void projectChanged(bool data);
    private:
};


#endif // QFPROJECTRAWDATAMODEL_H
