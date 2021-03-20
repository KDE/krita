/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SVGCLIPPATHHELPER_H
#define SVGCLIPPATHHELPER_H

#include <KoFlakeCoordinateSystem.h>
#include <QList>

class KoShape;

class SvgClipPathHelper
{
public:
    SvgClipPathHelper();
    ~SvgClipPathHelper();

    /// Set the clip path units type
    void setClipPathUnits(KoFlake::CoordinateSystem clipPathUnits);
    /// Returns the clip path units type
    KoFlake::CoordinateSystem clipPathUnits() const;

    QList<KoShape *> shapes() const;
    void setShapes(const QList<KoShape *> &shapes);

    bool isEmpty() const;

private:
    KoFlake::CoordinateSystem m_clipPathUnits;
    QList<KoShape*> m_shapes;
};

#endif // SVGCLIPPATHHELPER_H
