/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOTITLEDTABWIDGET_H
#define KOTITLEDTABWIDGET_H

#include <QTabWidget>

#include "kritawidgets_export.h"

class QLabel;


class KRITAWIDGETS_EXPORT KoTitledTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    KoTitledTabWidget(QWidget *parent);

private Q_SLOTS:
    void slotUpdateTitle();

private:
    QLabel *m_titleLabel;
};

#endif // KOTITLEDTABWIDGET_H
