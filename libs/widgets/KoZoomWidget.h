/*  
    SPDX-FileCopyrightText: 2004 Ariya Hidayat <ariya@kde.org>
    SPDX-FileCopyrightText: 2006 Peter Simonsson <peter.simonsson@gmail.com>
    SPDX-FileCopyrightText: 2006-2007 C. Boemann <cbo@boemann.dk>
    SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KOZOOMWIDGET_H
#define KOZOOMWIDGET_H

#include <QWidget>
#include "KoZoomAction.h"
#include <QScopedPointer>
#include <kritawidgets_export.h>

class KRITAWIDGETS_EXPORT KoZoomWidget : public QWidget
{
    Q_OBJECT

public:
    KoZoomWidget(QWidget* parent, int maxZoom);
    ~KoZoomWidget() override;

    bool isZoomInputFlat() const;
    void setZoomInputFlat(bool flat);

Q_SIGNALS:
   /**
    * Signal sliderValueChanged is triggered when the user moves the slider
    * @param value value of the slider
    */
    void sliderValueChanged(int value);

   /**
    * Signal zoomLevelChanged is triggered when the user changes the KoZoomInput combobox widget
    * @param level value of the slider
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
