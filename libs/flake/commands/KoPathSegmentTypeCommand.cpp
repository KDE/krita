/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006, 2009 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoPathSegmentTypeCommand.h"

#include <KoPathSegment.h>
#include <klocalizedstring.h>

KoPathSegmentTypeCommand::KoPathSegmentTypeCommand(const KoPathPointData & pointData, SegmentType segmentType, KUndo2Command *parent)
: KUndo2Command(parent)
, m_segmentType(segmentType)
{
    QList<KoPathPointData> pointDataList;
    pointDataList.append(pointData);
    initialize(pointDataList);
}

KoPathSegmentTypeCommand::KoPathSegmentTypeCommand(const QList<KoPathPointData> & pointDataList, SegmentType segmentType,
        KUndo2Command *parent)
        : KUndo2Command(parent)
        , m_segmentType(segmentType)
{
    initialize(pointDataList);
}

KoPathSegmentTypeCommand::~KoPathSegmentTypeCommand()
{
}

void KoPathSegmentTypeCommand::redo()
{
    KUndo2Command::redo();
    QList<KoPathPointData>::const_iterator it(m_pointDataList.constBegin());
    for (; it != m_pointDataList.constEnd(); ++it) {
        KoPathShape * pathShape = it->pathShape;
        pathShape->update();

        KoPathSegment segment = pathShape->segmentByIndex(it->pointIndex);

        if (m_segmentType == Curve) {
            // we change type to curve -> set control point positions
            QPointF pointDiff = segment.second()->point() - segment.first()->point();
            segment.first()->setControlPoint2(segment.first()->point() + pointDiff / 3.0);
            segment.second()->setControlPoint1(segment.first()->point() + pointDiff * 2.0 / 3.0);
        } else {
            // we are changing type to line -> remove control points
            segment.first()->removeControlPoint2();
            segment.second()->removeControlPoint1();
        }

        pathShape->normalize();
        pathShape->update();
    }
}

void KoPathSegmentTypeCommand::undo()
{
    KUndo2Command::undo();
    for (int i = 0; i < m_pointDataList.size(); ++i) {
        const KoPathPointData & pd = m_pointDataList.at(i);
        pd.pathShape->update();
        KoPathSegment segment = pd.pathShape->segmentByIndex(pd.pointIndex);
        const SegmentTypeData segmentData(m_segmentData.at(i));

        if (m_segmentType == Line) {
            // change type back to curve -> reactivate control points and their positions
            segment.first()->setControlPoint2(pd.pathShape->documentToShape(segmentData.m_controlPoint2));
            segment.second()->setControlPoint1(pd.pathShape->documentToShape(segmentData.m_controlPoint1));
        } else {
            // change back to line -> remove control points
            segment.first()->removeControlPoint2();
            segment.second()->removeControlPoint1();
        }

        segment.first()->setProperties(segmentData.m_properties2);
        segment.second()->setProperties(segmentData.m_properties1);

        pd.pathShape->normalize();
        pd.pathShape->update();
    }
}

void KoPathSegmentTypeCommand::initialize(const QList<KoPathPointData> & pointDataList)
{
    QList<KoPathPointData>::const_iterator it(pointDataList.begin());
    for (; it != pointDataList.end(); ++it) {
        KoPathSegment segment = it->pathShape->segmentByIndex(it->pointIndex);
        if (segment.isValid()) {
            if (m_segmentType == Curve) {
                // don not change segment if already a curve
                if (segment.first()->activeControlPoint2() || segment.second()->activeControlPoint1())
                    continue;
            } else {
                // do not change segment if already a line
                if (! segment.first()->activeControlPoint2() && ! segment.second()->activeControlPoint1())
                    continue;
            }

            m_pointDataList.append(*it);
            SegmentTypeData segmentData;

            KoPathShape * pathShape = segment.first()->parent();

            // we are changing a curve to a line -> save control point positions
            if (m_segmentType == Line) {
                segmentData.m_controlPoint2 = pathShape->shapeToDocument(segment.first()->controlPoint2());
                segmentData.m_controlPoint1 = pathShape->shapeToDocument(segment.second()->controlPoint1());
            }
            // save point properties
            segmentData.m_properties2 = segment.first()->properties();
            segmentData.m_properties1 = segment.second()->properties();
            m_segmentData.append(segmentData);
        }
    }

    if (m_segmentType == Curve) {
        setText(kundo2_i18n("Change segments to curves"));
    } else {
        setText(kundo2_i18n("Change segments to lines"));
    }
}
