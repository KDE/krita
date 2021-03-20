/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SKETCHINPUTCONTEXT_H
#define SKETCHINPUTCONTEXT_H

#include <QInputContext>

#include "krita_sketch_export.h"

class KRITA_SKETCH_EXPORT SketchInputContext : public QInputContext
{
public:
    explicit SketchInputContext(QObject* parent = 0);
    virtual ~SketchInputContext();

    virtual bool isComposing() const;
    virtual void reset();
    virtual QString language();
    virtual QString identifierName();
    virtual bool filterEvent(const QEvent* event);
};

#endif // SKETCHINPUTCONTEXT_H
