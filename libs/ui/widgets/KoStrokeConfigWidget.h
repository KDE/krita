/* This file is part of the KDE project
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
 * Copyright (C) 2002 Rob Buis <buis@kde.org>
 * Copyright (C) 2004 Laurent Montel <montel@kde.org>
 * Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * Copyright (C) 2005 Inge Wallin <inge@lysator.liu.se>
 * Copyright (C) 2005, 2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2005-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2011 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 * Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef STROKECONFIGWIDGET_H
#define STROKECONFIGWIDGET_H

#include "kritaui_export.h"

#include <QWidget>
#include <QLabel>
#include <KoFlakeTypes.h>
#include <KoFlake.h>


class KoUnit;
class KoShapeStrokeModel;
class KoShapeStroke;
class KoMarker;
class KoCanvasBase;
class KoShapeStroke;
class KisSpinBoxUnitManager;

/// A widget for configuring the stroke of a shape
class KRITAUI_EXPORT KoStrokeConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KoStrokeConfigWidget(KoCanvasBase *canvas, QWidget *parent);
    ~KoStrokeConfigWidget() override;

    void setNoSelectionTrackingMode(bool value);

    // Getters
    Qt::PenStyle lineStyle() const;
    QVector<qreal> lineDashes() const;
    qreal lineWidth() const;
    QColor color() const;
    qreal miterLimit() const;
    KoMarker *startMarker() const;
    KoMarker *endMarker() const;
    Qt::PenCapStyle capStyle() const;
    Qt::PenJoinStyle joinStyle() const;
    QLabel *thicknessLabel;
    QLabel *strokeStyleLabel;
    QFrame* separatorLine;

    /**
     * Creates KoShapeStroke object filled with the options
     * configured by the widget. The caller is in charge of
     * deletion of the returned object
     */
    KoShapeStrokeSP createShapeStroke();

    void setActive(bool active);

    void updateStyleControlsAvailability(bool enabled);
    void setUnitManagers(KisSpinBoxUnitManager* managerLineWidth, KisSpinBoxUnitManager* managerMitterLimit);

    void activate();
    void deactivate();

private Q_SLOTS:

    void updateMarkers(const QList<KoMarker*> &markers);

    void canvasResourceChanged(int key, const QVariant &value);

    /// selection has changed
    void selectionChanged();

    /// apply line changes to the selected shapes
    void applyDashStyleChanges();
    void applyLineWidthChanges();
    void applyJoinCapChanges();

    /// apply marker changes to the selected shape
    void applyMarkerChanges(int rawPosition);

Q_SIGNALS:
    void sigStrokeChanged();

private:
    void setUnit(const KoUnit &unit, KoShape *representativeShape);
    void blockChildSignals(bool block);
    void loadCurrentStrokeFillFromResourceServer();

private:
    class Private;
    Private * const d;
};

#endif // SHADOWCONFIGWIDGET_H
