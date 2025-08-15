/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QHBoxLayout>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <KoResourcePaths.h>
#include <klocalizedstring.h>
#include "GlyphPaletteAltPopup.h"

GlyphPaletteAltPopup::GlyphPaletteAltPopup(QWidget *parent)
    : QFrame(parent)
{
    setWindowFlags(Qt::Popup);
    setFrameStyle(QFrame::Box | QFrame::Plain);
    m_quickWidget = new KisQQuickWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(this);

    m_quickWidget->setFixedSize(300, 400);

    layout->addWidget(m_quickWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);

    m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_quickWidget->setSource(QUrl("qrc:/GlyphPaletteAlts.qml"));
    if (!m_quickWidget->errors().empty()) {
        qWarning() << "Errors in " << windowTitle() << ":" << m_quickWidget->errors();
    } else if (m_quickWidget->rootObject()){
        m_quickWidget->rootObject()->setProperty("columns", QVariant::fromValue(3));
    }
    m_quickWidget->setPalette(this->palette());
}

GlyphPaletteAltPopup::~GlyphPaletteAltPopup()
{
    /// Prevent accessing destroyed objects in QML engine
    /// See:
    ///   * https://invent.kde.org/graphics/krita/-/commit/d8676f4e9cac1a8728e73fec3ff1df1763c713b7
    ///   * https://bugreports.qt.io/browse/QTBUG-81247
    m_quickWidget->setParent(nullptr);
    delete m_quickWidget;
}

void GlyphPaletteAltPopup::setRootIndex(const int index)
{
    if (m_charMapModel) {
        QModelIndex idx = m_charMapModel->index(index, 0);
        if (idx.isValid()) {
            m_quickWidget->rootObject()->setProperty("parentIndex", QVariant::fromValue(index));
        }
    }
}

void GlyphPaletteAltPopup::setCellSize(const int width, const int height)
{
    m_quickWidget->setFixedSize(width*3, height*4);
}

void GlyphPaletteAltPopup::setModel(QAbstractItemModel *model)
{
    m_charMapModel = model;
    if (m_quickWidget->rootObject() && m_charMapModel) {
        m_quickWidget->rootObject()->setProperty("glyphModel", QVariant::fromValue(m_charMapModel));
        m_quickWidget->rootObject()->setProperty("useCharMap", QVariant::fromValue(true));
    }
}

void GlyphPaletteAltPopup::setMarkup(const QStringList &families, const int size, const int weight, const int width, const QFont::Style style, const QVariantMap &axes, const QString &language)
{
    if (m_quickWidget->rootObject()) {
        m_quickWidget->rootObject()->setProperty("fontFamilies", QVariant::fromValue(families));
        m_quickWidget->rootObject()->setProperty("fontSize", QVariant::fromValue(size));
        m_quickWidget->rootObject()->setProperty("fontWeight", QVariant::fromValue(weight));
        m_quickWidget->rootObject()->setProperty("fontWidth", QVariant::fromValue(width));
        m_quickWidget->rootObject()->setProperty("fontStyle", QVariant::fromValue(style));
        m_quickWidget->rootObject()->setProperty("fontAxesValues",  QVariant::fromValue(axes));
        m_quickWidget->rootObject()->setProperty("language", QVariant::fromValue(language));
    }
}

void GlyphPaletteAltPopup::slotInsertRichText(const int charRow, const int glyphRow, const bool replace, const bool useCharMap)
{
    emit sigInsertRichText(charRow, glyphRow, replace, useCharMap);
}
