/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTEXTBRUSHMODEL_H
#define KISTEXTBRUSHMODEL_H

#include <QObject>
#include <QString>
#include <QGuiApplication>
#include <lager/cursor.hpp>
#include <KisBrushModel.h>

using namespace KisBrushModel;

class KisTextBrushModel : public QObject
{
    Q_OBJECT
public:
    KisTextBrushModel(lager::cursor<CommonData> commonData,
                      lager::cursor<TextBrushData> textBrushData)
        : m_commonData(commonData),
          m_textBrushData(textBrushData),
          LAGER_QT(spacing) {m_commonData[&CommonData::spacing]},
          LAGER_QT(text) {m_textBrushData[&TextBrushData::text]},
          LAGER_QT(font) {m_textBrushData[&TextBrushData::font]},
          LAGER_QT(usePipeMode) {m_textBrushData[&TextBrushData::usePipeMode]}
    {
    }

    ~KisTextBrushModel();

    // the state must be declared **before** any cursors or readers
    lager::cursor<CommonData> m_commonData;
    lager::cursor<TextBrushData> m_textBrushData;

    LAGER_QT_CURSOR(qreal, spacing);
    LAGER_QT_CURSOR(QString, text);
    LAGER_QT_CURSOR(QString, font);
    LAGER_QT_CURSOR(bool, usePipeMode);
};

#endif // KISTEXTBRUSHMODEL_H
