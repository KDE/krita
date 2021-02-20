/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PAINTING_TWEAKS_H
#define __KIS_PAINTING_TWEAKS_H

#include "kritaglobal_export.h"

#include <QPen>
#include <QBrush>

class QPainter;
class QRegion;
class QRect;
class QPen;

namespace KisPaintingTweaks {

    /**
     * This is a workaround for QPainter::clipRegion() bug. When zoom
     * is about 2000% and rotation is in a range[-5;5] degrees, the
     * generated region will have about 20k+ regtangles inside. Their
     * processing will be really slow. These functions fworkarounds
     * the issue.
     */
    KRITAGLOBAL_EXPORT QRegion safeClipRegion(const QPainter &painter);

    /**
     * \see safeClipRegion()
     */
    KRITAGLOBAL_EXPORT QRect safeClipBoundingRect(const QPainter &painter);

    KRITAGLOBAL_EXPORT void initAntsPen(QPen *antsPen, QPen *outlinePen,
                                        int antLength = 4, int antSpace = 4);


    /**
     * A special class to save painter->pen() and painter->brush() using RAII
     * principle.
     */
    class KRITAGLOBAL_EXPORT PenBrushSaver
    {
    public:
        struct allow_noop_t { explicit allow_noop_t() = default; };
        static constexpr allow_noop_t	allow_noop { };

        /**
         * Saves pen and brush state of the provided painter object. \p painter cannot be null.
         */
        PenBrushSaver(QPainter *painter);

        /**
         * Overrides pen and brush of \p painter with the provided values. \p painter cannot be null.
         */
        PenBrushSaver(QPainter *painter, const QPen &pen, const QBrush &brush);

        /**
         * Overrides pen and brush of \p painter with the provided values. \p painter cannot be null.
         */
        PenBrushSaver(QPainter *painter, const QPair<QPen, QBrush> &pair);

        /**
         * A special constructor of PenBrushSaver that allows \p painter to be null. Passing null
         * pointer will basically mean that the whole saver existence will be a noop.
         */
        PenBrushSaver(QPainter *painter, const QPair<QPen, QBrush> &pair, allow_noop_t);

        /**
         * Restores the state of the painter that has been saved during the construction of the saver
         */
        ~PenBrushSaver();

    private:
        PenBrushSaver(const PenBrushSaver &rhs) = delete;
        QPainter *m_painter;
        QPen m_pen;
        QBrush m_brush;
    };
}

#endif /* __KIS_PAINTING_TWEAKS_H */
