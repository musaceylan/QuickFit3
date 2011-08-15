#include "qfespimb040cameraconfig.h"
#include "qfespimb040mainwindow.h"
#include <QtGui>
#include <iostream>

QFESPIMB040CameraConfig::viewDataStruct::viewDataStruct() {
    reset();
}

void QFESPIMB040CameraConfig::viewDataStruct::reset() {
    rawImage.assign(100, 100, 0);
    acqFrames=acqFramesQR=0;
    acqStartedQR=acqStarted=lastAcquisitionTime=QTime::currentTime();
    continuous_is_first=false;
    abort_continuous_acquisition=false;
    extension=NULL;
    camera=NULL;
    usedCamera=0;
    timestamp=0;
    exposureTime=0;
}


QFESPIMB040CameraConfig::QFESPIMB040CameraConfig(QFESPIMB040MainWindow* parent, int camViewID, QFExtensionServices* pluginServices):
    QGroupBox(parent)
{
    m_camViewID=camViewID;
    m_parent=parent;
    m_pluginServices=pluginServices;
    m_extManager=pluginServices->getExtensionManager();
    camView=NULL;
    locked=false;


    // initialize raw image memory ...
    viewData.reset();

    // create widgets and actions
    setTitle(tr(" Camera %1: ").arg(camViewID+1));
    createWidgets(m_pluginServices->getExtensionManager());
    createActions();
    displayStates(QFESPIMB040CameraConfig::Disconnected);
    if (cmbAcquisitionDevice->count()<=0) displayStates(QFESPIMB040CameraConfig::Inactive);

}

QFESPIMB040CameraConfig::~QFESPIMB040CameraConfig()
{
    if (camView) {
        camView->close();
        delete camView;
        camView=NULL;
    }
}

bool QFESPIMB040CameraConfig::lockCamera(QFExtension** extension, QFExtensionCamera** ecamera, int* camera, QString* acquisitionSettingsFilename, QString* previewSettingsFilename) {
    if (locked || (!actDisConnect->isChecked()) || (!viewData.camera) || (!viewData.extension)) {
        *camera=-1;
        *extension=NULL;
        *ecamera=NULL;
        return false;
    }
    actStartStopPreview->setChecked(false);
    displayStates(QFESPIMB040CameraConfig::Locked);

    QString filename="";
    *previewSettingsFilename="";
    *acquisitionSettingsFilename="";
    *acquisitionSettingsFilename=cmbAcquisitionConfiguration->currentConfigFilename();
    //if (cmbPreviewConfiguration->currentIndex()>=0) *previewSettingsFilename=cmbPreviewConfiguration->itemData(cmbPreviewConfiguration->currentIndex()).toString();
    *previewSettingsFilename=cmbPreviewConfiguration->currentConfigFilename();
    *extension=viewData.extension;
    *ecamera=viewData.camera;
    *camera=viewData.usedCamera;
    locked=true;
    return true;
}

void QFESPIMB040CameraConfig::releaseCamera() {
    if (locked) {
        displayStates(QFESPIMB040CameraConfig::Connected);
        locked=false;
    }
}


void QFESPIMB040CameraConfig::loadSettings(ProgramOptions* settings, QString prefix) {
    if (camView) camView->loadSettings(settings, prefix+"cam_view/");

    cmbAcquisitionDevice->setCurrentIndex(settings->getQSettings()->value(prefix+"last_device", 0).toInt());
    spinAcquisitionDelay->setValue(settings->getQSettings()->value(prefix+"acquisition_delay", 0).toDouble());
    cmbPreviewConfiguration->setCurrentConfig(settings->getQSettings()->value(prefix+"preview_config", "default").toString());
    cmbAcquisitionConfiguration->setCurrentConfig(settings->getQSettings()->value(prefix+"acquisition_config", "default").toString());
}

void QFESPIMB040CameraConfig::storeSettings(ProgramOptions* settings, QString prefix) {
    if (camView) camView->storeSettings(settings, prefix+"cam_view/");

    settings->getQSettings()->setValue(prefix+"last_device", cmbAcquisitionDevice->currentIndex());
    settings->getQSettings()->setValue(prefix+"acquisition_delay", spinAcquisitionDelay->value());
    settings->getQSettings()->setValue(prefix+"acquisition_config", cmbAcquisitionConfiguration->currentConfigName());//currentConfigFilename());
    settings->getQSettings()->setValue(prefix+"preview_config", cmbPreviewConfiguration->currentConfigName());//currentConfigFilename());
}

