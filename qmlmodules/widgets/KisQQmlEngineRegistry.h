/*
 * SPDX-FileCopyrightText: 2026 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISQQMLENGINEREGISTRY_H
#define KISQQMLENGINEREGISTRY_H

#include <QObject>
#include <QQmlEngine>

/**
 * @brief The KisQQmlEngineRegistry class
 * A registry to reuse the QQmlEngine where possible.
 */

class KisQQmlEngineRegistry: public QObject
{
    Q_OBJECT
public:
    KisQQmlEngineRegistry(QObject *parent = nullptr);
    ~KisQQmlEngineRegistry();

    static KisQQmlEngineRegistry *instance();

    /**
     * @brief qQuickWidgetEngine
     * @return engine that should be used for qquickwidgets.
     */
    QQmlEngine *qQuickWidgetEngine();
private:
    struct Private;
    QScopedPointer<Private> d;

    Q_DISABLE_COPY(KisQQmlEngineRegistry);
};

#endif // KISQQMLENGINEREGISTRY_H
