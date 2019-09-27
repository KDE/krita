/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>

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
#ifndef KIS_VIEW_H
#define KIS_VIEW_H

#include <QWidget>

#include <KoColorSpace.h>
#include <KoColorProfile.h>

#include <kis_types.h>
#include "kritaui_export.h"

#include "widgets/kis_floating_message.h"

class KisDocument;
class KisMainWindow;
class KisPrintJob;
class KisCanvasController;
class KisZoomManager;
class KisCanvas2;
class KisViewManager;
class KisDocument;
class KisCanvasResourceProvider;
class KisCoordinatesConverter;
class KisInputManager;

class KoZoomController;
class KoZoomController;
struct KoPageLayout;
class KoCanvasResourceProvider;

// KDE classes
class QAction;
class KActionCollection;
class KConfigGroup;

// Qt classes
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QPrintDialog;
class QCloseEvent;
class QStatusBar;
class QMdiSubWindow;

/**
 * This class is used to display a @ref KisDocument.
 *
 * Multiple views can be attached to one document at a time.
 */
class KRITAUI_EXPORT KisView : public QWidget
{
    Q_OBJECT

public:
    /**
     * Creates a new view for the document.
     */
    KisView(KisDocument *document, KisViewManager *viewManager, QWidget *parent = 0);
    ~KisView() override;

    // Temporary while teasing apart view and mainwindow
    void setViewManager(KisViewManager *view);
    KisViewManager *viewManager() const;

public:

    /**
     *  Retrieves the document object of this view.
     */
    KisDocument *document() const;

    /**
     * Deletes the view and creates a new one, displaying @p document,
     * in the same sub-window.
     *
     * @return the new view
     */
    KisView *replaceBy(KisDocument *document);

    /**
     * In order to print the document represented by this view a new print job should
     * be constructed that is capable of doing the printing.
     * The default implementation returns 0, which silently cancels printing.
     */
    KisPrintJob * createPrintJob();

    /**
     * Create a QPrintDialog based on the @p printJob
     */
    QPrintDialog *createPrintDialog(KisPrintJob *printJob, QWidget *parent);

    /**
     * @return the KisMainWindow in which this view is currently.
     */
    KisMainWindow *mainWindow() const;

    /**
     * Tells this view which subwindow it is part of.
     */
    void setSubWindow(QMdiSubWindow *subWindow);

    /**
     * @return the statusbar of the KisMainWindow in which this view is currently.
     */
    QStatusBar *statusBar() const;

    /**
     * This adds a widget to the statusbar for this view.
     * If you use this method instead of using statusBar() directly,
     * KisView will take care of removing the items when the view GUI is deactivated
     * and readding them when it is reactivated.
     * The parameters are the same as QStatusBar::addWidget().
     */
    void addStatusBarItem(QWidget * widget, int stretch = 0, bool permanent = false);

    /**
     * Remove a widget from the statusbar for this view.
     */
    void removeStatusBarItem(QWidget * widget);

    /**
     * Return the zoomController for this view.
     */
    KoZoomController *zoomController() const;

    /// create a list of actions that when activated will change the unit on the document.
    QList<QAction*> createChangeUnitActions(bool addPixelUnit = false);

    void closeView();

public:

    /**
     * The zoommanager handles everything action-related to zooming
     */
    KisZoomManager *zoomManager() const;

    /**
     * The CanvasController decorates the canvas with scrollbars
     * and knows where to start painting on the canvas widget, i.e.,
     * the document offset.
     */
    KisCanvasController *canvasController() const;
    KisCanvasResourceProvider *resourceProvider() const;

    /**
     * Filters events and sends them to canvas actions. Shared
     * among all the views/canvases
     *
     * NOTE: May be null while initialization!
     */
    KisInputManager* globalInputManager() const;

    /**
     * @return the canvas object
     */
    KisCanvas2 *canvasBase() const;

    /// @return the image this view is displaying
    KisImageWSP image() const;


    KisCoordinatesConverter *viewConverter() const;

    void resetImageSizeAndScroll(bool changeCentering,
                                 const QPointF &oldImageStillPoint = QPointF(),
                                 const QPointF &newImageStillPoint = QPointF());

    void setCurrentNode(KisNodeSP node);
    KisNodeSP currentNode() const;
    KisLayerSP currentLayer() const;
    KisMaskSP currentMask() const;
    /**
     * @brief softProofing
     * @return whether or not we're softproofing in this view.
     */
    bool softProofing();
    /**
     * @brief gamutCheck
     * @return whether or not we're using gamut warnings in this view.
     */
    bool gamutCheck();

    /// Convenience method to get at the active selection (the
    /// selection of the current layer, or, if that does not exist,
    /// the global selection.
    KisSelectionSP selection();

    void notifyCurrentStateChanged(bool isCurrent);
    bool isCurrent() const;

    void setShowFloatingMessage(bool show);
    void showFloatingMessage(const QString &message, const QIcon& icon, int timeout = 4500,
                             KisFloatingMessage::Priority priority = KisFloatingMessage::Medium,
                             int alignment = Qt::AlignCenter | Qt::TextWordWrap);

    bool canvasIsMirrored() const;

    void syncLastActiveNodeToDocument();

    void saveViewState(KisPropertiesConfiguration &config) const;
    void restoreViewState(const KisPropertiesConfiguration &config);

public Q_SLOTS:

    /**
     * Display a message in the status bar (calls QStatusBar::message())
     * @todo rename to something more generic
     * @param value determines autosaving
     */
    void slotSavingStatusMessage(const QString &text, int timeout, bool isAutoSaving = false);

    /**
     * End of the message in the status bar (calls QStatusBar::clear())
     * @todo rename to something more generic
     */
    void slotClearStatusText();
    /**
     * @brief slotSoftProofing set whether or not we're softproofing in this view.
     * Will be setting the same in the canvas belonging to the view.
     */
    void slotSoftProofing(bool softProofing);
    /**
     * @brief slotGamutCheck set whether or not we're gamutchecking in this view.
     * Will be setting the same in the vans belonging to the view.
     */
    void slotGamutCheck(bool gamutCheck);

    bool queryClose();

    void slotScreenChanged();

    void slotThemeChanged(QPalette pal);

private Q_SLOTS:
    void slotImageNodeAdded(KisNodeSP node);
    void slotContinueAddNode(KisNodeSP newActiveNode);

    void slotImageNodeRemoved(KisNodeSP node);
    void slotContinueRemoveNode(KisNodeSP newActiveNode);

Q_SIGNALS:
    // From KisImage
    void sigSizeChanged(const QPointF &oldStillPoint, const QPointF &newStillPoint);
    void sigProfileChanged(const KoColorProfile *  profile);
    void sigColorSpaceChanged(const KoColorSpace*  cs);
    void titleModified(QString,bool);

    void sigContinueAddNode(KisNodeSP newActiveNode);
    void sigContinueRemoveNode(KisNodeSP newActiveNode);

protected:

    // QWidget overrides
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

    /**
     * Generate a name for this view.
     */
    QString newObjectName();

public Q_SLOTS:
    void slotLoadingFinished();
    void slotSavingFinished();
    void slotImageResolutionChanged();
    void slotImageSizeChanged(const QPointF &oldStillPoint, const QPointF &newStillPoint);


private:

    class Private;
    Private * const d;

    static bool s_firstView;
};

#endif
