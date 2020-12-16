/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_CMB_CONTOUR_H
#define KIS_CMB_CONTOUR_H

#include <QComboBox>

class KisCmbContour : public QComboBox
{
    Q_OBJECT
public:
    explicit KisCmbContour(QWidget *parent = 0);

Q_SIGNALS:

public Q_SLOTS:

};

#endif // KIS_CMB_CONTOUR_H
