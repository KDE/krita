/*
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TOOL_POLYGON_H_
#define TOOL_POLYGON_H_

#include <QObject>
#include <QVariant>

/**
 * A module that provides a polygon tool.
 */
class ToolPolygon : public QObject
{
    Q_OBJECT

public:

    ToolPolygon(QObject *parent, const QVariantList &);
    ~ToolPolygon() override;

};

#endif // TOOL_POLYGON_H_
