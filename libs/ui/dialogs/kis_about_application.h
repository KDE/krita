/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_ABOUT_APPLICATION_H
#define KIS_ABOUT_APPLICATION_H

#include <QDialog>
#include <QList>

#include <kaboutdata.h>

class KisAboutApplication : public QDialog
{
    Q_OBJECT
public:
    explicit KisAboutApplication(QWidget *parent = 0);


private:
    QWidget *createTranslatorsWidget(const QList<KAboutPerson> &translators,
                                     const QString &ocsProviderUrl);
};



#endif // KIS_ABOUT_APPLICATION_H
