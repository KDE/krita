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
    void zoomLevelChangedIndex(int index);

    void sigUsePrintResolutionModeChanged(bool value);

public Q_SLOTS:
    void setSliderState(int size, int index);
    void setZoomLevelsState(const QStringList &values, int index);
    void setCurrentZoomLevel(const QString &valueString);
    void setCurrentZoomLevel(int index);
    void setSliderValue(int value);

    void setUsePrintResolutionMode(bool value);
private:
    class Private;
    QScopedPointer<Private> const d;
};

#endif // KOZOOMWIDGET_H