void QFESPIMB040CameraConfig::closeEvent ( QCloseEvent * event ) {
    // disconnect devices and close camera view:
    if (actDisConnect->isChecked()) {
        disconnectDevice();
        actDisConnect->setChecked(false);
    }
    if (camView) {
        camView->close();
    }
}


void QFESPIMB040CameraConfig::createWidgets(QFExtensionManager* extManager) {
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // create main layout
    ////////////////////////////////////////////////////////////////////////////////////////////////
    QFormLayout* camlayout=new QFormLayout(this);
    setLayout(camlayout);


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // create camera view widgets ... and hide them
    ////////////////////////////////////////////////////////////////////////////////////////////////
    camView=new QFESPIMB040CameraView(m_camViewID, QString(m_pluginServices->getConfigFileDirectory()+"/spimb040_cam1.log"), m_pluginServices, NULL);
    camView->close();



    ////////////////////////////////////////////////////////////////////////////////////////////////
    // create input widgets for camera devices
    ////////////////////////////////////////////////////////////////////////////////////////////////
    cmbAcquisitionDevice=new QFCameraComboBox(extManager, this);
    QHBoxLayout* hbl=new QHBoxLayout(this);
    hbl->setContentsMargins(0,0,0,0);
    hbl->addWidget(cmbAcquisitionDevice);
    btnConnect=new QToolButton(this);
    hbl->addWidget(btnConnect);
    camlayout->addRow(tr("<b>Device:</b>"), hbl);
    cmbAcquisitionDevice->setEnabled(false);



    cmbPreviewConfiguration=new QFCameraConfigEditorWidget(m_pluginServices->getConfigFileDirectory(), this);
    cmbPreviewConfiguration->connectTo(cmbAcquisitionDevice);
    camlayout->addRow(tr("<b>Preview:</b>"), cmbPreviewConfiguration);

    hbl=new QHBoxLayout(this);
    hbl->setContentsMargins(0,0,0,0);
    spinAcquisitionDelay=new QDoubleSpinBox(this);
    spinAcquisitionDelay->setMinimum(0);
    spinAcquisitionDelay->setSuffix(tr("ms"));
    spinAcquisitionDelay->setMaximum(10000);
    spinAcquisitionDelay->setSingleStep(1);
    spinAcquisitionDelay->setDecimals(0);
    spinAcquisitionDelay->setEnabled(false);
    btnPreviewSingle=new QToolButton(this);
    btnPreviewSingle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    QFrame* line=new QFrame(this);
    line->setFrameStyle(QFrame::VLine | QFrame::Raised);
    line->setLineWidth(0);
    line->setMidLineWidth(1);
    line->setMinimumWidth(8);
    line->setMaximumWidth(8);
    hbl->addWidget(btnPreviewSingle);
    hbl->addWidget(line);
    hbl->addWidget(new QLabel(tr("delay: "), this));
    hbl->addWidget(spinAcquisitionDelay);
    btnPreviewContinuous=new QToolButton(this);
    btnPreviewContinuous->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    hbl->addWidget(btnPreviewContinuous);
    hbl->addStretch();
    camlayout->addRow("", hbl);




    cmbAcquisitionConfiguration=new QFCameraConfigEditorWidget(m_pluginServices->getConfigFileDirectory(), this);
    cmbAcquisitionConfiguration->connectTo(cmbAcquisitionDevice);
    camlayout->addRow(tr("<b>Acquisition:</b>"), cmbAcquisitionConfiguration);
}





void QFESPIMB040CameraConfig::createActions() {
    actDisConnect = new QAction(QIcon(":/spimb040/acquisitionconnect.png"), tr("&Connect"), this);
    actDisConnect->setCheckable(true);
    connect(actDisConnect, SIGNAL(triggered()), this, SLOT(disConnectAcquisitionDevice()));

    actStartStopPreview = new QAction(QIcon(":/spimb040/acquisitionstart.png"), tr("&Start acquisition"), this);
    actStartStopPreview->setCheckable(true);
    connect(actStartStopPreview, SIGNAL(triggered()), this, SLOT(startStopPreview()));

    actPreviewSingle = new QAction(QIcon(":/spimb040/acquisitionsingle.png"), tr("&Acquire single image"), this);
    connect(actPreviewSingle, SIGNAL(triggered()), this, SLOT(previewSingle()));


    camView->addUserAction(cmbPreviewConfiguration->configAction());
    camView->addUserAction(actStartStopPreview);
    camView->addUserAction(actPreviewSingle);
    btnConnect->setDefaultAction(actDisConnect);
    btnPreviewSingle->setDefaultAction(actPreviewSingle);
    btnPreviewContinuous->setDefaultAction(actStartStopPreview);



}

