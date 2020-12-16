/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
