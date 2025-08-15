/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "GlyphPaletteDialog.h"

#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QHBoxLayout>

#include <KLocalizedString>

#include <KoResourcePaths.h>
#include <KoFontGlyphModel.h>
#include <KoFontRegistry.h>
#include <KoSvgText.h>

GlyphPaletteDialog::GlyphPaletteDialog(QWidget *parent)
    : KoDialog(parent)
    , m_altPopup(new GlyphPaletteAltPopup(this))
    , m_model(new KoFontGlyphModel(this))
    , m_charMapModel(new GlyphPaletteProxyModel(this))
{
    setMinimumSize(500, 300);

    m_quickWidget = new KisQQuickWidget(this);
    this->setMainWidget(m_quickWidget);
    m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_quickWidget->setPalette(this->palette());
    this->setWindowTitle(i18nc("@title:window", "Glyph Palette"));

    m_charMapModel->setSourceModel(m_model);
    m_altPopup->setModel(m_charMapModel);
    connect(m_model, SIGNAL(modelReset()), m_charMapModel, SLOT(emitBlockLabelsChanged()));

    m_quickWidget->rootContext()->setContextProperty("glyphModel", QVariant::fromValue(m_model));
    m_quickWidget->rootContext()->setContextProperty("charMapProxyModel", QVariant::fromValue(m_charMapModel));

    m_quickWidget->setSource(QUrl("qrc:/GlyphPalette.qml"));
    if (!m_quickWidget->errors().empty()) {
        qWarning() << "Errors in " << windowTitle() << ":" << m_quickWidget->errors();
    }
    connect(m_altPopup, SIGNAL(sigInsertRichText(int,int,bool,bool)), this, SLOT(slotInsertRichText(int,int,bool,bool)));
}

GlyphPaletteDialog::~GlyphPaletteDialog()
{
    delete m_quickWidget;

}

void GlyphPaletteDialog::setGlyphModelFromProperties(const QPair<KoSvgTextProperties, KoSvgTextProperties> &properties, const QString &text)
{
    if (m_lastUsedProperties.second.cssFontInfo() == properties.second.cssFontInfo()) {
        if (m_model && m_model->rowCount() > 0) {
            if (text.isEmpty()) return;
            QModelIndex idx = m_model->indexForString(text);
            if (idx.isValid() && m_quickWidget->rootObject()) {
                m_quickWidget->rootObject()->setProperty("currentIndex", QVariant::fromValue(idx.row()));
                return;
            }
        }
    }
    const qreal res = 72.0;
    QVector<int> lengths;
    const KoCSSFontInfo info = properties.second.cssFontInfo();
    const std::vector<FT_FaceSP> faces = KoFontRegistry::instance()->facesForCSSValues(
        lengths,
        info,
        text,
        static_cast<quint32>(res),
        static_cast<quint32>(res));

    QString language = properties.second.propertyOrDefault(KoSvgTextProperties::TextLanguage).toString();
    if (faces.empty()) return;
    m_model->setFace(faces.front(), QLatin1String(language.toLatin1()));

    QVariantMap map = properties.second.propertyOrDefault(KoSvgTextProperties::FontVariationSettingsId).toMap();

    QModelIndex idx = m_model->indexForString(text);
    if (m_quickWidget->rootObject()) {
        m_quickWidget->rootObject()->setProperty("fontFamilies", QVariant::fromValue(info.families));
        m_quickWidget->rootObject()->setProperty("fontSize", QVariant::fromValue(info.size));
        m_quickWidget->rootObject()->setProperty("fontWeight", QVariant::fromValue(info.weight));
        m_quickWidget->rootObject()->setProperty("fontWidth", QVariant::fromValue(info.width));
        m_quickWidget->rootObject()->setProperty("fontStyle", QVariant::fromValue(info.slantMode));
        m_quickWidget->rootObject()->setProperty("fontAxesValues", QVariant::fromValue(map));
        m_quickWidget->rootObject()->setProperty("language", QVariant::fromValue(language));
        if (idx.isValid()) {
            m_quickWidget->rootObject()->setProperty("currentIndex", QVariant::fromValue(idx.row()));
        }
    }
    if (m_altPopup) {
        m_altPopup->setMarkup(info.families, info.size, info.weight, info.width, info.slantMode, map, language);
    }
    m_lastUsedProperties = properties;
}

void GlyphPaletteDialog::slotInsertRichText(const int charRow, const int glyphRow, const bool replace, const bool useCharMap)
{
    if (m_quickWidget->rootObject()) {
        QAbstractItemModel *model = useCharMap? qobject_cast<QAbstractItemModel*>(m_charMapModel): qobject_cast<QAbstractItemModel*>(m_model);
        QModelIndex idx = model->index(charRow, 0);
        QString  text = idx.isValid()? model->data(idx, Qt::DisplayRole).toString(): QString();
        KoSvgTextProperties props = m_lastUsedProperties.first;
        QVariantMap otf;

        if (glyphRow > -1) {
            idx = model->index(glyphRow, 0, idx);
            text = model->data(idx, Qt::DisplayRole).toString();
            otf = model->data(idx, KoFontGlyphModel::OpenTypeFeatures).toMap();
        }

        if (!text.isEmpty()) {
            KoSvgTextShape *richText = new KoSvgTextShape();

            if (!otf.isEmpty()) {
                props.setProperty(KoSvgTextProperties::FontFeatureSettingsId, QVariant::fromValue(otf));
            }

            richText->setPropertiesAtPos(-1, props);
            richText->insertText(0, text);
            emit signalInsertRichText(richText, replace);
        }
        if (m_altPopup->isVisible()) {
            slotHidePopupPalette();
        }
    }

}

void GlyphPaletteDialog::slotShowPopupPalette(const int charRow, const int x, const int y, const int cellWidth, const int cellHeight)
{
    m_altPopup->setRootIndex(charRow);
    m_altPopup->setCellSize(cellWidth, cellHeight);
    m_altPopup->raise();
    m_altPopup->show();
    m_altPopup->move(this->mapToGlobal(QPoint(x, y)+m_quickWidget->pos()));
    m_altPopup->activateWindow();
}

void GlyphPaletteDialog::slotHidePopupPalette()
{
    m_altPopup->hide();
}
