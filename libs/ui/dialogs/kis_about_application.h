/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_ABOUT_APPLICATION_H
#define KIS_ABOUT_APPLICATION_H

#include <KoDialog.h>

class KisAboutApplication : public KoDialog
{
    Q_OBJECT
public:
    KisAboutApplication(QWidget *parent = nullptr);
};



#endif // KIS_ABOUT_APPLICATION_H
