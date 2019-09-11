/*
 * Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef FILESTEST
#define FILESTEST

#include "testutil.h"

#include <QDir>

#include <kaboutdata.h>
#include <klocalizedstring.h>
#include <kis_debug.h>

#include <KisImportExportManager.h>

#include <KisDocument.h>
#include <KisPart.h>
#include <kis_image.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <QTemporaryFile>
#include <QFileInfo>
#include <QApplication>
#include <QFile>
#include <QFileDevice>
#include <QIODevice>

namespace TestUtil
{

void testFiles(const QString& _dirname, const QStringList& exclusions, const QString &resultSuffix = QString(), int fuzzy = 0, int maxNumFailingPixels = 0, bool showDebug = true)
{
    QDir dirSources(_dirname);
    QStringList failuresFileInfo;
    QStringList failuresDocImage;
    QStringList failuresCompare;

    Q_FOREACH (QFileInfo sourceFileInfo, dirSources.entryInfoList()) {
        qDebug() << sourceFileInfo.fileName();
        if (exclusions.indexOf(sourceFileInfo.fileName()) > -1) {
            continue;
        }
        if (!sourceFileInfo.isHidden() && !sourceFileInfo.isDir()) {
            QFileInfo resultFileInfo(QString(FILES_DATA_DIR) + "/results/" + sourceFileInfo.fileName() + resultSuffix + ".png");

            if (!resultFileInfo.exists()) {
                failuresFileInfo << resultFileInfo.fileName();
                continue;
            }

            KisDocument *doc = qobject_cast<KisDocument*>(KisPart::instance()->createDocument());

            KisImportExportManager manager(doc);
            doc->setFileBatchMode(true);

            KisImportExportErrorCode status = manager.importDocument(sourceFileInfo.absoluteFilePath(), QString());
            Q_UNUSED(status);

            if (!doc->image()) {
                failuresDocImage << sourceFileInfo.fileName();
                continue;
            }

            QString id = doc->image()->colorSpace()->id();
            if (id != "GRAYA" && id != "GRAYAU16" && id != "RGBA" && id != "RGBA16") {
                dbgKrita << "Images need conversion";
                doc->image()->convertImageColorSpace(KoColorSpaceRegistry::instance()->rgb8(),
                                                    KoColorConversionTransformation::IntentAbsoluteColorimetric,
                                                    KoColorConversionTransformation::NoOptimization);
                doc->image()->waitForDone();
            }

            qApp->processEvents();
            doc->image()->waitForDone();
            QImage sourceImage = doc->image()->projection()->convertToQImage(0, doc->image()->bounds());



            QImage resultImage(resultFileInfo.absoluteFilePath());
            resultImage = resultImage.convertToFormat(QImage::Format_ARGB32);
            sourceImage = sourceImage.convertToFormat(QImage::Format_ARGB32);

            QPoint pt;

            if (!TestUtil::compareQImages(pt, resultImage, sourceImage, fuzzy, fuzzy, maxNumFailingPixels, showDebug)) {
                failuresCompare << sourceFileInfo.fileName() + ": " + QString("Pixel (%1,%2) has different values").arg(pt.x()).arg(pt.y()).toLatin1();
                sourceImage.save(sourceFileInfo.fileName() + ".png");
                resultImage.save(resultFileInfo.fileName() + ".expected.png");
                continue;
            }

            delete doc;
        }
    }
    if (failuresCompare.isEmpty() && failuresDocImage.isEmpty() && failuresFileInfo.isEmpty()) {
        return;
    }
    qWarning() << "Comparison failures: " << failuresCompare;
    qWarning() << "No image failures: " << failuresDocImage;
    qWarning() << "No comparison image: " <<  failuresFileInfo;

    QFAIL("Failed testing files");
}


void prepareFile(QFileInfo sourceFileInfo, bool removePermissionToWrite, bool removePermissionToRead)
{

    QFileDevice::Permissions permissionsBefore;
    if (sourceFileInfo.exists()) {
        permissionsBefore = QFile::permissions(sourceFileInfo.absoluteFilePath());
        ENTER_FUNCTION() << permissionsBefore;
    } else {
        QFile file(sourceFileInfo.absoluteFilePath());
        bool opened = file.open(QIODevice::ReadWrite);
        if (!opened) {
            qDebug() << "The file cannot be opened/created: " << file.error() << file.errorString();
        }
        permissionsBefore = file.permissions();
        file.close();
    }
    QFileDevice::Permissions permissionsNow = permissionsBefore;
    if (removePermissionToRead) {
        permissionsNow = permissionsBefore &
                (~QFileDevice::ReadUser & ~QFileDevice::ReadOwner
                 & ~QFileDevice::ReadGroup & ~QFileDevice::ReadOther);
    }
    if (removePermissionToWrite) {
        permissionsNow = permissionsBefore &
                (~QFileDevice::WriteUser & ~QFileDevice::WriteOwner
                 & ~QFileDevice::WriteGroup & ~QFileDevice::WriteOther);
    }

    QFile::setPermissions(sourceFileInfo.absoluteFilePath(), permissionsNow);

}

void restorePermissionsToReadAndWrite(QFileInfo sourceFileInfo)
{
    QFileDevice::Permissions permissionsNow = sourceFileInfo.permissions();
    QFileDevice::Permissions permissionsAfter = permissionsNow
            | (QFileDevice::ReadUser | QFileDevice::ReadOwner
            | QFileDevice::ReadGroup | QFileDevice::ReadOther)
            | (QFileDevice::WriteUser | QFileDevice::WriteOwner
            | QFileDevice::WriteGroup | QFileDevice::WriteOther);
    QFile::setPermissions(sourceFileInfo.absoluteFilePath(), permissionsAfter);
}


void testImportFromWriteonly(const QString& _dirname, QString mimetype = "")
{
    QString writeonlyFilename = _dirname + "writeonlyFile.txt";
    QFileInfo sourceFileInfo(writeonlyFilename);

    prepareFile(sourceFileInfo, false, true);

    KisDocument *doc = qobject_cast<KisDocument*>(KisPart::instance()->createDocument());

    KisImportExportManager manager(doc);
    doc->setFileBatchMode(true);

    KisImportExportErrorCode status = manager.importDocument(sourceFileInfo.absoluteFilePath(), mimetype);
    qDebug() << "import result = " << status;

    QString failMessage = "";
    bool fail = false;

    if (status == ImportExportCodes::FileFormatIncorrect) {
        qDebug() << "Make sure you set the correct mimetype in the test case.";
        failMessage = "Incorrect status.";
        fail = true;
    }

    qApp->processEvents();

    if (doc->image()) {
        doc->image()->waitForDone();
    }

    delete doc;

    restorePermissionsToReadAndWrite(sourceFileInfo);

    QVERIFY(!status.isOk());
    if (fail) {
        QFAIL(failMessage.toUtf8());
    }

}


void testExportToReadonly(const QString& _dirname, QString mimetype = "", bool useDocumentExport=false)
{
    QString readonlyFilename = _dirname + "readonlyFile.txt";

    QFileInfo sourceFileInfo(readonlyFilename);
    prepareFile(sourceFileInfo, true, false);

    KisDocument *doc = qobject_cast<KisDocument*>(KisPart::instance()->createDocument());

    KisImportExportManager manager(doc);
    doc->setFileBatchMode(true);

    KisImportExportErrorCode status = ImportExportCodes::OK;
    QString failMessage = "";
    bool fail = false;

    {
    MaskParent p;
    ENTER_FUNCTION() << doc->image();

    doc->setCurrentImage(p.image);

    if (useDocumentExport) {
        bool result = doc->exportDocumentSync(QUrl(sourceFileInfo.absoluteFilePath()), mimetype.toUtf8());
        status = result ? ImportExportCodes::OK : ImportExportCodes::Failure;
    } else {
        status = manager.exportDocument(sourceFileInfo.absoluteFilePath(), sourceFileInfo.absoluteFilePath(), mimetype.toUtf8());
    }

    qDebug() << "export result = " << status;

    if (!useDocumentExport && status == ImportExportCodes::FileFormatIncorrect) {
        qDebug() << "Make sure you set the correct mimetype in the test case.";
        failMessage = "Incorrect status.";
        fail = true;
    }


    qApp->processEvents();

    if (doc->image()) {
        doc->image()->waitForDone();
    }

    }
    delete doc;

    restorePermissionsToReadAndWrite(sourceFileInfo);

    QVERIFY(!status.isOk());
    if (fail) {
        QFAIL(failMessage.toUtf8());
    }
}



void testImportIncorrectFormat(const QString& _dirname, QString mimetype = "")
{
    QString incorrectFormatFilename = _dirname + "incorrectFormatFile.txt";
    QFileInfo sourceFileInfo(incorrectFormatFilename);

    prepareFile(sourceFileInfo, false, false);

    KisDocument *doc = qobject_cast<KisDocument*>(KisPart::instance()->createDocument());

    KisImportExportManager manager(doc);
    doc->setFileBatchMode(true);

    KisImportExportErrorCode status = manager.importDocument(sourceFileInfo.absoluteFilePath(), mimetype);
    qDebug() << "import result = " << status;


    qApp->processEvents();

    if (doc->image()) {
        doc->image()->waitForDone();
    }

    delete doc;

    QVERIFY(!status.isOk());
    QVERIFY(status == KisImportExportErrorCode(ImportExportCodes::FileFormatIncorrect)
            || status == KisImportExportErrorCode(ImportExportCodes::ErrorWhileReading)); // in case the filter doesn't know if it can't read or just parse

}


void testExportToColorSpace(const QString& _dirname, QString mimetype, const KoColorSpace* space, KisImportExportErrorCode expected, bool useDocumentExport=false)
{
    QString colorspaceFilename = _dirname + "colorspace.txt";

    QFileInfo sourceFileInfo(colorspaceFilename);
    prepareFile(sourceFileInfo, true, true);
    restorePermissionsToReadAndWrite(sourceFileInfo);

    KisDocument *doc = qobject_cast<KisDocument*>(KisPart::instance()->createDocument());

    KisImportExportManager manager(doc);
    doc->setFileBatchMode(true);

    KisImportExportErrorCode statusExport = ImportExportCodes::OK;
    KisImportExportErrorCode statusImport = ImportExportCodes::OK;

    QString failMessage = "";
    bool fail = false;

    {
    MaskParent p;

    doc->setCurrentImage(p.image);
    doc->image()->convertImageColorSpace(space, KoColorConversionTransformation::Intent::IntentPerceptual, KoColorConversionTransformation::ConversionFlag::Empty);
    doc->image()->waitForDone();

    if (useDocumentExport) {
        bool result = doc->exportDocumentSync(QUrl(QString("file:") + QString(colorspaceFilename)), mimetype.toUtf8());
        statusExport = result ? ImportExportCodes::OK : ImportExportCodes::Failure;
    } else {
        statusExport = manager.exportDocument(colorspaceFilename, colorspaceFilename, mimetype.toUtf8());
    }

    statusImport = manager.importDocument(colorspaceFilename, mimetype.toUtf8());
    if (!(statusImport == ImportExportCodes::OK)) {
        fail = true;
        failMessage = "Incorrect status";
    }

    bool mismatch = (*(doc->image()->colorSpace()) != *space) || (doc->image()->colorSpace()->profile() != space->profile());
    if (mismatch) {
        qDebug() << "Document color space = " << (doc->image()->colorSpace())->id();
        qDebug() << "Saved color space = " << space->id();
        fail = true;
        failMessage = "Mismatch of color spaces";
    }

    if (!useDocumentExport && statusExport == ImportExportCodes::FileFormatIncorrect) {
        qDebug() << "Make sure you set the correct mimetype in the test case.";
        failMessage = "Incorrect status.";
        fail = true;
    }


    qApp->processEvents();

    if (doc->image()) {
        doc->image()->waitForDone();
    }

    }
    delete doc;

    QFile::remove(colorspaceFilename);

    if (fail) {
        QFAIL(failMessage.toUtf8());
    }

    QVERIFY(statusExport.isOk());
    if (!useDocumentExport) {
        QVERIFY(statusExport == expected);
    }

}






}
#endif
