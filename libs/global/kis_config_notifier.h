/*
 *  Copyright (c) 2007 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef KIS_CONFIG_NOTIFIER_H_
#define KIS_CONFIG_NOTIFIER_H_

#include <QObject>
#include <QScopedPointer>

#include "kritaglobal_export.h"

/**
 * An object that emits a signal to inform interested parties that the
 * configuration settings have changed.
 */
class KRITAGLOBAL_EXPORT KisConfigNotifier : public QObject
{
    Q_OBJECT
public:
    KisConfigNotifier();
    ~KisConfigNotifier() override;

    /**
     * @return the KisConfigNotifier singleton
     */
    static KisConfigNotifier *instance();

    /**
     * Notify that the configuration has changed. This will cause the
     * configChanged() signal to be emitted.
     */
    void notifyConfigChanged(void);

    void notifyDropFramesModeChanged();
    void notifyPixelGridModeChanged();

Q_SIGNALS:
    /**
     * This signal is emitted whenever notifyConfigChanged() is called.
     */
    void configChanged(void);
    void dropFramesModeChanged();
    void pixelGridModeChanged();
private:
    KisConfigNotifier(const KisConfigNotifier&);
    KisConfigNotifier operator=(const KisConfigNotifier&);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_CONFIG_NOTIFIER_H_
