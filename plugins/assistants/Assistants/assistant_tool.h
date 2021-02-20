/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _RULERASSISTANTTOOL_H_
#define _RULERASSISTANTTOOL_H_

#include <QObject>
#include <QVariant>


class AssistantToolPlugin : public QObject
{
    Q_OBJECT
public:
    AssistantToolPlugin(QObject *parent, const QVariantList &);
    ~AssistantToolPlugin() override;

};

#endif
