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

#include "qfimagereaderlibtiff.h"
#include <QObject>
#include "libtiff_tools.h"
#include <QObject>
#include <QtGlobal>
#include <QDebug>
#include "programoptions.h"

FILE* QFImageReaderLIBTIFF::fLibTIFFLog=NULL;
QString QFImageReaderLIBTIFF::fLibTIFFLogFilename=QString("");
QMutex* QFImageReaderLIBTIFF::mutex=NULL;


QFImageReaderLIBTIFF::QFImageReaderLIBTIFF() {
    if (!mutex) mutex=new QMutex(QMutex::Recursive);
    width=0;
    channels=1;
    height=0;
    filename="";
    imageDescription="";
    tif=NULL;
    QString lf=ProgramOptions::getConfigValue("importers_basicimages/libtiff/logfile", ProgramOptions::getInstance()->getConfigFileDirectory()+"/importers_basicimages_libtiff.log").toString();
    if (ProgramOptions::getConfigValue("importers_basicimages/libtiff/log", false).toBool()) {
        QMutexLocker lock(mutex);

        if (fLibTIFFLogFilename!=lf) {
            if (fLibTIFFLog) {
                FILE* f=fLibTIFFLog;
                fLibTIFFLog=NULL;
                fclose(f);
            }
            fLibTIFFLogFilename=lf;
            fLibTIFFLog=fopen(fLibTIFFLogFilename.toLocal8Bit().data(), "a");
        }
        TIFFSetWarningHandler(libTIFFWarningHandler);
        TIFFSetErrorHandler(libTIFFErrorHandler);
    } else {
        QMutexLocker lock(mutex);
        if (fLibTIFFLog) {
            FILE* f=fLibTIFFLog;
            fLibTIFFLog=NULL;
            fclose(f);
        }
        fLibTIFFLogFilename="";
        TIFFSetWarningHandler(0);
        TIFFSetErrorHandler(0);
    }
}


QFImageReaderLIBTIFF::~QFImageReaderLIBTIFF() {
    if (tif!=NULL) TIFFClose(tif);
    tif=NULL;
}

QString QFImageReaderLIBTIFF::filter() const {
    return QObject::tr("TIFF File [libTIFF] (*.tiff *.tif)");
}

QString QFImageReaderLIBTIFF::formatName() const {
    return QObject::tr("TIFF [libTIFF]");
}


bool QFImageReaderLIBTIFF::open(QString filename) {
    close();


    fileinfo.init(filename);
    //TIFFSetWarningHandler(0);
    //qDebug()<<"QFImageReaderLIBTIFF::open("<<filename<<")     tif="<<tif;
    tif = TIFFOpen(filename.toLatin1().data(),"r");
    if (tif) {
        uint32 nx,ny;
        uint16 nc;
        uint16 photo;
        TIFFGetField(tif,TIFFTAG_IMAGEWIDTH,&nx);
        TIFFGetField(tif,TIFFTAG_IMAGELENGTH,&ny);
        TIFFGetField(tif,TIFFTAG_PHOTOMETRIC,&photo);
        bool hasSPP=(bool)TIFFGetField(tif,TIFFTAG_SAMPLESPERPIXEL,&nc);

        char* val=NULL;
        TIFFGetField(tif,TIFFTAG_IMAGEDESCRIPTION,&val);
        if (val) {
            imageDescription=val;
        }
        width=nx;
        height=ny;
        channels = nc;
        if (!hasSPP) {
            if (photo==3) channels=3;
            else channels=1;
        }
        this->filename=filename;
        char* desc=NULL;
        if (!TIFFGetField(tif, TIFFTAG_DOCUMENTNAME, &desc)) {
            TIFFGetField(tif, TIFFTAG_IMAGEDESCRIPTION, &desc);
        }
        if (desc) fileinfo.comment=desc;

        qfimdtGetQuickFitMetaData(fileinfo.properties, filename);

        bool metaOK=false;
        int cnt=0;
        while (!metaOK && cnt<5) {
            char* vall=NULL;
            TIFFGetField(tif,TIFFTAG_IMAGEDESCRIPTION,&vall);
            QByteArray imgD;
            if (vall) {
                imgD=vall;
            }

            if (imgD.size()>0) {
                metaOK=qfimdtGetOMEMetaData(fileinfo.properties, imgD);
                if (!metaOK) {
                    metaOK=qfimdtGetImageJMetaData(fileinfo.properties, imgD);
                }
                if (!metaOK) {
                    metaOK=qfimdtGetTinyTIFFMetaData(fileinfo.properties, imgD);
                }
                imageDescription+=QString("\n\n%1").arg(QString::fromLatin1(imgD.data()));
            }
            metaOK=metaOK||(!nextFrame());
            cnt++;
        }
        reset();


        //TIFFSetWarningHandler(NULL);
        //qDebug()<<"  QFImageReaderLIBTIFF::open("<<filename<<")   tif="<<tif<<"  result=false";
        logTIFFMessage("QFImageReaderLIBTIFF", "opened file %s\n", filename.toLocal8Bit().data());
        return true;
    } else {
        width=0;
        channels=1;
        height=0;
        setLastError(QObject::tr("libtiff: error opening file '%1'").arg(filename));
        this->filename="";
        //qDebug()<<"  QFImageReaderLIBTIFF::open("<<filename<<")   tif="<<tif<<"  result=false";
        return false;
    }
}