void QFESPIMB040CameraConfig::displayStates(QFESPIMB040CameraConfig::States state) {
    if (state==QFESPIMB040CameraConfig::Connected) {
        setEnabled(true);
        actDisConnect->setIcon(QIcon(":/spimb040/acquisitiondisconnect.png"));
        actDisConnect->setText(tr("&Disconnect"));
        actDisConnect->setEnabled(true);
        actDisConnect->setChecked(true);
        actStartStopPreview->setEnabled(true);
        actStartStopPreview->setChecked(false);
        actStartStopPreview->setIcon(QIcon(":/spimb040/acquisitionstart.png"));
        actStartStopPreview->setText(tr("&Start Preview"));
        actPreviewSingle->setEnabled(true);

        cmbAcquisitionDevice->setEnabled(false);
        cmbPreviewConfiguration->setEnabled(true);
        cmbAcquisitionConfiguration->setEnabled(true);
        spinAcquisitionDelay->setEnabled(true);


    } else if (state==QFESPIMB040CameraConfig::Previewing) {
        setEnabled(true);
        actDisConnect->setIcon(QIcon(":/spimb040/acquisitiondisconnect.png"));
        actDisConnect->setText(tr("&Disconnect"));
        actDisConnect->setEnabled(true);
        actDisConnect->setChecked(true);
        actStartStopPreview->setEnabled(true);
        actStartStopPreview->setChecked(true);
        actStartStopPreview->setIcon(QIcon(":/spimb040/acquisitionstop.png"));
        actStartStopPreview->setText(tr("&Stop Preview"));
        actPreviewSingle->setEnabled(false);


        cmbAcquisitionDevice->setEnabled(false);
        cmbPreviewConfiguration->setEnabled(false);
        cmbAcquisitionConfiguration->setEnabled(true);
        spinAcquisitionDelay->setEnabled(true);


    } else if (state==QFESPIMB040CameraConfig::Inactive) {
        setEnabled(false);
        actDisConnect->setIcon(QIcon(":/spimb040/acquisitionconnect.png"));
        actDisConnect->setText(tr("&Connect"));
        actDisConnect->setEnabled(false);
        actDisConnect->setChecked(false);
        actStartStopPreview->setEnabled(false);
        actStartStopPreview->setChecked(false);
        actStartStopPreview->setIcon(QIcon(":/spimb040/acquisitionstart.png"));
        actStartStopPreview->setText(tr("&Start Preview"));
        actPreviewSingle->setEnabled(false);


        cmbAcquisitionDevice->setEnabled(false);
        cmbPreviewConfiguration->setEnabled(false);
        cmbAcquisitionConfiguration->setEnabled(false);
        spinAcquisitionDelay->setEnabled(false);


    } else if (state==QFESPIMB040CameraConfig::Locked) {
        setEnabled(true);
        actDisConnect->setIcon(QIcon(":/spimb040/acquisitiondisconnect.png"));
        actDisConnect->setText(tr("&Disconnect"));
        actDisConnect->setEnabled(false);
        actDisConnect->setChecked(true);
        actStartStopPreview->setEnabled(false);
        actStartStopPreview->setChecked(false);
        actStartStopPreview->setIcon(QIcon(":/spimb040/acquisitionstart.png"));
        actStartStopPreview->setText(tr("&Start Preview"));
        actPreviewSingle->setEnabled(false);


        cmbAcquisitionDevice->setEnabled(false);
        cmbPreviewConfiguration->setEnabled(false);
        cmbAcquisitionConfiguration->setEnabled(false);
        spinAcquisitionDelay->setEnabled(false);
    } else { // Disconnected
        setEnabled(true);
        actDisConnect->setIcon(QIcon(":/spimb040/acquisitionconnect.png"));
        actDisConnect->setText(tr("&Connect"));
        actDisConnect->setEnabled(true);
        actDisConnect->setChecked(false);
        actStartStopPreview->setEnabled(false);
        actStartStopPreview->setChecked(false);
        actStartStopPreview->setIcon(QIcon(":/spimb040/acquisitionstart.png"));
        actStartStopPreview->setText(tr("&Start Preview"));
        actPreviewSingle->setEnabled(false);


        cmbAcquisitionDevice->setEnabled(true);
        cmbPreviewConfiguration->setEnabled(false);
        cmbAcquisitionConfiguration->setEnabled(false);
        spinAcquisitionDelay->setEnabled(false);
    }
}


