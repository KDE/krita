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
    , m_model(new KoFontGlyphModel(this))
    , m_charMapModel(new GlyphPaletteProxyModel(this))
{
    setMinimumSize(500, 300);

    m_quickWidget = new QQuickWidget(this);
    this->setMainWidget(m_quickWidget);
    m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_quickWidget->engine()->rootContext()->setContextProperty("mainWindow", this);
    m_quickWidget->engine()->rootContext()->setContextObject(new KLocalizedContext(this));

    /*
    // Default to fusion style unless the user forces another style
        if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
             QQuickStyle::setStyle(QStringLiteral("Fusion"));
        }
    */
    m_quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    m_quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

    m_quickWidget->engine()->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    m_quickWidget->engine()->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

    m_quickWidget->setPalette(this->palette());
    this->setWindowTitle(i18nc("@title:window", "Glyph Palette"));

    m_charMapModel->setSourceModel(m_model);
    connect(m_model, SIGNAL(modelReset()), m_charMapModel, SLOT(emitBlockLabelsChanged()));

    m_quickWidget->rootContext()->setContextProperty("glyphModel", QVariant::fromValue(m_model));
    m_quickWidget->rootContext()->setContextProperty("charMapProxyModel", QVariant::fromValue(m_charMapModel));

    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSource(QUrl("qrc:/GlyphPalette.qml"));
    if (!m_quickWidget->errors().empty()) {
        qWarning() << "Errors in " << windowTitle() << ":" << m_quickWidget->errors();
    }
}

void GlyphPaletteDialog::setGlyphModelFromProperties(const QPair<KoSvgTextProperties, KoSvgTextProperties> &properties, const QString &text)
{
    if (m_lastUsedProperties.property(KoSvgTextProperties::FontFamiliesId) == properties.second.property(KoSvgTextProperties::FontFamiliesId)
            && m_lastUsedProperties.property(KoSvgTextProperties::FontWeightId) == properties.second.property(KoSvgTextProperties::FontWeightId)
            && m_lastUsedProperties.property(KoSvgTextProperties::FontStyleId) == properties.second.property(KoSvgTextProperties::FontStyleId)
            && m_lastUsedProperties.property(KoSvgTextProperties::FontStretchId) == properties.second.property(KoSvgTextProperties::FontStretchId)) {
        if (m_model && m_model->rowCount() > 0) {
            QModelIndex idx = m_model->indexForString(text);
            if (idx.isValid() && m_quickWidget->rootObject()) {
                m_quickWidget->rootObject()->setProperty("currentIndex", QVariant::fromValue(idx.row()));
                return;
            }
        }
    }
    const qreal res = 72.0;
    QVector<int> lengths;
    const KoSvgText::CssFontStyleData style = properties.second.propertyOrDefault(KoSvgTextProperties::FontStyleId).value<KoSvgText::CssFontStyleData>();
    KoSvgText::AutoValue fontSizeAdjust = properties.second.propertyOrDefault(KoSvgTextProperties::FontSizeAdjustId).value<KoSvgText::AutoValue>();
    if (properties.second.hasProperty(KoSvgTextProperties::KraTextVersionId)) {
        fontSizeAdjust.isAuto = (properties.second.property(KoSvgTextProperties::KraTextVersionId).toInt() < 3);
    }
    QStringList families = properties.second.property(KoSvgTextProperties::FontFamiliesId).toStringList();
    qreal size = properties.second.propertyOrDefault(KoSvgTextProperties::FontSizeId).toReal();
    int weight = properties.second.propertyOrDefault(KoSvgTextProperties::FontWeightId).toInt();
    const std::vector<FT_FaceSP> faces = KoFontRegistry::instance()->facesForCSSValues(
        properties.second.property(KoSvgTextProperties::FontFamiliesId).toStringList(),
        lengths,
        properties.second.fontAxisSettings(),
        text,
        static_cast<quint32>(res),
        static_cast<quint32>(res),
        size,
        fontSizeAdjust.isAuto ? 1.0 : fontSizeAdjust.customValue,
        properties.second.propertyOrDefault(KoSvgTextProperties::FontWeightId).toInt(),
        properties.second.propertyOrDefault(KoSvgTextProperties::FontStretchId).toInt(),
        style.style,
        style.slantValue.isAuto? 14: style.slantValue.customValue);

    if (faces.empty()) return;
    m_model->setFace(faces.front());

    QModelIndex idx = m_model->indexForString(text);
    if (m_quickWidget->rootObject()) {
        m_quickWidget->rootObject()->setProperty("fontFamilies", QVariant::fromValue(families));
        m_quickWidget->rootObject()->setProperty("fontSize", QVariant::fromValue(size));
        m_quickWidget->rootObject()->setProperty("fontWeight", QVariant::fromValue(weight));
        m_quickWidget->rootObject()->setProperty("fontStyle", QVariant::fromValue(style));
        if (idx.isValid()) {
            m_quickWidget->rootObject()->setProperty("currentIndex", QVariant::fromValue(idx.row()));
        }
    }
    m_lastUsedProperties = properties.first;
}

void GlyphPaletteDialog::slotInsertRichText(int charRow, int glyphRow, bool replace)
{
    if (m_quickWidget->rootObject()) {
        QModelIndex idx = replace? m_model->index(charRow, 0): m_charMapModel->index(charRow, 0);
        QString  text;
        KoSvgTextProperties props = m_lastUsedProperties;
        QStringList otf;
        if (replace) {
            idx = m_model->index(glyphRow, 0, idx);
            text = m_model->data(idx, Qt::DisplayRole).toString();
            otf.append(m_model->data(idx, KoFontGlyphModel::OpenTypeFeatures).value<QStringList>());
        } else {
            idx = m_charMapModel->index(glyphRow, 0, idx);
            text = m_charMapModel->data(idx, Qt::DisplayRole).toString();
            otf.append(m_charMapModel->data(idx, KoFontGlyphModel::OpenTypeFeatures).value<QStringList>());
        }
        KoSvgTextShape *richText = new KoSvgTextShape();
        if (!otf.isEmpty()) {
            props.setProperty(KoSvgTextProperties::FontFeatureSettingsId, otf);
        }

        richText->setPropertiesAtPos(-1, props);
        richText->insertText(0, text);
        emit signalInsertRichText(richText, replace);
    }

}
