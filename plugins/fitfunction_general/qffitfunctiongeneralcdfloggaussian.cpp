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


#include "qffitfunctiongeneralcdfloggaussian.h"
#include "qfmathtools.h"
#include <cmath>
#include "statistics_tools.h"

QFFitFunctionGeneralCDFLogGaussian::QFFitFunctionGeneralCDFLogGaussian():
    QFCDFFitFunctionBase(1.0/5.0, true)
{
    //           type,         id,                        name,                                                    label (HTML),                      unit,          unitlabel (HTML),        fit,       userEditable, userRangeEditable, displayError, initialFix,                initialValue, minValue, maxValue, inc, absMin, absMax
    addParameter(FloatNumber,  "position",                "position",                                              "X<sub>0</sub>",                        "",         "",                   true,      true,         true,              QFFitFunction::DisplayError,       false, 0,            -1e10,    1e10,  1  );
    #define PARAM_POSITION 0
    addParameter(FloatNumber,  "width",                   "width",                                        "&sigma;",                           "",         "",                   true,      true,         true,              QFFitFunction::DisplayError,       false, 1,            1e-10,    1e10,  1  );
    #define PARAM_WIDTH 1

}

double QFFitFunctionGeneralCDFLogGaussian::evaluate(double t, const double* data) const {
    const double position=data[PARAM_POSITION];
    const double width=data[PARAM_WIDTH];
    return 0.5*(1.0+erf((log(t)-log(position))/sqrt(2.0*qfSqr(width))));
}


void QFFitFunctionGeneralCDFLogGaussian::calcParameter(double* /*data*/, double* /*error*/) const {
}

bool QFFitFunctionGeneralCDFLogGaussian::isParameterVisible(int parameter, const double* data) const {

    Q_UNUSED(parameter);
    Q_UNUSED(data);
    return true;
    // all parameters are visible at all times
}

unsigned int QFFitFunctionGeneralCDFLogGaussian::getAdditionalPlotCount(const double* params) const {

    Q_UNUSED(params);
    return 0;
    // we have one additional plot
}

QString QFFitFunctionGeneralCDFLogGaussian::transformParametersForAdditionalPlot(int plot, double* params) const {


    Q_UNUSED(plot);
    Q_UNUSED(params);
    return "";
}

bool QFFitFunctionGeneralCDFLogGaussian::get_implementsDerivatives() const
{
    return false;
}

