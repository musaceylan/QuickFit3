/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    

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

#include "qffitalgorithmthreaddedfit.h"
#include <QTime>

QFFitAlgorithmThreadedFit::QFFitAlgorithmThreadedFit(QObject* parent): QThread(parent)
{
    //ctor
    guess_params=false;

}

QFFitAlgorithmThreadedFit::~QFFitAlgorithmThreadedFit()
{
    //dtor
}

void QFFitAlgorithmThreadedFit::init(QFFitAlgorithm* algorithm, double* paramsOut, double* paramErrorsOut, double* dataX, double* dataY, double* dataWeight, uint64_t N, QFFitFunction* model, double* initialParams, bool* fixParams, double* paramsMin, double* paramsMax, int repeats, bool fitLogY, QVector<double> *COV) {
    this->algorithm=algorithm;
    this->paramsOut=paramsOut;
    this->paramErrorsOut=paramErrorsOut;
    this->dataX=dataX;
    this->dataY=dataY;
    this->dataWeight=dataWeight;
    this->N=N;
    this->model=model;
    this->initialParams=initialParams;
    this->fixParams=fixParams;
    this->paramsMin=paramsMin;
    this->paramsMax=paramsMax;
    this->repeats=repeats;
    this->fitLogY=fitLogY;
    this->COV=COV;
    guess_params=false;
}

void QFFitAlgorithmThreadedFit::setGuessParamsOnly(bool guess)
{
    guess_params=guess;
};

void QFFitAlgorithmThreadedFit::run() {
    QTime tstart=QTime::currentTime();
    double* init=duplicateArray(initialParams, model->paramCount());
    if (guess_params) {
        guessm=tr("parameter-guessing failed");
        guessok=false;
        if (model->estimateInitial(init,dataX, dataY,N, fixParams)) {
            //copyArray(paramsOut, init, model->paramCount());
            for (int i=0; i<model->paramCount(); i++) {
                if (!fixParams || !fixParams[i]) {
                    paramsOut[i]=init[i];
                }

                paramErrorsOut[i]=0;
            }
            guessm=tr("parameter-guessing successfull");
            guessok=true;
        }
    } else {
        for (int i=0; i<repeats; i++) {
            result=algorithm->fit(paramsOut, paramErrorsOut, dataX, dataY, dataWeight, N, model, init, fixParams, paramsMin, paramsMax, fitLogY, NULL, COV);
            copyArray(init, paramsOut, model->paramCount());
        }
    }
    qfFree(init);
     deltaTime=(double)tstart.msecsTo(QTime::currentTime());
};



