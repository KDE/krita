/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KISCLONEDOCUMENTSTROKE_H
#define KISCLONEDOCUMENTSTROKE_H

#include "kritaimage_export.h"
#include <QScopedPointer>
#include "kis_simple_stroke_strategy.h"

class KisDocument;

class KisCloneDocumentStroke : public QObject, public KisSimpleStrokeStrategy
{
    Q_OBJECT
public:
    KisCloneDocumentStroke(KisDocument *document);
    ~KisCloneDocumentStroke();

    void initStrokeCallback() override;
    void finishStrokeCallback() override;

Q_SIGNALS:
    void sigDocumentCloned(KisDocument *image);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISCLONEDOCUMENTSTROKE_H
