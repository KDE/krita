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

#include "testutil.h"

#include <QDir>

#include <kaboutdata.h>
#include <kimageio.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmimetype.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kio/netaccess.h>
#include <kio/job.h>

#include <KoFilterManager.h>

#include <kis_doc2.h>
#include <KoPart.h>
#include <kis_image.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <ktemporaryfile.h>
#include <QFileInfo>

namespace TestUtil
{

void testFiles(const QString& _dirname, const QStringList& exclusions, const QString &resultSuffix = QString(), int fuzzy = 0)
{
    QDir dirSources(_dirname);
    QStringList failuresFileInfo;
    QStringList failuresDocImage;
    QStringList failuresCompare;

    foreach(QFileInfo sourceFileInfo, dirSources.entryInfoList()) {
        if (exclusions.indexOf(sourceFileInfo.fileName()) > -1) {
            continue;
        }
        if (!sourceFileInfo.isHidden() && !sourceFileInfo.isDir()) {
            QFileInfo resultFileInfo(QString(FILES_DATA_DIR) + "/results/" + sourceFileInfo.fileName() + resultSuffix + ".png");

            if (!resultFileInfo.exists()) {
                failuresFileInfo << resultFileInfo.fileName();
                continue;
            }

            KisDoc2 doc;
            KoFilterManager manager(&doc);
            manager.setBatchMode(true);
            QByteArray nativeFormat = doc.nativeFormatMimeType();
            KoFilter::ConversionStatus status;
            QString s = manager.importDocument(sourceFileInfo.absoluteFilePath(), QString(),
                                               status);

            if (!doc.image()) {
                failuresDocImage << sourceFileInfo.fileName();
                continue;
            }

            QString id = doc.image()->colorSpace()->id();
            if (id != "GRAYA" && id != "GRAYA16" && id != "RGBA" && id != "RGBA16") {
                dbgKrita << "Images need conversion";
                doc.image()->convertImageColorSpace(KoColorSpaceRegistry::instance()->rgb8(),
                                                    KoColorConversionTransformation::IntentPerceptual,
                                                    KoColorConversionTransformation::Empty);
            }

            KTemporaryFile tmpFile;
            tmpFile.setSuffix(".png");
            tmpFile.open();
            doc.setBackupFile(false);
            doc.setOutputMimeType("image/png");
            doc.documentPart()->saveAs("file://" + tmpFile.fileName());

            QImage resultImage(resultFileInfo.absoluteFilePath());
            resultImage = resultImage.convertToFormat(QImage::Format_ARGB32);
            QImage sourceImage(tmpFile.fileName());
            sourceImage = sourceImage.convertToFormat(QImage::Format_ARGB32);

            tmpFile.close();

            QPoint pt;

            if (!TestUtil::compareQImages(pt, resultImage, sourceImage, fuzzy)) {
                failuresCompare << sourceFileInfo.fileName() + ": " + QString("Pixel (%1,%2) has different values").arg(pt.x()).arg(pt.y()).toLatin1();
                resultImage.save(sourceFileInfo.fileName() + ".png");
                continue;
            }
        }
    }
    if (failuresCompare.isEmpty() && failuresDocImage.isEmpty() && failuresFileInfo.isEmpty()) {
        return;
    }
    qDebug() << "Comparison failures: " << failuresCompare;
    qDebug() << "No image failures: " << failuresDocImage;
    qDebug() << "No comparison image: " <<  failuresFileInfo;

    QFAIL("Failed testing files");
}

}
