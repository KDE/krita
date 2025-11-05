/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextChangeTransformsOnRange.h"
#include "kis_command_ids.h"

SvgTextChangeTransformsOnRange::SvgTextChangeTransformsOnRange(KoSvgTextShape *shape, int startPos, int endPos, QVector<QPointF> positions, QVector<qreal> rotations, bool calculateDeltaPositions, KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand)
    , m_textShape(shape)
    , m_startPos(startPos)
    , m_endPos(endPos)
    , m_positions(positions)
    , m_rotations(rotations)
    , m_calculateDeltaPositions(calculateDeltaPositions)
    , m_textData(shape->getMemento())
{
    setText(kundo2_i18n("Adjust character transforms on text."));
}

SvgTextChangeTransformsOnRange::SvgTextChangeTransformsOnRange(KoSvgTextShape *shape, int startPos, int endPos, QPointF delta, OffsetType type, bool calculateDeltaPositions, KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand)
    , m_textShape(shape)
    , m_startPos(startPos)
    , m_endPos(endPos)
    , m_calculateDeltaPositions(calculateDeltaPositions)
    , m_textData(shape->getMemento())
{
    setText(kundo2_i18n("Adjust character transforms on text."));

    QVector<QPointF> positions;
    QVector<qreal> rotations;

    QList<KoSvgTextCharacterInfo> originalTf = shape->getPositionsAndRotationsForRange(startPos, endPos);

    if (type == OffsetAll) {

        while (!originalTf.isEmpty()) {
            KoSvgTextCharacterInfo tf = originalTf.takeFirst();
            positions.append(tf.finalPos + delta);

            rotations.append(tf.rotateDeg);
        }
    } else {
        QTransform deltaTf = getTransformForOffset(shape, startPos, endPos, delta, type);
        QLineF l(0, 0, 10, 0);
        l.setAngle(0);
        l = deltaTf.map(l);

        while (!originalTf.isEmpty()) {
            KoSvgTextCharacterInfo tf = originalTf.takeFirst();
            if (type != RotateOnly) {
                positions.append(deltaTf.map(tf.finalPos));
            } else {
                positions.append(tf.finalPos);
            }
            if (type != ScaleOnly) {
                rotations.append(tf.rotateDeg - (l.angle()));
            } else {
                rotations.append(tf.rotateDeg);
            }
        }
    }

    m_positions = positions;
    m_rotations = rotations;

}

void SvgTextChangeTransformsOnRange::undo()
{
    QRectF updateRect = m_textShape->boundingRect();

    m_textShape->setMemento(m_textData);

    updateRect |= m_textShape->boundingRect();
    m_textShape->notifyCursorPosChanged(m_startPos, m_endPos);
    m_textShape->updateAbsolute(updateRect);
}

void SvgTextChangeTransformsOnRange::redo()
{
    QRectF updateRect = m_textShape->boundingRect();

    const int posIndex = m_textShape->indexForPos(m_startPos);
    const int posAnchor = m_textShape->indexForPos(m_endPos);

    m_textShape->setCharacterTransformsOnRange(m_startPos, m_endPos, m_positions, m_rotations, m_calculateDeltaPositions);

    updateRect |= m_textShape->boundingRect();
    m_textShape->notifyCursorPosChanged(m_textShape->posForIndex(posIndex), m_textShape->posForIndex(posAnchor));
    m_textShape->updateAbsolute(updateRect);
}

int SvgTextChangeTransformsOnRange::id() const
{
    return KisCommandUtils::SvgTextChangeTransformsOnRangeCommandId;
}

bool SvgTextChangeTransformsOnRange::mergeWith(const KUndo2Command *otherCommand) {
    const SvgTextChangeTransformsOnRange *other = dynamic_cast<const SvgTextChangeTransformsOnRange *>(otherCommand);

    if (!other || other->m_textShape != m_textShape || other->m_startPos != m_startPos || other->m_endPos != m_endPos || other->m_calculateDeltaPositions != m_calculateDeltaPositions) {
        return false;
    }

    m_positions = other->m_positions;
    m_rotations = other->m_rotations;
    return true;
}

QTransform SvgTextChangeTransformsOnRange::getTransformForOffset(KoSvgTextShape *shape, int startPos, int endPos, QPointF delta, OffsetType type)
{
    QTransform deltaTf;
    if (type == OffsetAll) {
        return QTransform::fromTranslate(delta.x(), delta.y());
    } else {
        const int lineEnd = qMin(shape->lineEnd(qMin(startPos, endPos)), qMax(startPos, endPos));
        QList<KoSvgTextCharacterInfo> infos =
                shape->getPositionsAndRotationsForRange(qMin(startPos, endPos), lineEnd);
        if (infos.size() < 1) return deltaTf;
        const bool rtl = infos.first().rtl;
        KoSvgTextCharacterInfo first = infos.first();
        KoSvgTextCharacterInfo last = infos.last();
        if (infos.size() > 1) {
            std::sort(infos.begin(), infos.end(), KoSvgTextCharacterInfo::visualLessThan);
            for (auto it = infos.begin(); it != infos.end(); it++) {
                if (it->visualIndex >= 0) {
                    first = *it;
                    break;
                }
            }
            for (auto it = infos.rbegin(); it != infos.rend(); it++) {
                if (it->visualIndex >= 0) {
                    last = *it;
                    break;
                }
            }
        }
        QTransform t = QTransform::fromTranslate(last.finalPos.x(), last.finalPos.y());
        t.rotate(last.rotateDeg);
        last.finalPos = t.map(last.advance);

        QLineF originalLine(rtl? last.finalPos: first.finalPos, rtl? first.finalPos: last.finalPos);
        QLineF newLine(originalLine.p1(), originalLine.p2() + delta);

        const qreal scale = newLine.length()/originalLine.length();

        QTransform origin = QTransform::fromTranslate(originalLine.p1().x(), originalLine.p1().y());
        qreal angle = originalLine.angle() - newLine.angle();

        deltaTf = origin.inverted();
        deltaTf *= QTransform::fromScale(scale, scale);
        deltaTf *= QTransform().rotate(angle);
        deltaTf *= origin;
    }
    return deltaTf;
}
