/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
   Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef koRuler_h
#define koRuler_h


#include "komain_export.h"
#include <KoUnit.h>

#include <QWidget>
#include <QTextOption>

class QPaintEvent;

class KoViewConverter;
class KoCanvasBase;
class KoRulerPrivate;

/**
 * Decorator widget to draw a single ruler around a canvas.
 */
class KOMAIN_EXPORT KoRuler : public QWidget
{
Q_OBJECT
public:
    /**
     * Creates a ruler with the orientation @p orientation
     * @param parent parent widget
     * @param orientation the orientation of the ruler
     * @param viewConverter the view converter used to convert from point to pixel
     */
    KoRuler(QWidget* parent, Qt::Orientation orientation, const KoViewConverter* viewConverter);
    ~KoRuler();

    /// For paragraphs each tab definition is represented by this struct.
    struct Tab {
        qreal position;    ///< distance in point from the start of the text-shape
        QTextOption::TabType type;       ///< Determine which type is used.
    };

    /// The ruler's unit
    KoUnit unit() const;

    /// The length of the ruler in points (pt)
    qreal rulerLength() const;

    /// The orientation of the ruler
    Qt::Orientation orientation() const;

    /// The start indent of the first line
    qreal firstLineIndent() const;

    /// The start indent of the rest of the lines
    qreal paragraphIndent() const;

    /// The end indent of all lines
    qreal endIndent() const;

    /// The tab chooser widget, which you must put into a layout along with the ruler.
    /// Returns 0 for vertical rulers,
    QWidget *tabChooser();

    /**
     * set a list of actions that will be shown in a popup should the user right click on this ruler.
     * @param popupActionList the list of actions
     * @see popupActionList()
     */
    void setPopupActionList(const QList<QAction*> &popupActionList);
    /**
     * Return the actions list.
     * @see setPopupActionList()
     */
    QList<QAction*> popupActionList() const;

public slots:
    /// Set the unit of the ruler
    void setUnit(const KoUnit &unit);

    /** Set the offset. Use this function to sync the ruler with
      * the canvas' position on screen
      * @param offset The offset in pixels
      */
    void setOffset(int offset);

    /// Sets the length of the ruler to @p length in points (pt)
    void setRulerLength(qreal length);

    /** Set the active range, ie the part of the ruler that is most likely used.
      * set to 0, 0 when there is no longer any active range
      * @param start the start of the range in pt
      * @param end the end of the range in pt
      */
    void setActiveRange(qreal start, qreal end);

    /** Set the override active range, ie the part of the ruler that is most likely used.
      * set to 0, 0 when there is no longer any active range
      * The override, means that if set it takes precedence over the normal active range.
      * @param start the start of the range in pt
      * @param end the end of the range in pt
      */
    void setOverrideActiveRange(qreal start, qreal end);

    /** Set the state of the ruler so that it shows everything in right to left mode.
      * @param isRightToLeft state of right to left mode. Default is false.
      */
    void setRightToLeft(bool isRightToLeft);

    /** Set if the ruler should show indents as used in textditors.
      * Set the indents with setFirstLineIndent(), setParagraphIndent(), setEndIndent() .
      * @param show show indents if true. Default is false.
      */
    void setShowIndents(bool show);

    /** Set the position of the first line start indent relative to the active range.
      * If Right To left is set the indent is relative to the right side of the active range .
      * @param indent the value relative to the active range.
      */
    void setFirstLineIndent(qreal indent);

    /** Set the position of the rest of the lines start indent relative to the active range.
      * If Right To left is set the indent is relative to the right side of the active range .
      * @param indent the value relative to the active range.
      */
    void setParagraphIndent(qreal indent);

    /** Set the position of the end indent relative to the active range.
      * If Right To left is set the indent is relative to the left side of the active range .
      * @param indent the value relative to the active range.
      */
    void setEndIndent(qreal indent);

    /** Set whether the ruler should show the current mouse position.
      * Update the position with updateMouseCoordinate().
      * @param show show mouse position if true. Default is false.
      */
    void setShowMousePosition(bool show);

    /** Update the current position of the mouse pointer, repainting if changed.
      * The ruler offset will be applied before painting.
      * @param coordinate Either the x or y coordinate of the mouse depending
      *                   of the orientation of the ruler.
      */
    void updateMouseCoordinate(int coordinate);

    /**
     * Set whether the ruler should show the selection borders
     * @param show show selection borders if true, default is false.
     */
    void setShowSelectionBorders(bool show);

    /**
     * Update the selection borders
     * @param first the first selection border in points
     * @param second the other selection border in points
     */
    void updateSelectionBorders(qreal first, qreal second);

    /**
     * Set whether the ruler should show tabs
     * @param show show selection borders if true, default is false.
     */
    void setShowTabs(bool show);

    /**
     * Update the tabs
     * @param tabs a list of tabs that is shown on the ruler the first selection border in points
     */
    void updateTabs(const QList<Tab> &tabs);

    /***
     * Return the list of tabs set on this ruler.
     */
    QList<Tab> tabs() const;

    /**
     * Clear all previously set hotspots.
     * A hotspot is a position on the ruler that the user can manipulate by dragging.
     */
    void clearHotSpots();

    /**
     * Add or set a hotspot.
     * A hotspot is a position on the ruler that the user can manipulate by dragging.
     * @param position the new position of the hotspot.
     * @param id the unique id for the hotspot. If the id has not been set before, it will be added.
     */
    void setHotSpot(qreal position, int id = -1);

    /**
     * Remove a previously set hotspot, returning true if one is actually returned.
     * @param id the unique id for the hotspot.
     * A hotspot is a position on the ruler that the user can manipulate by dragging.
     */
    bool removeHotSpot(int id);

    /**
     * Connect the ruler to a guides tool
     * This allows the user to drag a guide out of the ruler and get in one smooth operation
     * the guide tool to draw and position the guide line.
     * @param canvas the canvas that has had the KoToolManager create the tool for previously.
     */
    void createGuideToolConnection(KoCanvasBase *canvas);

signals:
    /**
     * emitted when any of the indents is moved by the user.
     * @param final false until the user releases the mouse. So you can implement live update.
     */
    void indentsChanged(bool final);

    /**
     * Emitted when any of the tabs are moved, deleted or inserted by the user.
     * @param originalTabIndex the index in the list of tabs before the user interaction
     *          started, or -1 if this is a new tab
     * @param tab the new tab, or zero when the tab has been removed.
     */
    void tabChanged(int originalTabIndex, KoRuler::Tab *tab);

    /// emitted when there the user is about to change a tab or hotspot
    void aboutToChange();

    void hotSpotChanged(int id, qreal newPosition);

    /// emitted when the mouse is drag+released outside the ruler
    void guideLineCreated(Qt::Orientation orientation, qreal viewPosition);

protected:
    /// reimplemented
    virtual void paintEvent(QPaintEvent* event);
    /// reimplemented
    virtual void mousePressEvent(QMouseEvent *ev);
    /// reimplemented
    virtual void mouseReleaseEvent(QMouseEvent *ev);
    /// reimplemented
    virtual void mouseMoveEvent(QMouseEvent *ev);
    /// reimplemented
    virtual QSize minimumSizeHint() const;
    /// reimplemented
    virtual QSize sizeHint() const;

private:
    KoRulerPrivate * const d;
    friend class KoRulerPrivate;
};

#endif
