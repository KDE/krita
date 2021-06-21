/*
 *    SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com>
 *    SPDX-FileCopyrightText: 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISTAGFILTERWIDGET_H
#define KISTAGFILTERWIDGET_H

#include <QWidget>
#include <KisTagModel.h>

class KisTagFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KisTagFilterWidget(KisTagModel* model, QWidget* parent);
    ~KisTagFilterWidget() override;
    void clear();

    bool isFilterByTagChecked();

Q_SIGNALS:
    void filterTextChanged(const QString &filterText);
    void filterByTagChanged(const bool filterByTag);
private Q_SLOTS:
    void onTextChanged(const QString &lineEditText);
    void slotFilterByTagChanged(int filterByTag);
private:
    class Private;
    Private* const d;
};

#endif // KOTAGFILTERWIDGET_H