void QFImageReaderLIBTIFF::close() {
    if (!tif) return;
    //qDebug()<<"QFImageReaderLIBTIFF::close()     tif="<<tif;
    TIFFClose(tif);
    QString fn=filename;
    filename="";
    imageDescription="";
    tif=NULL;
    logTIFFMessage("QFImageReaderLIBTIFF", "closed file %s\n", fn.toLocal8Bit().data());
    //qDebug()<<"  QFImageReaderLIBTIFF::close()   tif="<<tif;
}

uint32_t QFImageReaderLIBTIFF::countFrames() {
    if (!tif) return 0;

    if (fileinfo.properties.contains("FRAMES")) {
        bool ok=false;
        uint32_t f=fileinfo.properties["FRAMES"].toULongLong(&ok);
        if (ok && f>0) {
            return f;
        }
    }

    uint32_t nb_images = 0;
    //tdir_t dir=TIFFCurrentDirectory(tif);
    TIFFSetDirectory(tif,0);
    //reset();
    do {
        ++nb_images;
    } while (TIFFReadDirectory(tif) && (nb_images<65000)); // LIBTIFF can only read up to 2^{16} frames!!!
    //TIFFSetDirectory(tif,dir);
    TIFFSetDirectory(tif,0);
    //reset();
    return nb_images;
}

void QFImageReaderLIBTIFF::reset() {
    if (!tif) return ;
    TIFFSetDirectory(tif,0);
    //QString fn=filename;
    //close();
    //open(fn);
}

bool QFImageReaderLIBTIFF::nextFrame() {
    if (!tif) return false;
    return TIFFReadDirectory(tif);
}

uint32_t QFImageReaderLIBTIFF::intFrameWidth() {
    return width;
}

uint32_t QFImageReaderLIBTIFF::intFrameHeight() {
    return height;
}

uint32_t QFImageReaderLIBTIFF::intFrameChannels()
{
    return channels;
}









