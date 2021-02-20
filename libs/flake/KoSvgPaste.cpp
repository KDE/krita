/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgPaste.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include <SvgParser.h>
#include <KoDocumentResourceManager.h>
#include <KoXmlReader.h>
#include <FlakeDebug.h>
#include <QRectF>

bool KoSvgPaste::hasShapes()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    bool hasSvg = false;
    if (mimeData) {
        Q_FOREACH(const QString &format, mimeData->formats()) {
            if (format.toLower().contains("svg")) {
                hasSvg = true;
                break;
            }
        }
    }

    return hasSvg;
}

QList<KoShape*> KoSvgPaste::fetchShapes(const QRectF viewportInPx, qreal resolutionPPI, QSizeF *fragmentSize)
{
    QList<KoShape*> shapes;

    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData) return shapes;

    QByteArray data;

    Q_FOREACH(const QString &format, mimeData->formats()) {
        if (format.toLower().contains("svg")) {
            data = mimeData->data(format);
            break;
        }
    }

    if (data.isEmpty()) {
        return shapes;
    }

    return fetchShapesFromData(data, viewportInPx, resolutionPPI, fragmentSize);

}

QList<KoShape*> KoSvgPaste::fetchShapesFromData(const QByteArray &data, const QRectF viewportInPx, qreal resolutionPPI, QSizeF *fragmentSize)
{
    QList<KoShape*> shapes;

    if (data.isEmpty()) {
        return shapes;
    }



    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    KoXmlDocument doc = SvgParser::createDocumentFromSvg(data, &errorMsg, &errorLine, &errorColumn);
    if (doc.isNull()) {
        qWarning() << "Failed to process an SVG file at"
                   << errorLine << ":" << errorColumn << "->" << errorMsg;
        return shapes;
    }

    KoDocumentResourceManager resourceManager;
    SvgParser parser(&resourceManager);
    parser.setResolution(viewportInPx, resolutionPPI);

    shapes = parser.parseSvg(doc.documentElement(), fragmentSize);

    return shapes;
}
