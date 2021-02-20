/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KODOCKWIDGETTITLEBARBUTTON_H_
#define KODOCKWIDGETTITLEBARBUTTON_H_

#include "kritawidgets_export.h"
#include <QAbstractButton>

class QEvent;
class QPaintEvent;

/**
 * @short A custom title bar button for dock widgets.
 *
 * Used in KoDockWidgetTitleBar but can be also used for similar
 * purposes inside other parents.
 */
class KRITAWIDGETS_EXPORT KoDockWidgetTitleBarButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit KoDockWidgetTitleBarButton(QWidget *parent = 0);
    ~KoDockWidgetTitleBarButton() override;

    QSize sizeHint() const override; ///< reimplemented from QWidget
    QSize minimumSizeHint() const override; ///< reimplemented from QWidget

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    class Private;
    Private * const d;
};

#endif // KODOCKWIDGETTITLEBARBUTTON_H_
