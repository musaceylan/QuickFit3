TEMPLATE = lib
TARGET = quickfit3lib
DEPENDPATH += .

include(../quickfit3.pri)

release {
    message("building in RELEASE mode, output is in $$QFOUTPUT")
} else {
    message("building in DEBUG mode, output is in $$QFOUTPUT")
}

INCLUDEPATH += .


QT += gui xml

DESTDIR = $$QFOUTPUT

# Input
HEADERS += dlgnewproperty.h \
           dlgqffitalgorithmprogressdialog.h \
           qenhancedtableview.h \
           programoptions.h \
           qfevaluationeditor.h \
           qfevaluationitem.h \
           qfevaluationitemfactory.h \
           qfevaluationpropertyeditor.h \
           qfpluginevaluation.h \
           qfpluginrawdata.h \
           qfpluginservices.h \
           qfproject.h \
           qfprojectrawdatamodel.h \
           qfprojecttreemodel.h \
           qfproperties.h \
           qfrawdataeditor.h \
           qfrawdatapropertyeditor.h \
           qfrawdatarecord.h \
           qfrawdatarecordfactory.h \
           qfrdrpropertymodel.h \
           qfrdrresultsmodel.h \
           qffitfunction.h \
           qffitalgorithm.h \
           qffitfunctionmanager.h \
           qffitalgorithmmanager.h \
           qfpluginfitfunction.h \
           qfpluginfitalgorithm.h \
           qftools.h \
           qfhtmlhelpwindow.h \
           qffitalgorithmthreaddedfit.h \
           qfextension.h \
           qfextensionmanager.h \
           qfevaluationresultsmodel.h \
           qfplugin.h \
           qflib_version.h \
           csvtools.h

SOURCES += dlgnewproperty.cpp \
           dlgqffitalgorithmprogressdialog.cpp \
           qenhancedtableview.cpp \
           programoptions.cpp \
           qfevaluationeditor.cpp \
           qfevaluationitem.cpp \
           qfevaluationitemfactory.cpp \
           qfevaluationpropertyeditor.cpp \
           qfproject.cpp \
           qfprojectrawdatamodel.cpp \
           qfprojecttreemodel.cpp \
           qfproperties.cpp \
           qfrawdataeditor.cpp \
           qfrawdatapropertyeditor.cpp \
           qfrawdatarecord.cpp \
           qfrawdatarecordfactory.cpp \
           qfrdrpropertymodel.cpp \
           qfrdrresultsmodel.cpp \
           qffitalgorithm.cpp \
           qffitfunctionmanager.cpp \
           qffitalgorithmmanager.cpp \
           qftools.cpp \
           qfhtmlhelpwindow.cpp \
           qffitalgorithmthreaddedfit.cpp \
           qfextensionmanager.cpp \
           qfevaluationresultsmodel.cpp \
           csvtools.cpp

FORMS +=   dlg_newproperty.ui \
           dlg_fitalgorithmprogress.ui

RESOURCES += lib.qrc

TRANSLATIONS= ./translations/de.lib.ts

INCLUDEPATH +=

CONFIG += exceptions rtti stl
