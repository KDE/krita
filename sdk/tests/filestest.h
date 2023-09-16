/*
 * SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef FILESTEST
#define FILESTEST

#include <testutil.h>
#include "testui.h"

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

#ifdef Q_OS_UNIX
#   include <unistd.h>
#endif

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
            resultImage.convertTo(QImage::Format_ARGB32);
            sourceImage.convertTo(QImage::Format_ARGB32);

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

    bool success = QFile::setPermissions(sourceFileInfo.absoluteFilePath(), permissionsNow);
    if (!success) {
        qWarning() << "prepareFile(): Failed to set permission of file" << sourceFileInfo.absoluteFilePath()
                   << "from" << permissionsBefore << "to" << permissionsNow;
    }
}

void restorePermissionsToReadAndWrite(QFileInfo sourceFileInfo)
{
    QFileDevice::Permissions permissionsNow = sourceFileInfo.permissions();
    QFileDevice::Permissions permissionsAfter = permissionsNow
            | (QFileDevice::ReadUser | QFileDevice::ReadOwner
            | QFileDevice::ReadGroup | QFileDevice::ReadOther)
            | (QFileDevice::WriteUser | QFileDevice::WriteOwner
            | QFileDevice::WriteGroup | QFileDevice::WriteOther);
    bool success = QFile::setPermissions(sourceFileInfo.absoluteFilePath(), permissionsAfter);
    if (!success) {
        qWarning() << "restorePermissionsToReadAndWrite(): Failed to set permission of file" << sourceFileInfo.absoluteFilePath()
                   << "from" << permissionsNow << "to" << permissionsAfter;
    }
}

const QString &impexTempFilesDir() {
    static const QString s_path = []() {
        const QString path = QDir::cleanPath(
                QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/impex_test") + '/';
        QDir(path).mkpath(QStringLiteral("."));
        return path;
    }();
    return s_path;
}


void testImportFromWriteonly(QString mimetype)
{
#ifdef Q_OS_WIN
    /// on Windows one cannot create a write-only file, so just skip this test
    /// (but keep it compiled to avoid compilation issues)
    QSKIP("Cannot test write-only file on Windows.");
#endif

#ifdef Q_OS_UNIX
    if (geteuid() == 0) {
        QSKIP("Test is being run as root; removing read permission has no effect.");
    }
#endif

    QString writeonlyFilename = impexTempFilesDir() + "writeonlyFile.txt";
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

    if (fail || status.isOk()) {
        qDebug() << "The file permission is:" << QFile::permissions(sourceFileInfo.absoluteFilePath());
    }

    restorePermissionsToReadAndWrite(sourceFileInfo);

    QVERIFY(!status.isOk());
    if (fail) {
        QFAIL(failMessage.toUtf8());
    }

}


void testExportToReadonly(QString mimetype)
{
#ifdef Q_OS_UNIX
    if (geteuid() == 0) {
        QSKIP("Test is being run as root; removing write permission has no effect.");
    }
#endif

    QString readonlyFilename = impexTempFilesDir() + "readonlyFile.txt";

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

    bool result = doc->exportDocumentSync(sourceFileInfo.absoluteFilePath(), mimetype.toUtf8());
    status = result ? ImportExportCodes::OK : ImportExportCodes::Failure;

    qDebug() << "export result = " << status;

    qApp->processEvents();

    if (doc->image()) {
        doc->image()->waitForDone();
    }

    }
    delete doc;

    if (status.isOk()) {
        qDebug() << "The file permission is:" << QFile::permissions(sourceFileInfo.absoluteFilePath());
    }

    restorePermissionsToReadAndWrite(sourceFileInfo);

    QVERIFY(!status.isOk());
    if (fail) {
        QFAIL(failMessage.toUtf8());
    }
}



void testImportIncorrectFormat(QString mimetype)
{
    QString incorrectFormatFilename = impexTempFilesDir() + "incorrectFormatFile.txt";
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


void testExportToColorSpace(QString mimetype, const KoColorSpace* space, KisImportExportErrorCode expected)
{
    QString colorspaceFilename = impexTempFilesDir() + "colorspace.txt";

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

    bool result = doc->exportDocumentSync(QString(colorspaceFilename), mimetype.toUtf8());
    statusExport = result ? ImportExportCodes::OK : ImportExportCodes::Failure;

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
    QVERIFY(statusExport == expected);
}






}
#endif
