#include "qfrdrimagestack.h"
#include "qfrdrimagestack_data.h"
#include "qfrdrimagestack_dataeditor.h"

QFRDRImageStackPlugin::QFRDRImageStackPlugin(QObject* parent):
    QObject(parent)
{
    //constructor
}

QFRDRImageStackPlugin::~QFRDRImageStackPlugin()
{
    //destructor
}

QFRawDataRecord* QFRDRImageStackPlugin::createRecord(QFProject* parent) {
    // factory method: create a QFRawDataRecord objectof the type of this plugin (QFRDRImageStackData)
    return new QFRDRImageStackData(parent);
}


void QFRDRImageStackPlugin::registerToMenu(QMenu* menuMain) {
    // create menu entries to insert data with this type
    QMenu* menu=menuMain->addMenu(QIcon(getIconFilename()), tr("image &stacks"));
    QAction* action=new QAction(tr("single-file image stack(s)"), parentWidget);
    action->setStatusTip(tr("Insert an image file as image stack"));
    connect(action, SIGNAL(triggered()), this, SLOT(insertSingleFileImageStack()));
    menu->addAction(action);
}


void QFRDRImageStackPlugin::insertSingleFileImageStack() {
    if (project) {
        // file format to import
        QStringList format_names=QFRDRImageStackData::getImageFilterList(services);
        QStringList format_descriptions=QFRDRImageStackData::getImageFormatNameList(services);
        QStringList reader_id=QFRDRImageStackData::getImageReaderIDList(services);
        // look into INI which was the last used format
        QString current_format_name=settings->getQSettings()->value("image_stack/current_format_filter", format_names.value(0,"")).toString();
        // let the user select some files to import
        QStringList files = qfGetOpenFileNames(parentWidget,
                              tr("Select Image File(s) to Import ..."),
                              settings->getCurrentRawDataDir(),
                              format_names.join(";;"), &current_format_name);
        // store the format we just used
        settings->getQSettings()->setValue("image_stack/current_format_filter", current_format_name);
        // here we store some initial parameters
        QMap<QString, QVariant> initParams;
        // set whatever you want (FILETYPE is just an example)!
        QString format_id=reader_id.value(format_names.indexOf(current_format_name), "");
        initParams["FILETYPE"]=format_id;
        initParams["STACKTYPE"]="ONEFILEPERCHANNEL";

        // add all properties in initParams that will be readonly
        QStringList paramsReadonly;
        paramsReadonly<<"FILETYPE"<<"STACKTYPE";

        // now we iterate over all files and use QuickFit's progress bar interface (see plugin services)
        QStringList list = files;
        QStringList::Iterator it = list.begin();
        services->setProgressRange(0, list.size());
        services->setProgress(0);
        int i=0;
        while(it != list.end()) {
            i++;
            QString filename=*it;
            if (QFile::exists(filename)) {
                services->log_text(tr("loading [%2] '%1' ...\n").arg(filename).arg(format_descriptions.value(format_names.indexOf(current_format_name), "")));
                if (!format_id.isEmpty()) {
                    // insert new record:                  type ID, name for record,           list of files,    initial parameters, which parameters are readonly?
                    QFRawDataRecord* e=project->addRawData(getID(), QFileInfo(filename).fileName(), QStringList(filename), initParams, paramsReadonly, QStringList("image"));
                    if (e->error()) { // when an error occured: remove record and output an error message
                        project->deleteRawData(e->getID());
                        QMessageBox::critical(parentWidget, tr("QuickFit 3.0"), tr("Error while importing '%1':\n%2").arg(filename).arg(e->errorDescription()));
                        services->log_error(tr("Error while importing '%1':\n    %2\n").arg(filename).arg(e->errorDescription()));
                    }
                }
                settings->setCurrentRawDataDir(QFileInfo(*it).dir().absolutePath());
                ++it;
            }
            services->setProgress(i);
            QApplication::processEvents();
        }
        services->setProgress(0);
    }
}



Q_EXPORT_PLUGIN2(image_stack, QFRDRImageStackPlugin)