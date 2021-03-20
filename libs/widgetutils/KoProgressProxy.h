/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KO_PROGRESS_PROXY
#define KO_PROGRESS_PROXY

#include "kritawidgetutils_export.h"

class QString;

/**
 * A proxy interface for a real progress status reporting thing, either
 * a widget such as a KoProgressProxy childclass that also inherits this
 * interface, or something that prints progress to stdout.
 */
class KRITAWIDGETUTILS_EXPORT KoProgressProxy
{

public:

    virtual ~KoProgressProxy() { }

    virtual int maximum() const = 0;
    virtual void setValue(int value) = 0;
    virtual void setRange(int minimum, int maximum) = 0;
    virtual void setFormat(const QString &format) = 0;
    virtual void setAutoNestedName(const QString &name);
};


#endif
