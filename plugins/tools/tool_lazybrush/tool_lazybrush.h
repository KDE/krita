/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TOOL_LAZYBRUSH_H_
#define TOOL_LAZYBRUSH_H_

#include <QObject>
#include <QVariant>

class ToolLazyBrush : public QObject
{
    Q_OBJECT
public:
    ToolLazyBrush(QObject *parent, const QVariantList &);
    ~ToolLazyBrush() override;

};

#endif // TOOL_LAZYBRUSH_H_
