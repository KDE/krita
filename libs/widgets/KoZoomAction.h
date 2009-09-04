/* This file is part of the KDE libraries
    Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
    Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KOZOOMACTION_H
#define KOZOOMACTION_H

#include <kselectaction.h>
#include <KoZoomMode.h>

#include "kowidgets_export.h"

/**
 * Class KoZoomAction implements an action to provide zoom values.
 * In a toolbar, KoZoomAction will show a dropdown list (combobox), also with
 * the possibility for the user to enter arbritrary zoom value
 * (must be an integer). The values shown on the list are always
 * sorted.
 * In a statusbar it provides a scale (slider) plus an editable value plus
 * some buttons for special zoommodes
 */
class KOWIDGETS_EXPORT KoZoomAction : public KSelectAction
{
    Q_OBJECT
public:

  /**
   * Creates a new zoom action.
   * @param zoomModes which zoom modes that should be shown
   * @param text The text that will be displayed.
   * @param parent The action's parent object.
   */
    KoZoomAction( KoZoomMode::Modes zoomModes, const QString& text, QObject *parent);
    ~KoZoomAction();

    /**
     * Reimplemented from QWidgetAction.
     */
    virtual QWidget* createWidget(QWidget* parent);

    enum SpecialButton {
        AspectMode = 1, ///< changing aspect mode
        ZoomToSelection = 2, ///< zooms to current selection
        ZoomToAll = 4 ///< zooms to the whole content of the document
    };
    Q_DECLARE_FLAGS(SpecialButtons, SpecialButton)

    /**
     * Enables specified special buttons.
     * This should be called immediately after calling the constructor.
     * @param buttons mask of the special button flags to enable
     */
    void setSpecialButtons( SpecialButtons buttons );

    qreal effectiveZoom() const;

public slots:

  /**
   * Sets the zoom. If zoom not yet on the list of zoom values, it will be inserted
   * into the list at proper place so that the the values remain sorted.
   * emits zoomChanged
   */
    void setZoom( qreal zoom );

  /**
   * Change the zoom modes that should be shown
   */
    void setZoomModes( KoZoomMode::Modes zoomModes );

  /**
   * Change the zoom to a closer look than current
   * Zoom mode will be CONSTANT afterwards
   * emits zoomChanged
   */
    void zoomIn( );

  /**
   * Change the zoom to a wider look than current
   * Zoom mode will be CONSTANT afterwards
   * emits zoomChanged
   */
    void zoomOut( );

  /**
   * Set the actual zoom value used in the app. This is needed when using @ref zoomIn() , @ref zoomOut() and/or when
   * plugged into the viewbar.
   */
    void setEffectiveZoom(qreal zoom);

  /**
   * Change the selected zoom mode.
   */
    void setSelectedZoomMode( KoZoomMode::Mode mode );

  /**
   * Change status of "Use same aspect as pixels" button
   * (emits aspectModeChanged(bool) after the change, ALWAYS)
   */
    void setAspectMode(bool status);

protected slots:

    void triggered( const QString& text );
    void sliderValueChanged(int value);

signals:

  /**
   * Signal zoomChanged is triggered when user changes the zoom value, either by
   * choosing it from the list or by entering new value.
   * @param mode The selected zoom mode
   * @param zoom the zoom, only defined if @p mode is KoZoomMode::ZOOM_CONSTANT
   */
    void zoomChanged( KoZoomMode::Mode mode, qreal zoom );

  /**
   * Signal aspectModeChanged is triggered when the user toggles the widget.
   * Nothing else happens except that this signal is emitted.
   * @param status Whether the special aspect mode is on
   */
    void aspectModeChanged( bool status );

    /**
     * Signal is triggered when the user clicks the zoom to selection button.
     * Nothing else happens except that this signal is emitted.
     */
    void zoomedToSelection();

    /**
     * Signal is triggered when the user clicks the zoom to all button.
     * Nothing else happens except that this signal is emitted.
     */
    void zoomedToAll();

protected:

    /// Regenerates the action's items
    void regenerateItems( const qreal zoom, bool asCurrent = false );

private:
    Q_DISABLE_COPY( KoZoomAction )

    class Private;
    Private * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KoZoomAction::SpecialButtons)

#endif
