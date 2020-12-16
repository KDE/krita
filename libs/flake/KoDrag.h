/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KODRAG_H
#define KODRAG_H

#include "kritaflake_export.h"

#include <QList>

class QMimeData;
class QString;
class QByteArray;
class KoDragPrivate;
class KoShape;

/**
 * Class for simplifying adding a odf to the clip board
 *
 * For saving the odf a KoDragOdfSaveHelper class is used.
 * It implements the writing of the body of the document. The
 * setOdf takes care of saving styles and all the other
 * common stuff.
 */
class KRITAFLAKE_EXPORT KoDrag
{
public:
    KoDrag();
    ~KoDrag();


    /**
     * Load SVG data into the current mime data
     */
    bool setSvg(const QList<KoShape*> shapes);

    /**
     * Add additional mimeTypes
     */
    void setData(const QString &mimeType, const QByteArray &data);

    /**
     * Add the mimeData to the clipboard
     */
    void addToClipboard();

    /**
     * Get the mime data
     *
     * This transfers the ownership of the mimeData to the caller
     *
     * This function is for use in automated tests
     */
    QMimeData *mimeData();

private:
    KoDragPrivate * const d;
};

#endif /* KODRAG_H */
