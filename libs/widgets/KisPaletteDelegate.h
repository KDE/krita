/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PALETTE_DELEGATE_H
#define __KIS_PALETTE_DELEGATE_H

#include <QAbstractItemDelegate>

#include "kritawidgets_export.h"


class KRITAWIDGETS_EXPORT KisPaletteDelegate : public QAbstractItemDelegate
{
private:
    static const int BORDER_WIDTH;
public:
    KisPaletteDelegate(QObject * parent = 0);
    ~KisPaletteDelegate() override;

    void setCrossedKeyword(const QString &value)
    {
        m_crossedKeyword = value;
    }

    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override;

private:
    QString m_crossedKeyword;
};

#endif /* __KIS_PALETTE_DELEGATE_H */
