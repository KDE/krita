/*
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TOOL_DYNA_H_
#define TOOL_DYNA_H_

#include <QObject>
#include <QVariant>

/**
 * A module that provides a polygon tool.
 */
class ToolDyna : public QObject
{
    Q_OBJECT

public:

    ToolDyna(QObject *parent, const QVariantList &);
    ~ToolDyna() override;

};

#endif // TOOL_DYNA_H_
