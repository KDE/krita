/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2023 Halla Rempt <halla@valdyas.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KISCLICKABLELABEL_H
#define KISCLICKABLELABEL_H

#include <QLabel>
#include <QObject>
#include <QWidget>

#include <kritawidgetutils_export.h>

class KRITAWIDGETUTILS_EXPORT KisClickableLabel : public QLabel
{
    Q_OBJECT
public:

    explicit KisClickableLabel(QWidget *parent = nullptr);
    ~KisClickableLabel();

Q_SIGNALS:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event);


};

#endif // KISCLICKABLELABEL_H
