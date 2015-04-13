/*
Copyright (c) 2008-2014 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    last modification: $LastChangedDate$  (revision $Rev$)

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License (LGPL) as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License (LGPL) for more details.

    You should have received a copy of the GNU Lesser General Public License (LGPL)
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DLGSELECTPROJECTSUBSET_H
#define DLGSELECTPROJECTSUBSET_H

#include <QDialog>
#include "qfproject.h"
#include "qfprojectrawdatamodel.h"

namespace Ui {
    class DlgSelectProjectSubset;
}

class DlgSelectProjectSubset : public QDialog
{
        Q_OBJECT
        
    public:
        explicit DlgSelectProjectSubset(QFProject* project, QWidget *parent = 0);
        ~DlgSelectProjectSubset();
    protected slots:
        void showHelp();
        
    private:
        Ui::DlgSelectProjectSubset *ui;
        QFProject* project;
};

#endif // DLGSELECTPROJECTSUBSET_H
