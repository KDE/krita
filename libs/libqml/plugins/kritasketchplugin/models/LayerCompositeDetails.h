/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef LAYERCOMPOSITEDETAILS_H
#define LAYERCOMPOSITEDETAILS_H

#include <QObject>


class LayerCompositeDetails : public QObject
{
    Q_OBJECT
public:
    explicit LayerCompositeDetails(QObject* parent = 0);
    virtual ~LayerCompositeDetails();
};

#endif // LAYERCCOMPOSITEDETAILS_H