bool QFESPIMB040CameraConfig::connectDevice(QFExtension* extension, QFExtensionCamera* cam, int camera) {

    viewData.reset();

    locked=false;


    if (cam && extension) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        viewData.extension=extension;
        viewData.camera=cam;
        viewData.usedCamera=camera;
        cam->setLogging(m_parent);
        bool s=cam->connectDevice(camera);
        if (!s) {
            //cam->setLogging(NULL);
            viewData.reset();
            m_parent->log_error(tr("error connecting to device %1, camera %2 ...\n").arg(extension->getName()).arg(camera));
        }
        QApplication::restoreOverrideCursor();
        return s;
    } else {
        return false;
    }
}


void QFESPIMB040CameraConfig::disconnectDevice() {
    locked=false;
    if (viewData.camera) {
        if (viewData.camera->isConnected(viewData.usedCamera)) {
            viewData.camera->disconnectDevice(viewData.usedCamera);
        }
    }
}


bool QFESPIMB040CameraConfig::acquireSingle() {
    if (viewData.camera) {
        int usedcam=viewData.usedCamera;
        if (viewData.camera->isConnected(usedcam)) {
            //viewData.camera->acquire(usedcam);
            int dw=viewData.camera->getImageWidth(usedcam);
            int dh=viewData.camera->getImageHeight(usedcam);
            uint64_t timestamp=0;
            viewData.rawImage.resize(dw, dh);
            bool ok=viewData.camera->acquire(usedcam, viewData.rawImage.data(), &timestamp);
            viewData.lastAcquisitionTime=QTime::currentTime();
            double exposure=viewData.camera->getExposureTime(usedcam);
            viewData.timestamp=timestamp;
            viewData.exposureTime=exposure;
            viewData.acqFrames++;
            viewData.acqFramesQR++;
            return ok;
        }
    }
    return false;
}





void QFESPIMB040CameraConfig::disConnectAcquisitionDevice() {
    if (cmbAcquisitionDevice->currentIndex()<0) {
        // if there are no devices registered: deactivate everything!
        displayStates(QFESPIMB040CameraConfig::Disconnected);
        return;
    }

    QFExtension* extension=cmbAcquisitionDevice->currentExtension();
    QFExtensionCamera* cam=cmbAcquisitionDevice->currentExtensionCamera();
    int camIdx=cmbAcquisitionDevice->currentCameraID();

    //std::cout<<"disConnectAcquisitionDevice()  dev="<<p.x()<<" cam="<<p.y()<<"  cam*="<<cam<<" extension*="<<extension<<std::endl;

    if (cam && extension) {
        if (actDisConnect->isChecked()) {
            // connect to a device

            connectDevice(cmbAcquisitionDevice->currentExtension(), cmbAcquisitionDevice->currentExtensionCamera(), cmbAcquisitionDevice->currentCameraID());
            if (!cam->isConnected(camIdx)) {
                displayStates(QFESPIMB040CameraConfig::Disconnected);
                m_parent->log_error(tr("error connecting to device %1, camera %2!\n").arg(extension->getName()).arg(camIdx));
            } else {
                displayStates(QFESPIMB040CameraConfig::Connected);
                m_parent->log_text(tr("connected to device %1, camera %2!\n").arg(extension->getName()).arg(camIdx));
                //camView->show();
                //previewSingle();
            }
        } else {
            //disconnect from the current device and delete the settings widget
            displayStates(QFESPIMB040CameraConfig::Disconnected);
            if (cam->isConnected(camIdx))  {
                cam->disconnectDevice(camIdx);
                //cam->setLogging(NULL);
            }
            m_parent->log_text(tr("disconnected from device %1, camera %2!\n").arg(extension->getName()).arg(camIdx));
        }
    }
}




