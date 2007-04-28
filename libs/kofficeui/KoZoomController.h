/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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
#ifndef KOZOOMCONTROLLER_H
#define KOZOOMCONTROLLER_H

#include <QObject>
#include <QSizeF>

class KoCanvasController;
class KoZoomAction;
class KoZoomHandler;
class KActionCollection;
class QAction;
class QSize;

#include <KoZoomMode.h>
#include <kofficeui_export.h>

/**
 * This controller class handles zoom levels for any canvas. (kofficeui)
 *
 * For each KoCanvasController you instantiate this class once. This class then creates
 * a KoZoomAction and basically handles all zooming for you.
 *
 * All you have to do is connect to zoomChanged so that you can redraw your canvas.
 * You don't have to concern yourself with fit to width modes or anything.
*
 * The specialAspectMode toggle is only a UI element. It does nothing except emit the
 * aspectModeChanged signal.
 *
 * As documentation follows a description of the internal workflow:
 * # the user changes the slider/combo which causes the zoomAction to fire.
 * This will cause the private slot setZoom(mode, double) to be called which will
 * then cause the controller to calculate the effective zoomlevel. In mode
 * constant the zoomlevel is forwarded, in mode pageWidth this is calculated based
 * on the page size and the canvas size (the latter can be fetched from the
 * canvasController)
 * 
 * # KoView or decendent calls setZoom(double) / setZoomMode(mode)
 * Mostly used just one time for the initialisation.
 * uses the same code as the previous case to determine the effective zoomlevel.
 * Will set the effective zoom on the CanvasController.
 * 
 * # The user uses the zoomTool to zoom to a specific rect.
 * The zoom tool acts on the CanvasController to adjust the zoom level.
 * The canvasController emits the a request to alter zoom (as a factor of current zoom)
 * which the zoomController then acts upon and emits for the application to persist.  It
 * will alter the the mode to percent based.
 *
*/
class KOFFICEUI_EXPORT KoZoomController : public QObject {
Q_OBJECT
public:
    /**
    * Constructor. Create oner per canvasController.  The zoomAction is created in the constructor as a member.
    * @param co the canvasController
    * @param zh the zoom handler (viewconverter with setter methods)
    * @param ac the action collection whereto the KoZoomAction is added
    * @param doSpecialAspectMode wether the KoZoomAction should show a toggle
    */
    KoZoomController(KoCanvasController *co, KoZoomHandler *zh, KActionCollection *ac, bool doSpecialAspectMode);

    /// destructor
    ~KoZoomController();

    /// returns the zoomAction to be added to the actionCollection and Gui.
    KoZoomAction *zoomAction() const;

    /// setter for the view to set the zoom level.
    void setZoom(double zoom);

    /// setter for the view to set the zoom mode.
    void setZoomMode(KoZoomMode::Mode mode);

public slots:
    /**
    * Every time the canvas changes content size, tell us.  Note that the size is in pt.
    * @param pageSize the new page size in points
    */
    void setPageSize(const QSizeF &pageSize);

    /**
    * Every time the document changes size, tell us.
    * For most applictaions the document size equals the page size.
    * Note that the size is in pt.
    * @param documentSize the new document size in points
    */
    void setDocumentSize( const QSizeF &documentSize );

    /**
     * Sets a fitting margin that is used when zooming to page size/width.
     * Note that the fit margin is given in pixels.
     * @param margin the new fit margin to use, the default is zero
     */
    void setFitMargin( int margin );

signals:
    /// the document can use the emitted data for persistency purposes.
    void zoomChanged (KoZoomMode::Mode mode, double zoom);

    /// emitted when the special aspect mode toggle changes.
    void aspectModeChanged (bool aspectModeActivated);

private slots:
    // should realy be on d pointer..
    /// slot for the zoomAction to connect to.
    void setZoom(KoZoomMode::Mode mode, double zoom);

    /// so we know when the canvasController changes size
    void setAvailableSize(const QSize &Size);

    /// when the canvas controller wants us to change zoom
    void requestZoomBy(const double factor);


// important note;
// it may look like we are duplicating setZoom like methods and they should be merged. This is not the case.
// the idea behind the different methods is that a different unit connects to them and we may act slightly
// different based on that.

private:
    Q_DISABLE_COPY( KoZoomController )

    class Private;
    Private * const d;
};

#endif
