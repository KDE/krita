/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_EQUALIZER_WIDGET_H
#define __KIS_EQUALIZER_WIDGET_H

#include <QScopedPointer>
#include <QWidget>
#include <QMap>


#include "kritaanimationdocker_export.h"


class KRITAANIMATIONDOCKER_EXPORT KisEqualizerWidget : public QWidget
{
    Q_OBJECT

public:
    KisEqualizerWidget(int maxDistance, QWidget *parent);
    ~KisEqualizerWidget();

    struct EqualizerValues {
        int maxDistance;
        QMap<int, int> value;
        QMap<int, bool> state;
    };

    EqualizerValues getValues() const;
    void setValues(const EqualizerValues &values);

    void resizeEvent(QResizeEvent *event);

    void mouseMoveEvent(QMouseEvent *ev);

Q_SIGNALS:
    void sigConfigChanged();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_EQUALIZER_WIDGET_H */
