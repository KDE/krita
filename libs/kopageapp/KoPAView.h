/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOPAVIEW_H
#define KOPAVIEW_H

#include <QObject>

#include <KoView.h>
#include <KoZoomHandler.h>
#include <KoRuler.h>

class KoCanvasController;
class KoPACanvas;
class KoPADocument;
class KToggleAction;
class KoPAPage;
class KoShapeManager;
class KoZoomAction;

/// Creates a view with a KoPACanvas and rulers
class KOPAGEAPP_EXPORT KoPAView : public KoView
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param document the document of this view
     * @param parent the parent widget
     */
    explicit KoPAView( KoPADocument * document, QWidget * parent = 0 );
    virtual ~KoPAView();

    void updateReadWrite( bool readwrite );

    KoViewConverter * viewConverter() { return &m_zoomHandler; }

    KoZoomHandler* zoomHandler() { return &m_zoomHandler; }

    KoPACanvas * kogaCanvas() { return m_canvas; }
    KoPACanvas * kogaCanvas() const { return m_canvas; }

    /// @return Page that is shown in the canvas
    KoPAPage* activePage() const;

    /// Set page shown in the canvas to @p page
    void setActivePage( KoPAPage* page );

    /// @return the shape manager used for this view
    KoShapeManager* shapeManager() const;

    void setZoom(int zoom);

public slots:
    /// Shows/hides the rulers
    void setShowRulers(bool show);

protected:
    /// creates the widgets (called from the constructor)
    void initGUI();
    /// creates the actions (called from the constructor)
    void initActions();

protected slots:
    void viewSnapToGrid();
    void viewGrid();
    void viewZoom(KoZoomMode::Mode mode, int zoom);

    void textBold(bool bold);
    void textItalic(bool italic);
    void textUnderline(bool underline);
    void textStrikeOut(bool strikeout);
    void slotNonbreakingSpace();
    void slotNonbreakingHyphen();
    void slotSoftHyphen();
    void slotLineBreak();
    void formatFont();

    /// Called when the canvas controller is resized
    virtual void canvasControllerResized();

    /// Called when the mouse position changes on the canvas
    virtual void updateMousePosition(const QPoint& position);

    /// Called when the selection changed
    virtual void selectionChanged();

protected:
    KoPADocument *m_doc;
    KoPACanvas *m_canvas;
    KoPAPage *m_activePage;

private:
    void recalculateZoom();

    KoCanvasController * m_canvasController;
    KoZoomHandler m_zoomHandler;

    KToggleAction *m_actionViewSnapToGrid;
    KToggleAction *m_actionViewShowGrid;
    KToggleAction *m_actionFormatBold;
    KToggleAction *m_actionFormatItalic;
    KToggleAction *m_actionFormatUnderline;
    KToggleAction *m_actionFormatStrikeOut;

    KoRuler *m_horizontalRuler;
    KoRuler *m_verticalRuler;
    KToggleAction* m_viewRulers;

    KoZoomAction *m_viewZoomAction;

    KAction *m_actionFormatFont;
};

#endif /* KOPAVIEW_H */
