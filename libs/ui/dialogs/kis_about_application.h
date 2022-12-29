/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_ABOUT_APPLICATION_H
#define KIS_ABOUT_APPLICATION_H

#include <QDialog>

class KisAboutApplication : public QDialog
{
    Q_OBJECT
public:
    KisAboutApplication(QWidget *parent = nullptr);
};



#endif // KIS_ABOUT_APPLICATION_H
