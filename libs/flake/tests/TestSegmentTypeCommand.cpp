/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#include "TestSegmentTypeCommand.h"
#include "KoPathSegmentTypeCommand.h"

void TestSegmentTypeCommand::changeToCurve()
{
    KoPathShape path;
    path.moveTo( QPointF(0,0) );
    path.lineTo( QPointF(100,0) );

    KoPathPointData segment(&path, KoPathPointIndex(0,0));
    QList<KoPathPointData> segments;
    segments.append(segment);

    // get first segment
    KoPathSegment s = path.segmentByIndex(KoPathPointIndex(0,0));

    KoPathSegmentTypeCommand cmd(segments, KoPathSegmentTypeCommand::Curve);

    QVERIFY(!s.first()->activeControlPoint2());
    QVERIFY(!s.second()->activeControlPoint1());

    cmd.redo();

    QVERIFY(s.first()->activeControlPoint2());
    QVERIFY(s.second()->activeControlPoint1());

    cmd.undo();

    QVERIFY(!s.first()->activeControlPoint2());
    QVERIFY(!s.second()->activeControlPoint1());
}

void TestSegmentTypeCommand::changeToLine()
{
    KoPathShape path;
    path.moveTo( QPointF(0,0) );
    path.curveTo( QPointF(25,25), QPointF(75,25), QPointF(100,0) );

    KoPathPointData segment(&path, KoPathPointIndex(0,0));
    QList<KoPathPointData> segments;
    segments.append(segment);

    // get first segment
    KoPathSegment s = path.segmentByIndex(KoPathPointIndex(0,0));

    KoPathSegmentTypeCommand cmd(segments, KoPathSegmentTypeCommand::Line);

    QVERIFY(s.first()->activeControlPoint2());
    QVERIFY(s.second()->activeControlPoint1());

    cmd.redo();

    QVERIFY(!s.first()->activeControlPoint2());
    QVERIFY(!s.second()->activeControlPoint1());

    cmd.undo();

    QVERIFY(s.first()->activeControlPoint2());
    QVERIFY(s.second()->activeControlPoint1());
}

QTEST_MAIN(TestSegmentTypeCommand)
#include <TestSegmentTypeCommand.moc>
