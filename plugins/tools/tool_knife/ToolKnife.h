/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TOOL_KNIFE_H_
#define TOOL_KNIFE_H_

#include <QObject>
#include <QVariant>

class ToolKnife : public QObject
{
    Q_OBJECT
public:
    ToolKnife(QObject *parent, const QVariantList &);
    ~ToolKnife() override;

};

#endif // TOOL_KNIFE_H_
