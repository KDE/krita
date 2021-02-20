/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
