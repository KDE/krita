/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef QMLGLOBALENGINE_H
#define QMLGLOBALENGINE_H

#include "krita_sketch_export.h"

class QQmlEngine;
class KRITA_SKETCH_EXPORT QmlGlobalEngine
{
public:
    QQmlEngine* engine() const;
    void setEngine(QQmlEngine* engine);

    static QmlGlobalEngine* instance();

private:
    QmlGlobalEngine();
    ~QmlGlobalEngine();

    QQmlEngine* m_engine;

    static QmlGlobalEngine* sm_instance;
};

#endif // QMLGLOBALENGINE_H
