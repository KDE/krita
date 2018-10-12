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

#ifndef KODRAGODFSAVEHELPER_H
#define KODRAGODFSAVEHELPER_H

#include "kritaflake_export.h"

#include <QtGlobal>

class KoShapeSavingContext;
class KoXmlWriter;
class KoGenStyles;
class KoEmbeddedDocumentSaver;
class KoDragOdfSaveHelperPrivate;

class KRITAFLAKE_EXPORT KoDragOdfSaveHelper
{
public:
    KoDragOdfSaveHelper();
    virtual ~KoDragOdfSaveHelper();

    /**
     * Create and return the context used for saving
     *
     * If you need a special context for saving you can reimplement this function.
     * The default implementation return a KoShapeSavingContext.
     *
     * The returned context is valid as long as the KoDragOdfSaveHelper is existing
     */
    virtual KoShapeSavingContext *context(KoXmlWriter *bodyWriter, KoGenStyles &mainStyles, KoEmbeddedDocumentSaver &embeddedSaver);

    /**
     * This method is called for writing the body of odf document.
     *
     * You need to have created a context before calling this function
     */
    virtual bool writeBody() = 0;

protected:
    /// constructor
    KoDragOdfSaveHelper(KoDragOdfSaveHelperPrivate &);

    KoDragOdfSaveHelperPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(KoDragOdfSaveHelper)
};

#endif /* KODRAGODFSAVEHELPER_H */