void QFESPIMB040CameraConfig::previewSingle() {
    if (viewData.camera) {
        int camIdx=viewData.usedCamera;
        //QFExtension* extension=viewData.extension;
        //QFExtensionCamera* cam=viewData.camera;
        if (viewData.camera->isConnected(camIdx)) {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            QString filename="";
            //if (cmbPreviewConfiguration->currentIndex()>=0) filename=cmbPreviewConfiguration->itemData(cmbPreviewConfiguration->currentIndex()).toString();
            filename=cmbPreviewConfiguration->currentConfigFilename();
            QSettings* settings=new QSettings(filename, QSettings::IniFormat);
            viewData.camera->useCameraSettings(camIdx, *settings);
            if (acquireSingle()) {
                camView->show();
                camView->displayImage(viewData.rawImage, viewData.timestamp, viewData.exposureTime);
            } else {
                m_parent->log_error(tr("could not acquire frame, as device is not connected ...!\n"));
            }
            delete settings;
            QApplication::restoreOverrideCursor();
        }
    }
}



void QFESPIMB040CameraConfig::startStopPreview() {
    if (viewData.camera) {
        int camIdx=viewData.usedCamera;
        //QFExtension* extension=viewData.extension;
        //QFExtensionCamera* cam=viewData.camera;
        if (viewData.camera->isConnected(camIdx)) {
            if (actStartStopPreview->isChecked()) {
                displayStates(QFESPIMB040CameraConfig::Previewing);
                viewData.acqFrames=0;
                viewData.acqFramesQR=0;
                viewData.acqStarted.start();
                viewData.acqStartedQR.start();
                viewData.abort_continuous_acquisition=false;
                viewData.continuous_is_first=true;
                QString filename="";
                //if (cmbPreviewConfiguration->currentIndex()>=0) filename=cmbPreviewConfiguration->itemData(cmbPreviewConfiguration->currentIndex()).toString();
                filename=cmbPreviewConfiguration->currentConfigFilename();
                QSettings* settings=new QSettings(filename, QSettings::IniFormat);
                viewData.camera->useCameraSettings(camIdx, *settings);
                camView->show();
                previewContinuous();
                delete settings;
            } else {
                displayStates(QFESPIMB040CameraConfig::Connected);
            }
        }
    }
}



void QFESPIMB040CameraConfig::previewContinuous() {
    if (viewData.camera) {
        int camIdx=viewData.usedCamera;
        QFExtension* extension=viewData.extension;
        QFExtensionCamera* cam=viewData.camera;
        if (viewData.abort_continuous_acquisition) {
            displayStates(QFESPIMB040CameraConfig::Connected);
            return;
        }

        if (cam->isConnected(camIdx)) {
            if (viewData.continuous_is_first) {
                viewData.continuous_is_first=false;
            }
            if (acquireSingle()) {
                camView->displayImage(viewData.rawImage, viewData.timestamp, viewData.exposureTime);
            }
        } else {
            m_parent->log_error(tr("could not acquire frame, as device is not connected ...!\n"));
        }


        if (viewData.acqFramesQR%5==0) {
            double framerate=(double)viewData.acqFramesQR/(double)(viewData.acqStartedQR.elapsed()/1000.0);
            //labVideoSettings->setText(tr("%1 x %2 : %3 fps     ").arg(image.width()).arg(image.height()).arg(framerate));
            camView->displayCameraConfig(extension->getName()+" #"+QString::number(camIdx), framerate);
            if (viewData.acqStartedQR.elapsed()>10000) {
                viewData.acqStartedQR.start();
                viewData.acqFramesQR=0;
            }
        }
        viewData.abort_continuous_acquisition=false;


        // start timer till next acquisition
        if (actStartStopPreview->isChecked()) {
            QTimer::singleShot(spinAcquisitionDelay->value(), this, SLOT(previewContinuous()));
        }
        /*} else {
            actStartStopAcquisition->setChecked(false);
            actStartStopAcquisition->setIcon(QIcon(":/spimb040/acquisitionstart.png"));
            actStartStopAcquisition->setText(tr("&Start Acquisition"));
            actStartStopAcquisition->setEnabled(false);
            actAcquireSingle->setEnabled(false);
            actCameraConfig->setEnabled(false);
        }*/
    }
}



