/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRECTSGRID_H
#define KISRECTSGRID_H

#include <QRect>
#include <QVector>
#include "kritaglobal_export.h"


/**
 * @brief A utility class to maintain a sparse grid of loaded/unloaded rects
 *
 * KisRectsGrid manages the presence of the rectangular cells in the grid
 * covering some specific area. The main usecase of the class is to maintain
 * an overlay device over some other paint device.
 *
 * When you need to ensure that the overlay has some particular `rect` loaded,
 * you just call `grid->addRect(rect)` and get a list of rects that should
 * still be loaded into the overlay. The returned list may be empty if all the
 * grid cells intersecting `rect` has already been loaded (added to the grid).
 *
 * The size of the cell is defined at the construction stage and must be
 * power of 2.
 */
class KRITAGLOBAL_EXPORT KisRectsGrid
{
public:
    /**
     * Create a grid with cell size set to \p gridSize
     */
    KisRectsGrid(int gridSize = 64);


    /**
     * Grow rectangle \p rc until it becomes aligned to
     * the grid cell borders.
     */
    QRect alignRect(const QRect &rc) const;

    /**
     * Add an arbitrary (non-aligned) rect to the grid
     *
     * The grid will form a list of cells that intersect \p rc and are still
     * not loaded, mark them as loaded and return the list to the caller.
     *
     * \param rc the rect to be added, not necessary aligned to the grid
     * \return the list of cells that has actually been changed
     */
    QVector<QRect> addRect(const QRect &rc);

    /**
     * Remove an arbitrary (non-aligned) rect from the grid
     *
     * The grid will form a list of loaded cells that are fully contained in \p
     * rc, mark them as unloaded and return the list to the caller.
     *
     * TODO: please note that removing two neighbouring non-aligned rectangles
     * may still leave some cells marked as loaded. Perhaps we should change
     * the meaning of this function to remove "all intersecting rectangles"
     * instead of "all contained rectangles".
     *
     * \param rc the rect to be removed, not necessary aligned to the grid
     * \return the list of cells that has actually been changed
     */
    QVector<QRect> removeRect(const QRect &rc);

    /**
     * Add an aligned rect to the grid
     *
     * The grid will form a list of cells that intersect \p rc and are still
     * not loaded, mark them as loaded and return the list to the caller.
     *
     * \param rc the rect to be added, the rect must be aligned
     * \return the list of cells that has actually been changed
     */
    QVector<QRect> addAlignedRect(const QRect &rc);

    /**
     * Remove an aligned rect from the grid
     *
     * The grid will form a list of loaded cells that are fully contained in \p
     * rc, mark them as unloaded and return the list to the caller.
     *
     * \param rc the rect to be removed, not necessary aligned to the grid
     * \return the list of cells that has actually been changed
     */
    QVector<QRect> removeAlignedRect(const QRect &rc);

    /**
     * Return is \p rc is fully covered by the loaded cells of the grid
     */
    bool contains(const QRect &rc) const;

    /**
     * Return the bounding box of the loaded cells of the grid
     */
    QRect boundingRect() const;

private:
    void resize(const QRect &newMappedAreaSize);
    static QRect shrinkRectToAlignedGrid(const QRect &srcRect, int lod);

private:
    int m_gridSize;
    int m_logGridSize;
    QVector<quint8> m_mapping;
    QRect m_mappedAreaSize; // measured in col/row

};

#endif // KISRECTSGRID_H
