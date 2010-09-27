#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QtGui>
#include "ui_optionsdialog.h"


class OptionsDialog : public QDialog, public Ui::OptionsDlg {
        Q_OBJECT
    public:
        /** Default constructor */
        OptionsDialog(QWidget* parent=NULL);
        /** Default destructor */
        virtual ~OptionsDialog();
    protected slots:
        void on_cmbStylesheet_currentIndexChanged( const QString & text );
        void on_cmbStylesheet_highlighted( const QString & text );
        void on_cmbStyle_currentIndexChanged( const QString & text );
        void on_cmbStyle_highlighted( const QString & text );
    private:
};

#endif // OPTIONSDIALOG_H
