/* This file is part of the KDE project
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
 * Copyright (C) 2002 Rob Buis <buis@kde.org>
 * Copyright (C) 2004 Laurent Montel <montel@kde.org>
 * Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * Copyright (C) 2005 Inge Wallin <inge@lysator.liu.se>
 * Copyright (C) 2005, 2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2005-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>
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

#include "kowidgets_export.h"

#include <QtGui/QWidget>

class KoUnit;
class KoLineBorder;

/// A widget for configuring the stroke of a shape
class KOWIDGETS_EXPORT KoStrokeConfigWidget : public QWidget
{
    Q_OBJECT
public:
    KoStrokeConfigWidget( QWidget * parent );
    ~KoStrokeConfigWidget();

    // Getters
    Qt::PenStyle lineStyle() const;
    QVector<qreal> lineDashes() const;
    qreal lineWidth() const;
    qreal miterLimit() const;

    void updateControls(KoLineBorder &border);

    void locationChanged(Qt::DockWidgetArea area);

public slots:
    void setUnit( const KoUnit &unit );

signals:
    void currentIndexChanged();
    void widthChanged();
    void capChanged(int button);
    void joinChanged(int button);
    void miterLimitChanged();
#if 0
    /// Is emitted whenever the shadow color has changed
    void shadowColorChanged( const KoColor &color );

    /// Is emitted whenever the shadow offset has changed
    void shadowOffsetChanged( const QPointF &offset );

    /// Is emitted whenever the shadow blur radius has changed
    void shadowBlurChanged( const qreal &blur );

    /// Is emitted whenever the shadow visibility has changed
    void shadowVisibilityChanged( bool visible );
#endif

private:
    void blockChildSignals(bool block);


private slots:
#if 0
    void visibilityChanged();
    void offsetChanged();
    void blurChanged();
#endif
private:
    class Private;
    Private * const d;
};

#endif // SHADOWCONFIGWIDGET_H
