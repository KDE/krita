/* This file is part of the KDE project
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOSHADOWCONFIGWIDGET_H
#define KOSHADOWCONFIGWIDGET_H

#include "kritawidgets_export.h"
#include <QWidget>

class KoUnit;
class KoCanvasBase;
class KisSpinBoxUnitManager;

/// A widget for configuring the shadow of a shape
class KRITAWIDGETS_EXPORT KoShadowConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KoShadowConfigWidget(QWidget *parent);
    ~KoShadowConfigWidget() override;

    /// Sets the shadow color
    void setShadowColor(const QColor &color);

    /// Returns the shadow color
    QColor shadowColor() const;

    /// Sets the shadow offset
    void setShadowOffset(const QPointF &offset);

    /// Returns the shadow offset
    QPointF shadowOffset() const;

    /// Sets the shadow blur radius
    void setShadowBlur(const qreal &blur);

    /// Returns the shadow blur radius
    qreal shadowBlur() const;

    /// Sets if the shadow is visible
    void setShadowVisible(bool visible);

    /// Returns if shadow is visible
    bool shadowVisible() const;

public Q_SLOTS:

    void setUnitManagers(KisSpinBoxUnitManager* managerBlur, KisSpinBoxUnitManager* managerOffset);
    void setUnit( const KoUnit &unit );
    void setCanvas(KoCanvasBase *canvas);

private Q_SLOTS:
    void visibilityChanged();
    void applyChanges();
    void selectionChanged();
    void resourceChanged( int key, const QVariant & res );

private:
    class Private;
    Private *const d;
};

#endif // KOSHADOWCONFIGWIDGET_H
