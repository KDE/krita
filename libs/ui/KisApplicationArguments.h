/*
 * Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KISAPPLICATIONARGUMENTS_H
#define KISAPPLICATIONARGUMENTS_H

#include <QScopedPointer>

class QApplication;
class QByteArray;
class QStringList;
class KisDocument;

#include "kritaui_export.h"

class KRITAUI_EXPORT KisApplicationArguments
{
public:

    KisApplicationArguments(const QApplication &app);
    KisApplicationArguments(const KisApplicationArguments &rhs);
    ~KisApplicationArguments();

    void operator=(const KisApplicationArguments& rhs);
    QByteArray serialize();
    static KisApplicationArguments deserialize(QByteArray &serialized);

    QStringList filenames() const;

    int dpiX() const;
    int dpiY() const;
    bool doTemplate() const;
    bool exportAs() const;
    bool exportSequence() const;
    QString exportFileName() const;
    QString workspace() const;
    QString windowLayout() const;
    QString session() const;
    bool canvasOnly() const;
    bool noSplash() const;
    bool fullScreen() const;
    bool doNewImage() const;
    KisDocument *image() const;

private:

    KisApplicationArguments();

    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KISAPPLICATIONARGUMENTS_H
