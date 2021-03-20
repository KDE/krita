/* This file is part of the KDE project
 * Author: Agata Cacko cacko.azh@gmail.com
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_WRAPPABLE_HBOX_LAYOUT_H
#define KIS_WRAPPABLE_HBOX_LAYOUT_H

#include <QVector>
#include <QLayout>
#include "kritawidgetutils_export.h"

// code taken partially from https://doc.qt.io/qt-5/qtwidgets-layouts-flowlayout-example.html
// and https://doc.qt.io/qt-5/layout.html
class KRITAWIDGETUTILS_EXPORT KisWrappableHBoxLayout : public QLayout
{
    Q_OBJECT

public:
    explicit KisWrappableHBoxLayout(QWidget* parent);
    ~KisWrappableHBoxLayout() override;

    void addItem(QLayoutItem *item) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    int count() const override;
    QLayoutItem *itemAt(int) const override;
    QLayoutItem *takeAt(int) override;
    void setGeometry(const QRect &rect) override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;

protected:
    int doLayout(const QRect &rect, bool testOnly) const;

private:
    QVector<QLayoutItem*> m_items;
    int m_lastWidth {-1};

};


#endif // KIS_WRAPPABLE_HBOX_LAYOUT_H
