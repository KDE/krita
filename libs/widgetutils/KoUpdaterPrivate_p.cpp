/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KoUpdaterPrivate_p.h"
#include <KoUpdater.h>

KoUpdaterPrivate::KoUpdaterPrivate(KoProgressUpdater *parent, int weight, const QString &name, bool isPersistent)
    : QObject(0)
    , m_progress(0)
    , m_weight(weight)
    , m_interrupted(false)
    , m_autoNestedName()
    , m_subTaskName(name)
    , m_hasValidRange(true)
    , m_isPersistent(isPersistent)
    , m_parent(parent)
    , m_connectedUpdater(new KoUpdater(this))
{
}

KoUpdaterPrivate::~KoUpdaterPrivate()
{
    setInterrupted(true);
    m_connectedUpdater->deleteLater();
}

QString KoUpdaterPrivate::autoNestedName() const
{
    return m_autoNestedName;
}

QString KoUpdaterPrivate::subTaskName() const
{
    return m_subTaskName;
}

QString KoUpdaterPrivate::mergedSubTaskName() const
{
   QString result = m_subTaskName;

   if (!m_autoNestedName.isEmpty()) {
       if (result.isEmpty()) {
           result = m_autoNestedName;
       } else {
           result = QString("%1: %2").arg(result).arg(m_autoNestedName);
       }
   }

   return result;
}

bool KoUpdaterPrivate::hasValidRange() const
{
    return m_hasValidRange;
}

bool KoUpdaterPrivate::isPersistent() const
{
    return m_isPersistent;
}

bool KoUpdaterPrivate::isCompleted() const
{
    return m_progress >= 100;
}

void KoUpdaterPrivate::cancel()
{
    m_parent->cancel();
}

void KoUpdaterPrivate::setInterrupted(bool value)
{
    m_interrupted = value;
    emit sigInterrupted(m_interrupted);
}

void KoUpdaterPrivate::setProgress(int percent)
{
    m_progress = percent;
    emit sigUpdated();
}

void KoUpdaterPrivate::setAutoNestedName(const QString &name)
{
    m_autoNestedName = name;
    emit sigUpdated();
}

void KoUpdaterPrivate::setHasValidRange(bool value)
{
    m_hasValidRange = value;
    emit sigUpdated();
}

QPointer<KoUpdater> KoUpdaterPrivate::connectedUpdater() const
{
    return m_connectedUpdater;
}
