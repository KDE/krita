/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef KOASPECTBUTTON_H
#define KOASPECTBUTTON_H

#include <QAbstractButton>

#include "kowidgets_export.h"

/**
 * This button gives a visual indication of weather the 'aspect ratio' is locked.
 * Typically you would use this alongside 2 spinboxes with a value like a width and height.
 */
class KOWIDGETS_EXPORT KoAspectButton : public QAbstractButton {
    Q_OBJECT
public:
    /// constructor
    KoAspectButton(QWidget *parent);
    virtual ~KoAspectButton();

    /// Returns of keeping aspect ratio is on or off
    bool keepAspectRatio() const;

public slots:
    /**
     * Set the visual indicator to be locked or not.
     * This also emits the keepAspectRatioChanged if the value has changed.
     * @param keep if true, lock the aspect ratio.
     */
    void setKeepAspectRatio(bool keep);

signals:
    /**
     * This signal is emitted every time the button changes value, either by user interaction or
     *  by programetically setting it.
     */
    void keepAspectRatioChanged(bool keep);

protected:
    /// reimplemented
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void paintEvent (QPaintEvent *);
    virtual QSize sizeHint () const;
    void keyReleaseEvent (QKeyEvent *e);

private:
    class Private;
    Private * const d;
};

#endif
