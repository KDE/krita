/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KOASPECTBUTTON_H
#define KOASPECTBUTTON_H

#include <QToolButton>

#include "kritawidgets_export.h"

/**
 * This button gives a visual indication of weather the 'aspect ratio' is locked.
 * Typically you would use this alongside 2 spinboxes with a value like a width and height.
 */
class KRITAWIDGETS_EXPORT KoAspectButton : public QToolButton {
    Q_OBJECT
public:
    /// constructor
    explicit KoAspectButton(QWidget *parent);
    ~KoAspectButton() override;

    /// Returns of keeping aspect ratio is on or off
    bool keepAspectRatio() const;

public Q_SLOTS:
    /**
     * Set the visual indicator to be locked or not.
     * This also emits the keepAspectRatioChanged if the value has changed.
     * @param keep if true, lock the aspect ratio.
     */
    void setKeepAspectRatio(bool keep);

    // connect the button release
    void buttonReleased();

Q_SIGNALS:
    /**
     * This signal is emitted every time the button changes value, either by user interaction or
     *  by programetically setting it.
     */
    void keepAspectRatioChanged(bool keep);

private:
    class Private;
    Private * const d;
};

#endif
