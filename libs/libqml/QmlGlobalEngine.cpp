/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "QmlGlobalEngine.h"

QmlGlobalEngine* QmlGlobalEngine::sm_instance = 0;

QQmlEngine* QmlGlobalEngine::engine() const
{
    return m_engine;
}

void QmlGlobalEngine::setEngine(QQmlEngine* engine)
{
    m_engine = engine;
}

QmlGlobalEngine* QmlGlobalEngine::instance()
{
    if(!sm_instance) {
        sm_instance = new QmlGlobalEngine;
    }
    return sm_instance;
}

QmlGlobalEngine::QmlGlobalEngine() : m_engine(0)
{

}

QmlGlobalEngine::~QmlGlobalEngine()
{

}
