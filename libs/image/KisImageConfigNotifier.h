/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISIMAGECONFIGNOTIFIER_H
#define KISIMAGECONFIGNOTIFIER_H

#include <QObject>
#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisImageConfigNotifier : public QObject
{
    Q_OBJECT
public:
    explicit KisImageConfigNotifier();
    ~KisImageConfigNotifier() override;

    static KisImageConfigNotifier* instance();

    /**
     * Notify that the configuration has changed. This will cause the
     * configChanged() signal to be emitted.
     */
    void notifyConfigChanged(void);

Q_SIGNALS:
    /**
     * This signal is emitted whenever notifyConfigChanged() is called.
     */
    void configChanged(void);

private:
    Q_DISABLE_COPY(KisImageConfigNotifier)

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISIMAGECONFIGNOTIFIER_H
