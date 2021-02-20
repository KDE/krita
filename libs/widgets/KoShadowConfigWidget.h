/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 C. Boemann <cbo@boemann.dk>
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
