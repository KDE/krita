/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TOOL_CROP_H_
#define TOOL_CROP_H_

#include <QObject>
#include <QVariant>

/**
 * A module that provides a crop tool.
 */
class ToolCrop : public QObject
{
    Q_OBJECT
public:
    ToolCrop(QObject *parent, const QVariantList &);
    ~ToolCrop() override;

};

#endif // TOOL_CROP_H_
