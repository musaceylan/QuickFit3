#include "qffitfunctionmanager.h"
#include <QDir>
#include <QtPlugin>
#include <QPluginLoader>

QFFitFunctionManager::QFFitFunctionManager(QObject* parent):
    QObject(parent)
{
    //ctor
}

QFFitFunctionManager::~QFFitFunctionManager()
{
    //dtor
}

void QFFitFunctionManager::searchPlugins(QString directory) {
    QDir pluginsDir = QDir(directory);
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin) {
            QFPluginFitFunction* iRecord = qobject_cast<QFPluginFitFunction*>(plugin);
            if (iRecord) {
                fitPlugins.append(iRecord);
                filenames.append(fileName);
            }
        }
    }
}


QMap<QString, QFFitFunction*> QFFitFunctionManager::getModels(QString id_start, QObject* parent) {
    QMap<QString, QFFitFunction*> res;

    for (int i=0; i<fitPlugins.size(); i++) {
        QStringList ids=fitPlugins[i]->getIDs();
        for (int j=0; j<ids.size(); j++) {
            if (id_start.isEmpty() || ids[j].startsWith(id_start)) {
                res[ids[j]]=fitPlugins[i]->get(ids[j], parent);
            }
        }
    }

    return res;
}

QString QFFitFunctionManager::getName(int i) const {
    return fitPlugins[i]->getName();
}

QString QFFitFunctionManager::getFilename(int i) const {
    return filenames[i];
}

QString QFFitFunctionManager::getDescription(int i) const {
    return fitPlugins[i]->getDescription();
}

QString QFFitFunctionManager::getAuthor(int i) const {
    return fitPlugins[i]->getAuthor();
}

QString QFFitFunctionManager::getCopyright(int i) const {
    return fitPlugins[i]->getCopyright();
}

QString QFFitFunctionManager::getWeblink(int i) const {
    return fitPlugins[i]->getWeblink();
}

QStringList QFFitFunctionManager::getIDList(int i) const {
    return fitPlugins[i]->getIDs();
}
