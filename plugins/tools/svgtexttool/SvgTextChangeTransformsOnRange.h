/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGTEXTCHANGETRANSFORMSONRANGE_H
#define SVGTEXTCHANGETRANSFORMSONRANGE_H

#include <kundo2command.h>
#include <KoSvgTextShape.h>


class SvgTextChangeTransformsOnRange : public KUndo2Command
{
public:
    SvgTextChangeTransformsOnRange(KoSvgTextShape *shape, int startPos, int endPos, QVector<QPointF> positions, QVector<qreal> rotations, bool calculateDeltaPositions, KUndo2Command *parentCommand = nullptr);

    enum OffsetType{
        OffsetAll,
        ScaleAndRotate,
        ScaleOnly,
        RotateOnly
    };

    SvgTextChangeTransformsOnRange(KoSvgTextShape *shape, int startPos, int endPos, QPointF delta, OffsetType type, bool calculateDeltaPositions, KUndo2Command *parentCommand = nullptr);
    ~SvgTextChangeTransformsOnRange() = default;
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const KUndo2Command *other) override;
private:
    KoSvgTextShape *m_textShape = nullptr;
    int m_startPos = -1;
    int m_endPos = -1;
    QVector<QPointF> m_positions;
    QVector<qreal> m_rotations;
    bool m_calculateDeltaPositions = false;
    KoSvgTextShapeMementoSP m_textData;
};

#endif // SVGTEXTCHANGETRANSFORMSONRANGE_H
