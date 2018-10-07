/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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
    return mimeData && mimeData->hasFormat("image/svg+xml");
}

QList<KoShape*> KoSvgPaste::fetchShapes(const QRectF viewportInPx, qreal resolutionPPI, QSizeF *fragmentSize)
{
    QList<KoShape*> shapes;

    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData) return shapes;

    QByteArray data = mimeData->data("image/svg+xml");
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
