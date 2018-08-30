/* This file is part of the KDE project
   Copyright (C) 2002 Lars Siebold <khandha5@gmx.net>
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2002-2003,2005 Rob Buis <buis@kde.org>
   Copyright (C) 2005 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2005 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2005,2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2006 Laurent Montel <montel@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef SVGWRITER_H
#define SVGWRITER_H

#include "kritaflake_export.h"
#include <QList>
#include <QSizeF>

class SvgSavingContext;
class KoShapeLayer;
class KoShapeGroup;
class KoShape;
class KoPathShape;
class QIODevice;
class QString;

/// Implements exporting shapes to SVG
class KRITAFLAKE_EXPORT SvgWriter
{
public:
    /// Creates svg writer to export specified layers
    SvgWriter(const QList<KoShapeLayer*> &layers);

    /// Creates svg writer to export specified shapes
    SvgWriter(const QList<KoShape*> &toplevelShapes);

    /// Destroys the svg writer
    virtual ~SvgWriter();

    /// Writes svg to specified output device
    bool save(QIODevice &outputDevice, const QSizeF &pageSize);

    /// Writes svg to the specified file
    bool save(const QString &filename, const QSizeF &pageSize, bool writeInlineImages);

    bool saveDetached(QIODevice &outputDevice);

    bool saveDetached(SvgSavingContext &savingContext);

    void setDocumentTitle(QString title);
    void setDocumentDescription(QString description);

private:
    void saveShapes(const QList<KoShape*> shapes, SvgSavingContext &savingContext);

    void saveLayer(KoShapeLayer *layer, SvgSavingContext &context);
    void saveGroup(KoShapeGroup *group, SvgSavingContext &context);
    void saveShape(KoShape *shape, SvgSavingContext &context);
    void savePath(KoPathShape *path, SvgSavingContext &context);
    void saveGeneric(KoShape *shape, SvgSavingContext &context);

    QList<KoShape*> m_toplevelShapes;
    bool m_writeInlineImages;
    QString m_documentTitle;
    QString m_documentDescription;
};

#endif // SVGWRITER_H
