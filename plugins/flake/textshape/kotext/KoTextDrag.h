/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOTEXTDRAG_H
#define KOTEXTDRAG_H

#include "kritatext_export.h"

class QMimeData;
class QString;
class QByteArray;
class KoTextOdfSaveHelper;

/**
 * Class for simplifying adding a odf text with tracked changes to the clip board
 *
 * For saving the odf a KoDragOdfSaveHelper class is used.
 * It implements the writing of the body of the document. The
 * setOdf takes care of saving styles and tracked changes and all the other
 * common stuff.
 */
class KRITATEXT_EXPORT KoTextDrag
{
public:
    KoTextDrag();
    ~KoTextDrag();

    /**
     * Set odf mime type
     *
     * This calls helper.writeBody();
     *
     * @param mimeType used for creating the odf document
     * @param helper helper for saving the body of the odf document
     */
    bool setOdf(const char * mimeType, KoTextOdfSaveHelper &helper);

    /**
     * Add additional mimeTypes
     */
    void setData(const QString & mimeType, const QByteArray & data);

    /**
     * Get the mime data
     *
     * This transfers the ownership of the mimeData to the caller
     */
    QMimeData *takeMimeData();

private:
    QMimeData *m_mimeData;
};

#endif /* KOTEXTDRAG_H */
