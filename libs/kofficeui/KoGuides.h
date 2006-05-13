// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2005 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2005 Casper Boemann Rasmussen <cbr@boemann.dk>

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

#ifndef KOGUIDES_H
#define KOGUIDES_H

#include <QEvent>
#include <QObject>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3ValueList>
#include <QKeyEvent>

#include <koffice_export.h>

class QPaintDevice;
class KoPoint;
class KoRect;
class KoView;
class KoZoomHandler;

class KOFFICEUI_EXPORT KoGuides : public QObject
{
    Q_OBJECT
public:        
    /**
     * @brief Constructor
     *
     * @param view The view in which the guides will be shown
     * @param zoomHandler The zoom handler of the view
     */
    KoGuides( KoView *view, KoZoomHandler *zoomHandler );

    /**
     * @brief Destructor
     */
    ~KoGuides();
        
    /**
     * @brief Paint the guides
     *
     * @param painter with which the guides are painted
     */
    void paintGuides( QPainter &painter );

    typedef int SnapStatus;
    static const SnapStatus SNAP_NONE, SNAP_HORIZ, SNAP_VERT, SNAP_BOTH;

    /**
     * @brief Handle mousePressEvent
     *
     * This checks if a mousePressEvent would affect a guide line.
     * If the mouse is pressed over a guide line it gets selected.
     * Guide lines which were select get unselect.
     * If also the Ctrl Key is pressed the selection of the guide 
     * gets toggled.
     * If no guide is under the position all guides get deselected.
     *
     * @param e QMouseEvent
     *
     * @return true if the event was handled
     * @return false otherwise
     *         The event counts a not handled when only guides where
     *         deselected.
     */
    bool mousePressEvent( QMouseEvent *e );

    /**
     * @brief Handle mouseMoveEvent
     *
     * If the mouse button is pressed and a guide was selected it moves the 
     * selected guides.
     * If the mouse is moved over a guide line the cursor gets updated.
     *
     * @param e QMouseEvent
     *
     * @return true if the event was handled (guide moved, cursor changed as
     *         guide lies below)    
     * @return false otherwise
     */
    bool mouseMoveEvent( QMouseEvent *e );
    
    /**
     *
     * @param e QMouseEvent
     *
     * @return true if the event was handled
     * @return false otherwise
     */
    bool mouseReleaseEvent( QMouseEvent *e );
    
    /**
     *
     * @param e QKeyEvent
     *
     * @return true if the event was handled
     * @return false otherwise
     */
    bool keyPressEvent( QKeyEvent *e );

    /**
     * @brief Set the guide lines.
     *
     * This removes all existing guides and set up new ones at the positions given.
     *
     * @param horizontalPos A list of the position of the horizontal guide lines.
     * @param verticalPos A list of the position of the vertical guide lines.
     */
    void setGuideLines( const Q3ValueList<double> &horizontalPos, const Q3ValueList<double> &verticalPos );

    /**
     * @brief Set the positions for snapping of auto guide lines
     *
     * This removes all existing auto guide lines and set up new ones at the positions given.
     *
     * @param horizontalPos A list of the position of the horizontal guide lines.
     * @param verticalPos A list of the position of the vertical guide lines.
     */
    void setAutoGuideLines( const Q3ValueList<double> &horizontalPos, const Q3ValueList<double> &verticalPos );

    /**
     * @brief Get the position of the guide lines
     *
     * This filles the passed lists with the positions of the guide lines. 
     * The lists will be emptied before any positions are added.
     *
     * @param horizontalPos A list of the position of the horizontal guide lines.
     * @param verticalPos A list of the position of the vertical guide lines.
     */
    void getGuideLines( Q3ValueList<double> &horizontalPos, Q3ValueList<double> &verticalPos ) const;

    /**
     * @brief Snap rect to guidelines
     *
     * This looks for a guide which is in reach for the guide as defined in snap.
     * This method has the abillity to combine more calls. The snapStatus and diff args are both input and
     * output. On first call you should set snapStatus to 0. The return value would then show in which
     * directions it has snapped. If you combine several KoGuides you can let these output arguments
     * be input for the next koGuide. That way you'll always catch the nearest guide.
     *
     * @param rect the rect which should be snapped
     * @param snap the distance within the guide should snap - but always snap if already snapped
     * @param snapStatus if horiz,vert or both directions are snapped (both in and out param). 
     * @param diff distance away from guide. Only valid if status is snapping (both in and out param)
     */
    void snapToGuideLines( KoRect &rect, int snap, SnapStatus &snapStatus, KoPoint &diff );

    /**
     * @brief Snap rect to guidelines
     *
     * This looks fo a guide which is in reach for the guide as defined in snap.
     *
     * @param pos the position which should be snapped
     * @param snap the distance wherein the guide should snap - but always snap if already snapped
     * @param snapStatus if horiz,vert or both directions are snapped (both in and out param)
     * @param diff distance away from guide. Only valid if status is snapping (both in and out param)
     */
    void snapToGuideLines( KoPoint &pos, int snap, SnapStatus &snapStatus, KoPoint &diff );

    /**
     * @brief repaint guides if any changed snapping status
     *
     * This issues a paint request if any guides have changed snapping status.
     *
     * @param snappedRect the rect after it has been snapped
     */
    void repaintSnapping( const KoRect &snappedRect );

    /**
     * @brief repaint guides if any changed snapping status
     *
     * This issues a paint request if any guides have changed snapping status.
     *
     * @param snappedPoint the point after it has been snapped
     */
    void repaintSnapping( const KoPoint &snappedPoint, SnapStatus snapStatus );

    /**
     * @brief repaint guides so none is snapped
     *
     * This issues a paint request if any guides have changed snapping status.
     * It also effectively un-snaps all since it doesn't take an argument 
     */
    void repaintAfterSnapping( );

