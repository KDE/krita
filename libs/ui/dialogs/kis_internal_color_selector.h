/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISINTERNALCOLORSELECTOR_H
#define KISINTERNALCOLORSELECTOR_H

#include "kritaui_export.h"
#include "kis_mainwindow_observer.h"
#include "kis_canvas2.h"
#include "KoColor.h"
#include "KoColorSpace.h"
#include <QScopedPointer>

#include "ui_wdgdlginternalcolorselector.h"

/**
 * @brief The KisInternalColorSelector class
 *
 * A non-modal color selector dialog that is not a plugin and can thus be used for filters.
 */
class KRITAUI_EXPORT KisInternalColorSelector : public QDialog, public KisMainwindowObserver
{
    Q_OBJECT
public:
    KisInternalColorSelector(QWidget* parent, const QString &caption);
    ~KisInternalColorSelector();
    /**
     * @brief setCanvas
     * reimplemented from the canvasobserver class.
     * @param canvas
     */
    virtual void setCanvas(KisCanvas2 *canvas);
    /**
     * @brief unsetCanvas
     * reimplemented from the canvas observer class.
     */
    virtual void unsetCanvas();

Q_SIGNALS:
    /**
     * @brief signalForegroundColorChosen
     * The most important signal. This will sent out when a color has been picked from the selector.
     * There will be a small delay to make sure that the selector causes too many updates.
     *
     * Do not connect this to slotColorUpdated.
     * @param color The new color chosen
     */

    void signalForegroundColorChosen(KoColor color);
public Q_SLOTS:
    /**
     * @brief slotColorUpdated
     * Very important slot. Is connected to krita's resources to make sure it has
     * the currently active color. It's very important that this function is able to understand
     * when the signal came from itself.
     * @param newColor This is the new color.
     */
    void slotColorUpdated(KoColor newColor);
private Q_SLOTS:

    /**
     * @brief slotLockSelector
     * This slot will prevent the color from being updated.
     */
    void slotLockSelector();

    /**
     * @brief slotColorSpaceChanged
     * Color space has changed.
     */
    void slotColorSpaceChanged(const KoColorSpace *cs);
    /**
     * @brief slotConfigurationChanged
     * Wrapper slot for changes to the colorspace.
     */
    void slotConfigurationChanged();

    void endUpdateWithNewColor();

private:
    Ui::WdgDlgInternalColorSelector *ui; //the UI
    struct Private; //The private struct
    const QScopedPointer<Private> m_d; //the private pointer

    /**
     * @brief updateAllElements
     * Updates each widget with the new element, and if it's responsible for the update sents
     * a signal out that there's a new color.
     */
    void updateAllElements();
};

#endif // KISINTERNALCOLORSELECTOR_H
