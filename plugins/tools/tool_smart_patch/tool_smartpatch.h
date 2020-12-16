/*
 *  SPDX-FileCopyrightText: 2017 Eugene Ingerman
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TOOL_SMARTPATCH_H_
#define TOOL_SMARTPATCH_H_

#include <QObject>
#include <QVariant>

class ToolSmartPatch : public QObject
{
    Q_OBJECT
public:
    ToolSmartPatch(QObject *parent, const QVariantList &);
    ~ToolSmartPatch() override;

};

#endif // TOOL_SMARTPATCH_H_
