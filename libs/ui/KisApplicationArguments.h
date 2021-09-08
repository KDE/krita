/*
 * SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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

    bool doTemplate() const;
    bool exportAs() const;
    bool exportSequence() const;
    QString exportFileName() const;
    QString workspace() const;
    QString windowLayout() const;
    QString session() const;
    QString fileLayer() const;
    bool canvasOnly() const;
    bool noSplash() const;
    bool fullScreen() const;
    bool doNewImage() const;
    KisDocument *createDocumentFromArguments() const;

private:

    KisApplicationArguments();

    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KISAPPLICATIONARGUMENTS_H
