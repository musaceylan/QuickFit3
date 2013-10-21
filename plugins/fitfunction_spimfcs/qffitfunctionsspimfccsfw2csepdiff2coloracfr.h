#ifndef QFFitFunctionsSPIMFCCSFW2CSepDiff2ColorACFR_H
#define QFFitFunctionsSPIMFCCSFW2CSepDiff2ColorACFR_H
#include "qfpluginfitfunction.h"




/*! \brief QFFitFunction class for a SPIM-FCCS fit model with pure diffusion for 2-color crosscorrelation and 1/e^2 lateral width
    \ingroup qf3fitfunp_fitfunctions_spimfcs
*/
class QFFitFunctionsSPIMFCCSFW2CSepDiff2ColorACFR: public QFFitFunction {
    public:
        QFFitFunctionsSPIMFCCSFW2CSepDiff2ColorACFR();
        virtual ~QFFitFunctionsSPIMFCCSFW2CSepDiff2ColorACFR() {}
        /*! \copydoc QFFitFunction::name()   */
        virtual QString name() const { return QString("SPIM-FCCS: 2-comp. normal diffusion, species A+B+AB, c per species, D1/D2 per channel, ACF red (1/e� radii)"); }
        /*! \copydoc QFFitFunction::id()   */
        virtual QString id() const { return QString("fccs_spim_fw_2csepdiff2coloracfr"); }

        /*! \copydoc QFFitFunction::evaluate()   */
        virtual double evaluate(double t, const double* parameters) const;

        /*! \copydoc QFFitFunction::calcParameter()   */
        virtual void calcParameter(double* parameterValues, double* error=NULL) const;

        /*! \copydoc QFFitFunction::isParameterVisible()   */
        virtual bool isParameterVisible(int parameter, const double* parameterValues) const;
        /*! \copydoc QFFitFunction::getAdditionalPlotCount()   */
        virtual unsigned int getAdditionalPlotCount(const double* params);

        /*! \copydoc QFFitFunction::transformParametersForAdditionalPlot()   */
        virtual QString transformParametersForAdditionalPlot(int plot, double* params);

        /*! \copydoc QFFitFunction::get_implementsDerivatives()   */
        virtual bool get_implementsDerivatives() { return false; }
        void sortParameter(double *parameterValues, double *error, bool *fix) const;
};

#endif // QFFitFunctionsSPIMFCCSFW2CSepDiff2ColorACFR_H
