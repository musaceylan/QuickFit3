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

#ifndef QFFitFunctionsSPIMFCSDiffCE2E2_H
#define QFFitFunctionsSPIMFCSDiffCE2E2_H
#include "qfpluginfitfunction.h"
#include "qfgeneralfitfunctionbase.h"



/*! \brief QFFitFunction class for a SPIM-FCS fit model with pure diffusion and lateral 1/e^2 width
    \ingroup qf3fitfunp_fitfunctions_spimfcs

    \f[ g(\tau)=G_\infty+\frac{1}{N/V_{\text{eff}}}\cdot\frac{1}{\sqrt{\pi}\cdot w_za^2}\cdot\sum\limits_{i=1}^3\rho_i\cdot\left[\mbox{erf}\left(\frac{a}{\sqrt{4D_i\tau+w_{xy}^2}}\right)+\sqrt{4D_i\tau+w_{xy}^2}\cdot\left[\exp\left(-\frac{a^2}{4D_i\tau+w_{xy}^2}\right)-1\right]\right]^2\cdot\left[1+\frac{4D_i\tau}{w_z^2}\right]^{-1/2} \f]

    \f[ V_{\text{eff}}=\frac{\sqrt{\pi}\cdot a^2w_z}{\left(\mbox{erf}\left(\frac{a}{w_{xy}}\right)+\frac{w_{xy}}{\sqrt{\pi}\cdot a}\cdot\left(e^{-(a/w_{xy})^2}-1\right)\right)^2} \f]
*/
//\f[ \Delta V_{\text{eff}}=\sqrt{\left(\Delta\sigma_z\cdot \pi^{3/2}a^2\right)^2+\left(\Delta a\cdot 2\pi^{3/2}a\sigma_z\right)^2} \f]
//\f[ C=\frac{N}{V_{\text{eff}}}=\frac{N}{\pi^{3/2}\cdot a^2\cdot \sigma_z} \f]
//\f[ \Delta C=\sqrt{\left(\frac{\Delta N}{\pi^{3/2}a^2\cdot\sigma_z}\right)^2+\left(\frac{\Delta\sigma_z\cdot N}{\pi^{3/2}a^2\sigma_z^2}\right)^2+\left(\frac{\Delta a\cdot 2\cdot N}{\pi^{3/2}a^3\cdot\sigma_z}\right)^2} \f]

class QFFitFunctionsSPIMFCSDiffCE2: public QFFCSLargeFocusFitFunctionBase {
    public:
        QFFitFunctionsSPIMFCSDiffCE2();
        virtual ~QFFitFunctionsSPIMFCSDiffCE2() {}
        /*! \copydoc QFFitFunction::name()   */
        virtual QString name() const { return QString("SPIM-FCS (camera): 3D Normal Diffusion: c, D as parameters (rect. pixel, 1/e^2 radii)"); }
        /*! \copydoc QFFitFunction::category()   */
        virtual QString category() const { return QObject::tr("SPIM-FCS (camera)"); }

        /*! \copydoc QFFitFunction::id()   */
        virtual QString id() const { return QString("fcs_spim_diffce2"); }

        virtual bool isDeprecated() const { return true; };

        /*! \copydoc QFFitFunction::evaluate()   */
        virtual double evaluate(double t, const double* parameters) const;

        /*! \copydoc QFFitFunction::calcParameter()   */
        virtual void calcParameter(double* parameterValues, double* error=NULL) const;

        /*! \copydoc QFFitFunction::isParameterVisible()   */
        virtual bool isParameterVisible(int parameter, const double* parameterValues) const;
        /*! \copydoc QFFitFunction::getAdditionalPlotCount()   */
        virtual unsigned int getAdditionalPlotCount(const double* params) const;

        /*! \copydoc QFFitFunction::transformParametersForAdditionalPlot()   */
        virtual QString transformParametersForAdditionalPlot(int plot, double* params) const;

        /*! \copydoc QFFitFunction::get_implementsDerivatives()   */
        virtual bool get_implementsDerivatives() const { return false; }

        /*! \copydoc QFFitFunction::evaluateDerivatives()   */
        //virtual void evaluateDerivatives(double* derivatives, double t, const double* parameters) const ;

        /*! \copydoc QFFitFunction::sortParameter()   */
        virtual void sortParameter(double* parameterValues, double* error=NULL , bool *fix=NULL) const;
};

#endif // QFFitFunctionsSPIMFCSDiffCE2E2_H