bool QFImageReaderLIBTIFF::intReadFrameUINT16(uint16_t* data, int channel) {
    if (!tif) return false;
    bool ok=true;
    uint16 samplesperpixel, bitspersample;
    uint16 sampleformat = SAMPLEFORMAT_UINT;
    uint32 nx,ny;
    TIFFGetField(tif,TIFFTAG_IMAGEWIDTH,&nx);
    TIFFGetField(tif,TIFFTAG_IMAGELENGTH,&ny);
    bool hasSPP=(bool)TIFFGetField(tif,TIFFTAG_SAMPLESPERPIXEL,&samplesperpixel);
    TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleformat);
    TIFFGetFieldDefaulted(tif,TIFFTAG_BITSPERSAMPLE,&bitspersample);
    //if (samplesperpixel>1 && samplesperpixel<500) {
    //    // if there are more than 500 samples, we assume this is an error in the file and read only one sample!
    //    setLastError(QObject::tr("can not load frames with more than one sample/pixel (RGB, RGBA, ...)   (from file '%1' with %2 samples/pixel)").arg(filename).arg(samplesperpixel));
    //    return false;
    if (samplesperpixel>500) {
        // if there are more than 500 samples, we assume this is an error in the file and read only one sample!
        setLastError(QObject::tr("can not load files with more than 500 samples/pixel   (from file '%1' with %2 samples/pixel)").arg(filename).arg(samplesperpixel));
        return false;
    } else {
        uint16  planarconfig;
        TIFFGetField(tif,TIFFTAG_PLANARCONFIG,&planarconfig);

        // we only read image with one sample per pixel, so we do nothave to care for TIFFTAG_PLANARCONFIG
        if (!TIFFReadRGB(tif, data, nx, ny, channel)) {
            if (TIFFIsTiled(tif)) { // load a tiled frame
                uint32 tw, th;
                TIFFGetField(tif,TIFFTAG_TILEWIDTH,&tw);
                TIFFGetField(tif,TIFFTAG_TILELENGTH,&th);
                switch (bitspersample) {
                    case 8 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadTiled<uint8_t,  uint16_t>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 8-bit tiled image in format different from UINT   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 16 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadTiled<uint16_t,  uint16_t>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 16-bit tiled image in format different from UINT   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    default: {
                            setLastError(QObject::tr("can not load tiled frame sample format in TIFF file   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                }
            } else { // load a non-tiled frame
                switch (bitspersample) {
                    case 8 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadNontiled<uint8_t,  uint16_t>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 8-bit image in format different from UINT   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 16 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadNontiled<uint16_t,  uint16_t>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 16-bit image in format different from UINT   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    default: {
                            setLastError(QObject::tr("can not load frame sample format in TIFF file   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                }
            }
        }
    }
    if (!ok) {
        setLastError(QObject::tr("unknown error when reading frame from TIFF file   (from file '%1')").arg(filename));
    }
    return ok;
}

bool QFImageReaderLIBTIFF::intReadFrameDouble(double *data, int channel) {
    if (!tif) return false;

    bool ok=true;

    uint16 samplesperpixel, bitspersample;
    uint16 sampleformat = SAMPLEFORMAT_UINT;
    uint32 nx,ny;
    TIFFGetField(tif,TIFFTAG_IMAGEWIDTH,&nx);
    TIFFGetField(tif,TIFFTAG_IMAGELENGTH,&ny);
   bool hasSPP=(bool)TIFFGetField(tif,TIFFTAG_SAMPLESPERPIXEL,&samplesperpixel);
    TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleformat);
    TIFFGetFieldDefaulted(tif,TIFFTAG_BITSPERSAMPLE,&bitspersample);
    //if (samplesperpixel>1 && samplesperpixel<500) {
    //    // if there are more than 500 samples, we assume this is an error in the file and read only one sample!
    //    setLastError(QObject::tr("can not load frames with more than one sample/pixel (RGB, RGBA, ...)   (from file '%1' with %2 samples/pixel)").arg(filename).arg(samplesperpixel));
    //    return false;
    if (samplesperpixel>500) {
        // if there are more than 500 samples, we assume this is an error in the file and read only one sample!
        setLastError(QObject::tr("can not load files with more than 500 samples/pixel   (from file '%1' with %2 samples/pixel)").arg(filename).arg(samplesperpixel));
        return false;
    } else {
        uint16  planarconfig;
        TIFFGetField(tif,TIFFTAG_PLANARCONFIG,&planarconfig);
        // we only read image with one sample per pixel, so we do nothave to care for TIFFTAG_PLANARCONFIG
        if (!TIFFReadRGB(tif, data, nx, ny, channel)) {
            if (TIFFIsTiled(tif)) { // load a tiled frame
                uint32 tw, th;
                TIFFGetField(tif,TIFFTAG_TILEWIDTH,&tw);
                TIFFGetField(tif,TIFFTAG_TILELENGTH,&th);
                switch (bitspersample) {
                    case 8 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadTiled<uint8_t,  double>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadTiled<int8_t,  double>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 8-bit tiled image in format different from INT or UINT   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 16 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadTiled<uint16_t,  double>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadTiled<int16_t,  double>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 16-bit tiled image in format different from INT or UINT   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 32 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadTiled<uint32_t,  double>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadTiled<int32_t,  double>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_IEEEFP) ok=TIFFLoadTiled<float,  double>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 32-bit tiled image in format different from INT, UINT, IEEE   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 64 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadTiled<uint64_t,  double>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadTiled<int64_t,  double>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_IEEEFP) ok=TIFFLoadTiled<double,  double>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 64-bit tiled image in format different from INT, UINT, IEEE   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    default: {
                            setLastError(QObject::tr("can not load tiled frame sample format in TIFF file   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                }
            } else { // load a non-tiled frame
                switch (bitspersample) {
                    case 8 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadNontiled<uint8_t,  double>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadNontiled<int8_t,  double>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 8-bit image in format different from INT or UINT   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 16 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadNontiled<uint16_t,  double>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadNontiled<int16_t,  double>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 16-bit image in format different from INT or UINT   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 32 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadNontiled<uint32_t,  double>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadNontiled<int32_t,  double>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_IEEEFP) ok=TIFFLoadNontiled<float,  double>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 32-bit image in format different from INT, UINT, IEEE   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 64 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadNontiled<uint64_t,  double>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadNontiled<int64_t,  double>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_IEEEFP) ok=TIFFLoadNontiled<double,  double>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 64-bit image in format different from INT, UINT, IEEE   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    default: {
                            setLastError(QObject::tr("can not load frame sample format in TIFF file   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                }
            }
        }
    }
    if (!ok) {
        setLastError(QObject::tr("unknown error when reading frame from TIFF file   (from file '%1')").arg(filename));
    }
    return ok;
}

bool QFImageReaderLIBTIFF::intReadFrameFloat(float* data, int channel) {
    if (!tif) return false;

    bool ok=true;

    uint16 samplesperpixel, bitspersample;
    uint16 sampleformat = SAMPLEFORMAT_UINT;
    uint32 nx,ny;
    TIFFGetField(tif,TIFFTAG_IMAGEWIDTH,&nx);
    TIFFGetField(tif,TIFFTAG_IMAGELENGTH,&ny);
    if (nx!=width || ny!=height) {
        setLastError(QObject::tr("ERROR reading TIFF: frame has a size, unequal to the previous frames (file '%1')").arg(filename));
        return false;
    }
    TIFFGetField(tif,TIFFTAG_SAMPLESPERPIXEL,&samplesperpixel);
    TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleformat);
    TIFFGetFieldDefaulted(tif,TIFFTAG_BITSPERSAMPLE,&bitspersample);
    //if (samplesperpixel>1 && samplesperpixel<500) {
    //    // if there are more than 500 samples, we assume this is an error in the file and read only one sample!
    //    setLastError(QObject::tr("can not load frames with more than one sample/pixel (RGB, RGBA, ...)   (from file '%1' with %2 samples/pixel)").arg(filename).arg(samplesperpixel));
    //    return false;
    if (samplesperpixel>500) {
        // if there are more than 500 samples, we assume this is an error in the file and read only one sample!
        setLastError(QObject::tr("can not load files with more than 500 samples/pixel   (from file '%1' with %2 samples/pixel)").arg(filename).arg(samplesperpixel));
        return false;
    } else {
        uint16 planarconfig;
        TIFFGetField(tif,TIFFTAG_PLANARCONFIG,&planarconfig);


        // we only read image with one sample per pixel, so we do nothave to care for TIFFTAG_PLANARCONFIG
        if (!TIFFReadRGB(tif, data, nx, ny, channel)) {
            if (TIFFIsTiled(tif)) { // load a tiled frame
                uint32 tw, th;
                TIFFGetField(tif,TIFFTAG_TILEWIDTH,&tw);
                TIFFGetField(tif,TIFFTAG_TILELENGTH,&th);
                switch (bitspersample) {
                    case 8 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadTiled<uint8_t,  float>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadTiled<int8_t,  float>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 8-bit tiled image in format different from INT or UINT   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 16 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadTiled<uint16_t,  float>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadTiled<int16_t,  float>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 16-bit tiled image in format different from INT or UINT   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 32 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadTiled<uint32_t,  float>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadTiled<int32_t,  float>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_IEEEFP) ok=TIFFLoadTiled<float,  float>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 32-bit tiled image in format different from INT, UINT, IEEE   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 64 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadTiled<uint64_t,  float>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadTiled<int64_t,  float>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_IEEEFP) ok=TIFFLoadTiled<double,  float>(data,  tif, nx, ny, tw, th, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 64-bit tiled image in format different from INT, UINT, IEEE   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    default: {
                            setLastError(QObject::tr("can not load tiled frame sample format in TIFF file   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                }
            } else { // load a non-tiled frame
                switch (bitspersample) {
                    case 8 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadNontiled<uint8_t,  float>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadNontiled<int8_t,  float>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 8-bit image in format different from INT or UINT   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 16 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadNontiled<uint16_t,  float>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadNontiled<int16_t,  float>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 16-bit image in format different from INT or UINT   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 32 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadNontiled<uint32_t,  float>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadNontiled<int32_t,  float>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_IEEEFP) ok=TIFFLoadNontiled<float,  float>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 32-bit image in format different from INT, UINT, IEEE   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    case 64 :
                        if (sampleformat==SAMPLEFORMAT_UINT) ok=TIFFLoadNontiled<uint64_t,  float>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_INT) ok=TIFFLoadNontiled<int64_t,  float>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else if (sampleformat==SAMPLEFORMAT_IEEEFP) ok=TIFFLoadNontiled<double,  float>(data,  tif, nx, ny, channel, samplesperpixel, planarconfig);
                        else {
                            setLastError(QObject::tr("can not load 64-bit image in format different from INT, UINT, IEEE   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                    default: {
                            setLastError(QObject::tr("can not load frame sample format in TIFF file   (from file '%1')").arg(filename));
                            return false;
                        }
                        break;
                }
            }
        }
    }
    if (!ok) {
        setLastError(QObject::tr("unknown error when reading frame from TIFF file   (from file '%1')").arg(filename));
    }
    return ok;
}
