/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2014 Denis Kuplyakov <dener.kup@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
#ifndef KOSECTIONEND_H
#define KOSECTIONEND_H

#include "kotext_export.h"

#include <QString>
#include <KoSection.h>

class KoXmlElement;
class KoShapeSavingContext;
class KoSection;

/**
 * Marks the end of the given section
 */
class KOTEXT_EXPORT KoSectionEnd {
public:
    KoSectionEnd(KoSection* section);
    void saveOdf(KoShapeSavingContext &context);

    QString name() const;
    KoSection *correspondingSection();

    ~KoSectionEnd();
private:
    class Private;
    Private *const d;

    friend class KoSection;
};

#endif // KOSECTIONEND_H
