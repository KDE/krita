/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISWARNINGWIDGET_H
#define KISWARNINGWIDGET_H

#include "kritaui_export.h"
#include <QWidget>

class QLabel;

class KRITAUI_EXPORT KisWarningWidget : public QWidget
{
    Q_OBJECT
public:
    KisWarningWidget(QWidget *parent);

    void setText(const QString &text);

    /**
     * The default warning message for a case when the user
     * tries to change color profile for a multilayered image
     */
    static QString changeImageProfileWarningText();

private:
    QLabel *m_warningIcon = 0;
    QLabel *m_warningText = 0;
};

#endif // KISWARNINGWIDGET_H
