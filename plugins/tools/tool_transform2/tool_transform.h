/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TOOL_TRANSFORM_H_
#define TOOL_TRANSFORM_H_

#include <QObject>
#include <QVariant>

/**
 * A module that provides a transform tool.
 */
class ToolTransform : public QObject
{
    Q_OBJECT
public:
    ToolTransform(QObject *parent, const QVariantList &);
    ~ToolTransform() override;
};

#endif // TOOL_TRANSFORM_H_
