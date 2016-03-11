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

#ifndef __KIS_ASPECT_RATIO_LOCKER_H
#define __KIS_ASPECT_RATIO_LOCKER_H

#include <QScopedPointer>
#include <QObject>
#include "kritaui_export.h"

class QSpinBox;
class KoAspectButton;

class KRITAUI_EXPORT KisAspectRatioLocker : public QObject
{
    Q_OBJECT
public:
    KisAspectRatioLocker(QObject *parent);
    ~KisAspectRatioLocker();

    void connectSpinBoxes(QSpinBox *spinOne, QSpinBox *spinTwo, KoAspectButton *apectButton);


private Q_SLOTS:
    void slotSpinOneChanged();
    void slotSpinTwoChanged();
    void slotAspectButtonChanged();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_ASPECT_RATIO_LOCKER_H */
