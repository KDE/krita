/*  
    Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
    Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
    Copyright (C) 2006-2007 C. Boemann <cbo@boemann.dk>
    Copyright (C) 2014 Sven Langkamp <sven.langkamp@gmail.com>

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

#ifndef KOZOOMWIDGET_H
#define KOZOOMWIDGET_H

#include <QWidget>
#include "KoZoomAction.h"
#include <QScopedPointer>

class KoZoomWidget : public QWidget
{
    Q_OBJECT

public:
    KoZoomWidget(QWidget* parent, KoZoomAction::SpecialButtons specialButtons, int maxZoom);
    ~KoZoomWidget() override;

Q_SIGNALS:
   /**
    * Signal sliderValueChanged is triggered when the user moves the slider
    * @param value value of the slider
    */
    void sliderValueChanged(int value);

   /**
    * Signal zoomLevelChanged is triggered when the user changes the KoZoomInput combobox widget
    * @param value value of the slider
    */
    void zoomLevelChanged(const QString& level);

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

public Q_SLOTS:
    void setZoomLevels(const QStringList &values);
    void setCurrentZoomLevel(const QString &valueString);
    void setSliderValue(int value);

   /**
    * Change status of "Use same aspect as pixels" button
    */
    void setAspectMode(bool status);
private:
    class Private;
    QScopedPointer<Private> const d;
};

#endif // KOZOOMWIDGET_H
