/*
Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

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

#ifndef DLGCOMPONENTINFO_H
#define DLGCOMPONENTINFO_H

#include "qfdialog.h"
#include "qftablemodel.h"

class QFEDiffusionCoefficientCalculator; // forward
namespace Ui {
    class DlgComponentInfo;
}

class DlgComponentInfo : public QFDialog
{
        Q_OBJECT
        
    public:
        explicit DlgComponentInfo(QFEDiffusionCoefficientCalculator* plugin, int component, QWidget *parent = 0);
        ~DlgComponentInfo();

    public slots:
        void setComponent(int component);
        void showHelp();
        void on_cmbPlot_currentIndexChanged(int index);
        void on_btnMailData_clicked();
    private:
        Ui::DlgComponentInfo *ui;
        QFEDiffusionCoefficientCalculator* plugin;
        QFTableModel* model;
};

#endif // DLGCOMPONENTINFO_H
