/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef GLYPHPALETTEDIALOG_H
#define GLYPHPALETTEDIALOG_H

#include <KoDialog.h>
#include <QtQuickWidgets/QQuickWidget>

#include <KoSvgTextProperties.h>
#include <KoFontGlyphModel.h>
#include <KoSvgTextShape.h>
#include <QStandardItemModel>

#include "GlyphPaletteProxyModel.h"

class GlyphPaletteDialog: public KoDialog
{
    Q_OBJECT
public:
    GlyphPaletteDialog(QWidget *parent = nullptr);

    void setGlyphModelFromProperties(KoSvgTextProperties properties, QString text = QString());

Q_SIGNALS:
    void signalInsertRichText(KoSvgTextShape *text, bool replace);
public Q_SLOTS:

    void slotInsertRichText(int charRow, int glyphRow = -1, bool replace = false);

    void slotChangeFilter(int filterRow);
private:
    QQuickWidget *m_quickWidget {0};
    KoSvgTextProperties m_lastUsedProperties;
    KoFontGlyphModel *m_model;
    GlyphPaletteProxyModel *m_charMapModel;
    QStandardItemModel *m_filters;
};

#endif // GLYPHPALETTEDIALOG_H
