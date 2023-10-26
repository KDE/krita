/*
 * SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTAGLABEL_H
#define KISTAGLABEL_H

#include <QWidget>
#include "kritaresourcewidgets_export.h"


class KRITARESOURCEWIDGETS_EXPORT KisTagLabel : public QWidget
{
    Q_OBJECT

public:
    explicit KisTagLabel(QString string, QWidget *parent = nullptr);
    ~KisTagLabel();

public:
    QString getText();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_string;
};

#endif // KISTAGLABEL_H
