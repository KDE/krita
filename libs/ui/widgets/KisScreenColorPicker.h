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
#ifndef KISSCREENCOLORPICKER_H
#define KISSCREENCOLORPICKER_H

#include <QScopedPointer>
#include <QEvent>
#include <QMouseEvent>

#include "KoColor.h"
#include <KisScreenColorPickerBase.h>

#include "kritaui_export.h"

/**
 * @brief The KisScreenColorPicker class
 * Based on the original QColorDialog's screen color picker, this class provides a button
 * that can be used to activate a colorpicker that can pick from anywhere on the screen.
 */
class KRITAUI_EXPORT KisScreenColorPicker : public KisScreenColorPickerBase
{
    Q_OBJECT
public:
    explicit KisScreenColorPicker(bool showInfoLabel = false, QWidget *parent = 0);
    ~KisScreenColorPicker() override;

    KoColor currentColor();

    bool handleColorPickingMouseMove(QMouseEvent *e);
    bool handleColorPickingMouseButtonRelease(QMouseEvent *e);
    bool handleColorPickingKeyPress(QKeyEvent *e);

    /// reloads icon(s) when theme is updated
    void updateIcons() override;

    static KisScreenColorPicker *createScreenColorPicker(QWidget *parent = 0) {return new KisScreenColorPicker(parent);}

Q_SIGNALS:
    void sigNewColorPicked(KoColor c);

public Q_SLOTS:
    void pickScreenColor();

private Q_SLOTS:
    void updateColorPicking();
protected:
    void changeEvent(QEvent *event) override;
private:
    struct Private; //The private struct
    const QScopedPointer<Private> m_d; //the private pointer

    void setCurrentColor(KoColor c);
    KoColor grabScreenColor(const QPoint &p);
    void updateColorLabelText(const QPoint &globalPos);
    void releaseColorPicking();
    void continueUpdateColorPicking(const QPoint &globalPos);
};

class KisScreenColorPickingEventFilter : public QObject {
public:
    explicit KisScreenColorPickingEventFilter(KisScreenColorPicker *w, QObject *parent = 0);

    bool eventFilter(QObject *, QEvent *event) override;
private:
    KisScreenColorPicker *m_w;
};

#endif // KISSCREENCOLORPICKER_H
