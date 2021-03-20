/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSHAREDRUNNABLE_H
#define KISSHAREDRUNNABLE_H

#include <kritaglobal_export.h>

#include <QRunnable>

class KisSharedThreadPoolAdapter;

class KRITAGLOBAL_EXPORT KisSharedRunnable : public QRunnable
{
public:
    virtual void runShared() = 0;
    void run() override final;

private:
    friend class KisSharedThreadPoolAdapter;
    void setSharedThreadPoolAdapter(KisSharedThreadPoolAdapter *adapter);

private:
    KisSharedThreadPoolAdapter *m_adapter = 0;
};

#endif // KISSHAREDRUNNABLE_H
