/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KOTOOLSELECTION_H
#define KOTOOLSELECTION_H

#include "kritaflake_export.h"
#include <QObject>

/**
 * Each tool can have a selection which is private to that tool and the specified shape
 * that it comes with.
 * This object is provided for applications to operate on that selection.  Copy paste
 * come to mind, but also marking the selected text bold.
 */
class KRITAFLAKE_EXPORT KoToolSelection : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param parent a parent for memory management purposes.
     */
    explicit KoToolSelection(QObject *parent = 0);
    ~KoToolSelection() override;

    /// return true if the tool currently has something selected that can be copied or deleted.
    virtual bool hasSelection() {
        return false;
    }
};

#endif
