/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_RESOURCE_THUMBNAIL_PAINTER_H
#define KIS_RESOURCE_THUMBNAIL_PAINTER_H

#include <QAbstractItemDelegate>

#include "KoCheckerBoardPainter.h"

#include "kritaresourcewidgets_export.h"
#include <KisResourceTypes.h>
#include <QObject>

/// The resource item delegate for rendering the resource preview
class KRITARESOURCEWIDGETS_EXPORT KisResourceThumbnailPainter : public QObject
{
    Q_OBJECT
public:
    explicit KisResourceThumbnailPainter(QObject *parent = 0);
    ~KisResourceThumbnailPainter() override {}

    //  (QPainter*, QModelIndex&, QRect, const QPalette&, bool)’

    QImage getReadyThumbnail(const QModelIndex& index, QSize rect, const QPalette& palette) const;
    void paint(QPainter *painter, const QModelIndex& index, QRect rect, const QPalette& palette, bool selected, bool addMargin) const;
    void paint(QPainter *painter, QImage thumbnail, QString resourceType, QString name, QRect rect, const QPalette& palette, bool selected, bool addMargin) const;

private:
    KoCheckerBoardPainter m_checkerPainter;
};

#endif