    /**
     * @brief Find the closesed disance to the next guide within the given distance
     *
     * @param rect The rect which should be snapped
     * @param diff distance in which too look for the closesed guide. The parameter is updated
     * with the closesed distance to a guide if one is found (both in and out param)
     */
    void diffNextGuide( KoRect &rect, KoPoint &diff );

public slots:
    /**
     * @brief Move Guide
     *
     * This slot can be connected to void KoRuler::moveGuide( const QPoint &, bool, int );
     * It will add a new guide when you move from the ruler to the canvas. After that it
     * moves the guide.
     *
     * @param pos The pos of the mouse 
     * @param horizontal true if the guide is horizontal, false if vertical
     * @param rulerWidth The witdth of the ruler as the pos is seen from the ruler widget.
     */
    void moveGuide( const QPoint &pos, bool horizontal, int rulerWidth );

    /**
     * @brief Add Guide
     *
     * This slot can be connected to void KoRuler::addGuide( const QPoint &, bool, int );
     * It will finish the inserting of a guide from moveGuide().
     *
     * @param pos The pos of the mouse 
     * @param horizontal true if the guide is horizontal, false if vertical
     * @param rulerWidth The witdth of the ruler as the pos is seen from the ruler widget.
     */
    void addGuide( const QPoint &pos, bool horizontal, int rulerWidth );

signals:
    /**
     * @brief Signal that shows that the guide lines are changed
     *
     * This signal is emmited when the guide lines are changed ( moved / deleted )
     *
     * @param view The view in which the guide lines are changed.
     */
    void guideLinesChanged( KoView * view );

    /**
     * @brief This signal is emitted when guides start/stop moving.
     *
     * @param state true when starting moving guides, false when stopping.
     */
    void moveGuides( bool state );

    /**
     * @brief This signal is emitted when guides start/stop painting.
     *
     * With this signal it is possible to only repaint the guides in the paint 
     * method of the canvas. Just set/unset a flag when this signal is emmited.
     * This signal is emitted before and after a repaint is done.
     *
     * @param state true when starting painting guides, false when stopping.
     */
    void paintGuides( bool state );

private slots:
    /**
     * @brief Execute a dialog to set the position of the guide
     */
    void slotChangePosition();

    /**
     * @brief remove all selected guides
     */
    void slotRemove();

private:
    /// Strukt holding the data of a guide line
    struct KoGuideLine 
    {
        KoGuideLine( Qt::Orientation o, double pos, bool a = false )
        : orientation( o )
        , position( pos )
        , selected( false )
        , snapping( false )
        , automatic( a )
        {}
        Qt::Orientation orientation;
        double position;
        bool selected; // if this guide is selected
        bool snapping; // if this guide is being snapped to
        bool automatic; // if this is a atomatic guide line
    };

    /**
     * @brief Paint the canvas
     */ 
    void paint();

    /**
     * @brief Add a guide line with the orientation o at the position pos
     *
     * @param pos where to insert the guide
     * @param o orientation of the guide line
     */
    void add( Qt::Orientation o, QPoint &pos );
    
    /**
     * @brief Select a guide
     *
     * @param gd guide to select
     */
    void select( KoGuideLine *guideLine );

    /**
     * @brief Unselect a guide
     *
     * @param gd guide to unselect
     */
    void unselect( KoGuideLine *guideLine );
        
    /**
     * @brief Unselect all selected KoGuideLineData
     *
     * @return true, when selection was changed
     * @return false otherwise
     */
    bool unselectAll();

    /**
     * @brief remove all selected guides
     */
    void removeSelected();

    /**
     * @brief Check if at least one guide is selected
     *
     * @return true if at least on guide is seleted
     * @return false otherwise
     */
    bool hasSelected();

    /**
     * @brief Find a guide
     *
     * This function looks for a guide at x or y pos. The position can differ by
     * diff.
     *
     * @param x x position to look for a guide
     * @param y y position to look for a guide
     * @param diff how far next to a guide sould it also be found
     *
     * @return the fould guide
     * @return 0 if none is found
     */
    KoGuideLine * find( KoPoint &p, double diff );

    /**
     * @brief Move selected guides.
     *
     * This moves all selected guides around. If more than one guide is selected it makes
     * sure the guides are not moved of the canvas.
     *
     * @param pos position of the mouse
     */
    void moveSelectedBy( QPoint &p );

    /**
     * @brief Map pos from screen
     * 
     * @param pos on screen
     *
     * @return pos in document
     */
    KoPoint mapFromScreen( const QPoint & pos );

    /**
     * @brief Map pos to screen
     * 
     * @param pos in document
     *
     * @return pos on screen
     */
    QPoint mapToScreen( const KoPoint & pos );

    /**
     * @brief Check if the both values are nearly the same.
     *
     * @param a first value
     * @param a second value
     *
     * @return true if they are the same
     * @return false otherwise
     */ 
    bool virtuallyEqual( double a, double b ) { return QABS( a - b ) < 1E-4; }

    /// view
    KoView * m_view;
    /// zoom handler of the view
    KoZoomHandler * m_zoomHandler;

    enum GuideLineType
    {
        GL,
        GL_SELECTED, 
        GL_AUTOMATIC,
        GL_END
    };

    /// array of list of the different guide line types
    Q3ValueList<KoGuideLine *> m_guideLines[GL_END];
    
    /// used to save the last mouse position
    QPoint m_lastPoint;
    /// true if a guide is selected at the moment
    bool m_mouseSelected;
    /// true if a guide is inserted at the moment
    bool m_insertGuide;
    /// popup menu
    class Popup;
    Popup * m_popup;
};

#endif /* KOGUIDES_H */
