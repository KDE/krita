/*
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TOOL_POLYLINE_H_
#define TOOL_POLYLINE_H_

#include <QObject>
#include <QVariant>

/**
 * A module that provides a polyline tool.
 */
class ToolPolyline : public QObject
{
    Q_OBJECT
public:
    ToolPolyline(QObject *parent, const QVariantList &);
    ~ToolPolyline() override;
};

#endif // TOOL_POLYLINE_H_
