/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestSegmentTypeCommand.h"
#include "KoPathSegmentTypeCommand.h"
#include <KoPathSegment.h>
#include <simpletest.h>

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

SIMPLE_TEST_MAIN(TestSegmentTypeCommand)
