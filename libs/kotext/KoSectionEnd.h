/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>
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
#ifndef KOSECTIONEND_H
#define KOSECTIONEND_H

#include "kritatext_export.h"

#include <QMetaType>
#include <QList>
#include <QString>
#include <QScopedPointer>

class KoShapeSavingContext;
class KoSection;

class KoSectionEndPrivate;
/**
 * Marks the end of the section
 */
class KRITATEXT_EXPORT KoSectionEnd {
public:
    ~KoSectionEnd(); // this is needed for QScopedPointer

    void saveOdf(KoShapeSavingContext &context) const;

    QString name() const;
    KoSection *correspondingSection() const;

protected:
    const QScopedPointer<KoSectionEndPrivate> d_ptr;

private:
    Q_DISABLE_COPY(KoSectionEnd)
    Q_DECLARE_PRIVATE(KoSectionEnd)

    explicit KoSectionEnd(KoSection *section);

    friend class KoSectionModel;
    friend class TestKoTextEditor;
};

Q_DECLARE_METATYPE(KoSectionEnd *)
Q_DECLARE_METATYPE(QList<KoSectionEnd *>)

#endif // KOSECTIONEND_H
