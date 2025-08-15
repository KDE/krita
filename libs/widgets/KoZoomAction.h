/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 Ariya Hidayat <ariya@kde.org>
    SPDX-FileCopyrightText: 2006 Peter Simonsson <peter.simonsson@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KOZOOMACTION_H
#define KOZOOMACTION_H

#include <kselectaction.h>
#include <KoZoomMode.h>

#include "kritawidgets_export.h"

class KoZoomState;

/**
 * Class KoZoomAction implements an action to provide zoom values.
 * In a toolbar, KoZoomAction will show a dropdown list (combobox), also with
 * the possibility for the user to enter arbitrary zoom value
 * (must be an integer). The values shown on the list are always
 * sorted.
 * In a statusbar it provides a scale (slider) plus an editable value plus
 * some buttons for special zoommodes
 */
class KRITAWIDGETS_EXPORT KoZoomAction : public KSelectAction
{
    Q_OBJECT
public:

  /**
   * Creates a new zoom action.
   * @param zoomModes which zoom modes that should be shown
   * @param text The text that will be displayed.
   * @param parent The action's parent object.
   */
    KoZoomAction(const QString& text, QObject *parent);
    ~KoZoomAction() override;

    /**
     * Reimplemented from QWidgetAction.
     */
    QWidget* createWidget(QWidget* parent) override;

public Q_SLOTS:

    void slotZoomStateChanged(const KoZoomState &zoomState);

    /**
     * Change status of canvas size mapping button
     * (emits canvasMappingModeChanged(bool) after the change, ALWAYS)
     */
    void setUsePrintResolutionMode(bool value);

protected Q_SLOTS:

    void slotTextZoomChanged(const QString &value);
    void slotZoomLevelChangedIndex(int index);
    void sliderValueChanged(int value);
    void slotUpdateGuiAfterZoom();

Q_SIGNALS:

  /**
   * Signal zoomChanged is triggered when user changes the zoom value, either by
   * choosing it from the list or by entering new value.
   * @param mode The selected zoom mode
   * @param zoom the zoom, only defined if @p mode is KoZoomMode::ZOOM_CONSTANT
   */
    void zoomChanged( KoZoomMode::Mode mode, qreal zoom );

    void sigUsePrintResolutionModeChanged(bool value);

    void sigInternalUpdateZoomLevelsComboState(const QStringList &values, int index, const QString &activeText);
    void sigInternalUpdateZoomLevelsSliderState(int size, int index);
    void sigInternalUpdateUsePrintResolutionMode(bool value);

protected:
    void regenerateItems();
    void syncSliderWithZoom();

private:
    Q_DISABLE_COPY( KoZoomAction )

    class Private;
    Private * const d;
};

#endif
