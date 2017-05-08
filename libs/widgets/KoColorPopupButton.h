/* This file is part of the KDE project
 * Copyright (c) 2013 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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

#ifndef KOCOLORPOPUPBUTTON_H_
#define KOCOLORPOPUPBUTTON_H_

#include <QToolButton>

#include "kritawidgets_export.h"

/**
 * @short A widget for 
 *
 */
class KRITAWIDGETS_EXPORT KoColorPopupButton: public QToolButton
{
    Q_OBJECT

public:
    /**
     * Constructor for the widget, where value is set to 0
     *
     * @param parent parent QWidget
     */
    explicit KoColorPopupButton(QWidget *parent=0);

    /**
     * Destructor
     */
    ~KoColorPopupButton() override;

    QSize sizeHint() const override;

Q_SIGNALS:
    /// Emitted when a resource was selected
    void iconSizeChanged();

protected:
    void resizeEvent(QResizeEvent *) override; ///< reimplemented from QToolButton
};

#endif
