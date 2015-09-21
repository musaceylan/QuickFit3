#
#    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg
#	
#	 
#
#    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).
#
#    This software is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

TEMPLATE = lib
CONFIG += plugin

TARGET = qfe_plotterexportercairo
DEFINES += TARGETNAME=$$TARGET
DEPENDPATH += ./


include(../../qf3plugins.pri)
include(../../libquickfitwidgets.pri)
include(../../extlibsb040/cairoqpaintdevice.pri)

DESTDIR = $${QFOUTPUT}/plugins/

# Input
HEADERS += qfe_plotterexportercairo.h \
    jkqtcairoengineadapter.h
SOURCES += qfe_plotterexportercairo.cpp \
    jkqtcairoengineadapter.cpp



FORMS =

RESOURCES += qfe_plotterexportercairo.qrc

TRANSLATIONS= ./translations/de.qfe_plotterexportercairo.ts

INCLUDEPATH += ../../lib/ \
               ../../libqf3widgets/

include(../../quickfit3_configqtwidgets.pri)



#SRC_DISTRIBUTED.files = $$HEADERS \
#                        $$SOURCES \
#                        $$FORMS \
#                        $$RESOURCES \
#                        $$TRANSLATIONS \
#                        $$GPL3LICENSE
