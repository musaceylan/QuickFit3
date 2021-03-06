/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>),
    German Cancer Research Center/University Heidelberg

    

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

#include "qfeplotterexporterlatex.h"
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtGlobal>
#include <QtWidgets>
#else
#include <QtGui>
#endif


#include "qfplugin.h"
#include <iostream>
#include "jkqtplatexengineadapter.h"
#ifdef HAS_EMF_ENGINE
#  include "jkqtpemfengineadapter.h"
#endif
#include "jkqtpbaseplotter.h"

#define LOG_PREFIX QString("qfe_plotterexporterlatex >>> ").toUpper()

QFEPlotterExporterLatex::QFEPlotterExporterLatex(QObject* parent):
    QObject(parent)
{
	logService=NULL;
}

QFEPlotterExporterLatex::~QFEPlotterExporterLatex() {

}


void QFEPlotterExporterLatex::deinit() {
	/* add code for cleanup here */
    for (int i=0; i<adapters.size(); i++) {
        if (adapters[i]) {
            JKQtBasePlotter::deregisterPaintDeviceAdapter(adapters[i]);
            delete adapters[i];
        }
    }
    adapters.clear();
}

void QFEPlotterExporterLatex::projectChanged(QFProject* oldProject, QFProject* project) {
    /* usually cameras do not have to react to a change of the project in QuickFit .. so you don't need to do anything here
       But: possibly you could read config information from the project here
     */

    Q_UNUSED(project);
    Q_UNUSED(oldProject);

}

void QFEPlotterExporterLatex::initExtension() {
    /* do initializations here but do not yet connect to the camera! */
    int i=0;
    adapters<<NULL;
    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPLatexEngineAdapter(true, true, QTeXPaintDevice::Tikz));
    i++;
    adapters<<NULL;
    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPLatexEngineAdapter(true, false, QTeXPaintDevice::Tikz));
    i++;
    adapters<<NULL;
    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPLatexEngineAdapter(false, true, QTeXPaintDevice::Tikz));
    i++;
    adapters<<NULL;
    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPLatexEngineAdapter(false, false, QTeXPaintDevice::Tikz));
    i++;
    adapters<<NULL;
    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPLatexEngineAdapter(false, true, QTeXPaintDevice::Tikz, false));
    i++;
    adapters<<NULL;
    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPLatexEngineAdapter(false, false, QTeXPaintDevice::Tikz, false));
    i++;
    adapters<<NULL;

    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPLatexEngineAdapter(true, true, QTeXPaintDevice::Pgf));
    i++;
    adapters<<NULL;
    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPLatexEngineAdapter(true, false, QTeXPaintDevice::Pgf));
    i++;
    adapters<<NULL;
    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPLatexEngineAdapter(false, true, QTeXPaintDevice::Pgf));
    i++;
    adapters<<NULL;
    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPLatexEngineAdapter(false, false, QTeXPaintDevice::Pgf));
    i++;
    adapters<<NULL;
    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPLatexEngineAdapter(false, true, QTeXPaintDevice::Pgf, false));
    i++;
    adapters<<NULL;
    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPLatexEngineAdapter(false, false, QTeXPaintDevice::Pgf, false));
#ifdef HAS_EMF_ENGINE
    i++;
    adapters<<NULL;
    JKQtBasePlotter::registerPaintDeviceAdapter(adapters[i]=new JKQTPEMFEngineAdapter());
#endif
}


void QFEPlotterExporterLatex::loadSettings(ProgramOptions* settingspo) {
	/* here you could read config information from the quickfit.ini file using settings object */
    if (!settingspo) return;
	if (settingspo->getQSettings()==NULL) return;
    QSettings& settings=*(settingspo->getQSettings()); // the QSettings object for quickfit.ini
	// ALTERNATIVE: read/write Information to/from plugins/extensions/<ID>/<ID>.ini file
	// QSettings settings(services->getConfigFileDirectory()+"/plugins/extensions/"+getID()+"/"+getID()+".ini", QSettings::IniFormat);

}

void QFEPlotterExporterLatex::storeSettings(ProgramOptions* settingspo) {
	/* here you could write config information to the quickfit.ini file using settings object */
    if (!settingspo) return;
	if (settingspo->getQSettings()==NULL) return;
    QSettings& settings=*(settingspo->getQSettings()); // the QSettings object for quickfit.ini

	// ALTERNATIVE: read/write Information to/from plugins/extensions/<ID>/<ID>.ini file
	// QSettings settings(services->getConfigFileDirectory()+"/plugins/extensions/"+getID()+"/"+getID()+".ini", QSettings::IniFormat);

}

void QFEPlotterExporterLatex::log_text(QString message) {
	if (logService) logService->log_text(LOG_PREFIX+message);
	else if (services) services->log_text(LOG_PREFIX+message);
}

void QFEPlotterExporterLatex::log_warning(QString message) {
	if (logService) logService->log_warning(LOG_PREFIX+message);
	else if (services) services->log_warning(LOG_PREFIX+message);
}

void QFEPlotterExporterLatex::log_error(QString message) {
	if (logService) logService->log_error(LOG_PREFIX+message);
	else if (services) services->log_error(LOG_PREFIX+message);
}


Q_EXPORT_PLUGIN2(qfe_plotterexporterlatex, QFEPlotterExporterLatex)
