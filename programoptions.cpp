#include "programoptions.h"
#include <iostream>

ProgramOptions::ProgramOptions( QString ini, QObject * parent, QApplication* app  ):
    QObject(parent)
{
    this->app=app;
    iniFilename=ini;
    QFileInfo fi(QCoreApplication::applicationFilePath());
    if (iniFilename.isEmpty()) {
        iniFilename= fi.absolutePath()+"/"+fi.completeBaseName()+".ini";
    }
    currentRawDataDir=fi.absolutePath();
    //std::cout<<"config file is: "<<iniFilename.toStdString()<<std::endl;
    settings=NULL;

    // default values
    style= QApplication::style()->metaObject()->className();
    stylesheet="default";
    languageID="en";
    maxThreads=1;

    readSettings();
}

ProgramOptions::~ProgramOptions()
{
    writeSettings();
}

void ProgramOptions::openSettingsDialog() {
    OptionsDialog* settingsDlg=new OptionsDialog(NULL);
    settingsDlg->spnMaxThreads->setValue(maxThreads);

    // find all available translations ... program is already in english, so add "en" by default (there is not en.qm file!)
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cd("translations");
    QStringList filters;
    filters << "*.qm";
    settingsDlg->cmbLanguage->clear();
    settingsDlg->cmbLanguage->addItem("en");
    QStringList sl=dir.entryList(filters, QDir::Files);
    for (int i=0; i<sl.size(); i++) {
        settingsDlg->cmbLanguage->addItem(sl[i].remove(".qm"), Qt::CaseInsensitive);
    }
    settingsDlg->cmbLanguage->setCurrentIndex( settingsDlg->cmbLanguage->findText(languageID));
    settingsDlg->cmbStyle->addItems(QStyleFactory::keys());
    settingsDlg->cmbStyle->setCurrentIndex(settingsDlg->cmbStyle->findText(style, Qt::MatchContains));


    // find all available stylesheets
    dir.cd("../stylesheets");
    filters.clear();
    filters << "*.qss";
    settingsDlg->cmbStylesheet->clear();
    sl=dir.entryList(filters, QDir::Files);
    for (int i=0; i<sl.size(); i++) {
        settingsDlg->cmbStylesheet->addItem(sl[i].remove(".qss"), Qt::CaseInsensitive);
    }
    settingsDlg->cmbStylesheet->setCurrentIndex( settingsDlg->cmbStylesheet->findText(stylesheet));

    if (settingsDlg->exec() == QDialog::Accepted ){
        maxThreads=settingsDlg->spnMaxThreads->value();
        languageID=settingsDlg->cmbLanguage->currentText();
        stylesheet=settingsDlg->cmbStylesheet->currentText();
        style=settingsDlg->cmbStyle->currentText();
        writeSettings();
        readSettings();
    }

}

void ProgramOptions::writeSettings() {
    settings->setValue("quickfit/language", languageID);
    settings->setValue("quickfit/stylesheet", stylesheet);
    settings->setValue("quickfit/style", style);
    settings->setValue("quickfit/max_threads", maxThreads);
    settings->setValue("quickfit/current_raw_data_dir", currentRawDataDir);
}


void ProgramOptions::readSettings() {
    if (!settings) {
        if (iniFilename=="native") { // on windows: registry, on Linux/MacOS: default
            settings= new QSettings(this);
        } else if (iniFilename=="native_inifile") { // ensures a INI file at the default location, even on windows
            settings= new QSettings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName(), this);
        } else {
            settings= new QSettings(iniFilename, QSettings::IniFormat, this);
        }
    }
    maxThreads=settings->value("quickfit/max_threads", maxThreads).toInt();
    currentRawDataDir=settings->value("quickfit/current_raw_data_dir", currentRawDataDir).toString();

    languageID=settings->value("quickfit/language", languageID).toString();
    if (languageID != "en") { // english is default
        QString fn=QString(QCoreApplication::applicationDirPath()+"/translations/%1.qm").arg(languageID);
        //std::cout<<"loading translation '"<<fn.toStdString()<<"' ... ";
        QTranslator* translator=new QTranslator(this);
        if (app && translator->load(fn)) {
            app->installTranslator(translator);
            //std::cout<<"OK\n";
            emit languageChanged(languageID);
        } else {
            //std::cout<<"ERROR\n";
        }
    }
    style=settings->value("quickfit/style", style).toString();
    if (app) {
        app->setStyle(style);
    }
    emit styleChanged(style);
    stylesheet=settings->value("quickfit/stylesheet", stylesheet).toString();
    if (app) {
        QString fn=QString(QCoreApplication::applicationDirPath()+"/stylesheets/%1.qss").arg(stylesheet);
        //std::cout<<"loading stylesheet '"<<fn.toStdString()<<"' ... ";
        QFile f(fn);
        f.open(QFile::ReadOnly);
        QTextStream s(&f);
        QString qss=s.readAll();
        //std::cout<<qss.toStdString()<<std::endl;
        app->setStyleSheet(qss);
        //std::cout<<"OK\n";
    }
    emit stylesheetChanged(stylesheet);

}



