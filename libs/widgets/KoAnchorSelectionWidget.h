/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOANCHORSELECTIONWIDGET_H
#define KOANCHORSELECTIONWIDGET_H

#include <QWidget>
#include <KoFlake.h>

#include "kritawidgets_export.h"


class KRITAWIDGETS_EXPORT KoAnchorSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KoAnchorSelectionWidget(QWidget *parent = 0);
    ~KoAnchorSelectionWidget() override;

    KoFlake::AnchorPosition value() const;
    QPointF value(const QRectF rect, bool *valid) const;

    void setValue(KoFlake::AnchorPosition value);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void valueChanged(KoFlake::AnchorPosition id);

public Q_SLOTS:
    void slotGroupClicked(int id);
private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KOANCHORSELECTIONWIDGET_H
