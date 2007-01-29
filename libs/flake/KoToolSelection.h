/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KOTOOLSELECTION_H
#define KOTOOLSELECTION_H

#include <flake_export.h>
#include <QObject>

/**
 * Each tool can have a selection which is private to that tool and the specified shape
 * that it comes with.
 * This object is provided for applications to operate on that selection.  Copy paste
 * come to mind, but also marking the selected text bold.
 */
class FLAKE_EXPORT KoToolSelection : public QObject {
    Q_OBJECT
public:
    KoToolSelection(QObject *parent = 0);
    virtual ~KoToolSelection();

    /// return true if the tool currently has something selected that can be copied or deleted
    virtual bool hasSelection() { return false; }
};

#endif
