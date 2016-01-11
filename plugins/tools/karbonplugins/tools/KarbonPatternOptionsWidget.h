/* This file is part of the KDE project
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

#ifndef KARBONPATTERNOPTIONSWIDGET_H
#define KARBONPATTERNOPTIONSWIDGET_H

#include <KoPatternBackground.h>

#include <QWidget>

class KarbonPatternOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KarbonPatternOptionsWidget(QWidget *parent = 0);
    virtual ~KarbonPatternOptionsWidget();

    /// Sets the pattern repeat
    void setRepeat(KoPatternBackground::PatternRepeat repeat);

    /// Return the pattern repeat
    KoPatternBackground::PatternRepeat repeat() const;

    /// Returns the pattern reference point identifier
    KoPatternBackground::ReferencePoint referencePoint() const;

    /// Sets the pattern reference point
    void setReferencePoint(KoPatternBackground::ReferencePoint referencePoint);

    /// Returns reference point offset in percent of the size to fill
    QPointF referencePointOffset() const;

    /// Sets the reference point offset in percent of the size to fill
    void setReferencePointOffset(const QPointF &offset);

    /// Returns tile repeat offset in percent of the size to fill
    QPointF tileRepeatOffset() const;

    /// Sets the tile repeat offset in percent of the size to fill
    void setTileRepeatOffset(const QPointF &offset);

    /// Returns the pattern size
    QSize patternSize() const;

    /// Sets the pattern size
    void setPatternSize(const QSize &size);

Q_SIGNALS:
    /// is emitted whenever an option has changed
    void patternChanged();
private Q_SLOTS:
    void updateControls();
private:
    class Private;
    Private *const d;

};

#endif // KARBONPATTERNOPTIONSWIDGET_H
