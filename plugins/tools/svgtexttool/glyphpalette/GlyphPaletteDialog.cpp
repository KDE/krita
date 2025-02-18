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
    m_altPopup->setModel(m_charMapModel);
    connect(m_model, SIGNAL(modelReset()), m_charMapModel, SLOT(emitBlockLabelsChanged()));

    m_quickWidget->rootContext()->setContextProperty("glyphModel", QVariant::fromValue(m_model));
    m_quickWidget->rootContext()->setContextProperty("charMapProxyModel", QVariant::fromValue(m_charMapModel));

    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSource(QUrl("qrc:/GlyphPalette.qml"));
    if (!m_quickWidget->errors().empty()) {
        qWarning() << "Errors in " << windowTitle() << ":" << m_quickWidget->errors();
    }
    connect(m_altPopup, SIGNAL(sigInsertRichText(int,int,bool)), this, SLOT(slotInsertRichText(int,int,bool)));
}

GlyphPaletteDialog::~GlyphPaletteDialog()
{

    /// Prevent accessing destroyed objects in QML engine
    /// See:
    ///   * https://invent.kde.org/graphics/krita/-/commit/d8676f4e9cac1a8728e73fec3ff1df1763c713b7
    ///   * https://bugreports.qt.io/browse/QTBUG-81247
    m_quickWidget->setParent(nullptr);
    delete m_quickWidget;

}

void GlyphPaletteDialog::setGlyphModelFromProperties(const QPair<KoSvgTextProperties, KoSvgTextProperties> &properties, const QString &text)
{
    if (m_lastUsedProperties.property(KoSvgTextProperties::FontFamiliesId) == properties.first.property(KoSvgTextProperties::FontFamiliesId)
            && m_lastUsedProperties.property(KoSvgTextProperties::FontWeightId) == properties.first.property(KoSvgTextProperties::FontWeightId)
            && m_lastUsedProperties.property(KoSvgTextProperties::FontStyleId) == properties.first.property(KoSvgTextProperties::FontStyleId)
            && m_lastUsedProperties.property(KoSvgTextProperties::FontStretchId) == properties.first.property(KoSvgTextProperties::FontStretchId)) {
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
    const KoSvgText::CssFontStyleData style = properties.second.propertyOrDefault(KoSvgTextProperties::FontStyleId).value<KoSvgText::CssFontStyleData>();
    KoSvgText::AutoValue fontSizeAdjust = properties.second.propertyOrDefault(KoSvgTextProperties::FontSizeAdjustId).value<KoSvgText::AutoValue>();
    if (properties.second.hasProperty(KoSvgTextProperties::KraTextVersionId)) {
        fontSizeAdjust.isAuto = (properties.second.property(KoSvgTextProperties::KraTextVersionId).toInt() < 3);
    }
    QStringList families = properties.second.property(KoSvgTextProperties::FontFamiliesId).toStringList();
    qreal size = properties.second.propertyOrDefault(KoSvgTextProperties::FontSizeId).toReal();
    const int weight = properties.second.propertyOrDefault(KoSvgTextProperties::FontWeightId).toInt();
    const int width = properties.second.propertyOrDefault(KoSvgTextProperties::FontStretchId).toInt();
    const std::vector<FT_FaceSP> faces = KoFontRegistry::instance()->facesForCSSValues(
        properties.second.property(KoSvgTextProperties::FontFamiliesId).toStringList(),
        lengths,
        properties.second.fontAxisSettings(),
        text,
        static_cast<quint32>(res),
        static_cast<quint32>(res),
        size,
        fontSizeAdjust.isAuto ? 1.0 : fontSizeAdjust.customValue,
        width,
        weight,
        style.style,
        style.slantValue.isAuto? 14: style.slantValue.customValue);

    if (faces.empty()) return;
    m_model->setFace(faces.front());

    QModelIndex idx = m_model->indexForString(text);
    if (m_quickWidget->rootObject()) {
        m_quickWidget->rootObject()->setProperty("fontFamilies", QVariant::fromValue(families));
        m_quickWidget->rootObject()->setProperty("fontSize", QVariant::fromValue(size));
        m_quickWidget->rootObject()->setProperty("fontWeight", QVariant::fromValue(weight));
        m_quickWidget->rootObject()->setProperty("fontWidth", QVariant::fromValue(width));
        m_quickWidget->rootObject()->setProperty("fontStyle", QVariant::fromValue(style.style));
        if (idx.isValid()) {
            m_quickWidget->rootObject()->setProperty("currentIndex", QVariant::fromValue(idx.row()));
        }
    }
    if (m_altPopup) {
        m_altPopup->setMarkup(families, size, weight, width, style.style);
    }
    m_lastUsedProperties = properties.first;
}

void GlyphPaletteDialog::slotInsertRichText(const int charRow, const int glyphRow, const bool replace)
{
    if (m_quickWidget->rootObject()) {
        QModelIndex idx = replace? m_model->index(charRow, 0): m_charMapModel->index(charRow, 0);
        QString  text = idx.isValid()? m_model->data(idx, Qt::DisplayRole).toString(): QString();
        KoSvgTextProperties props = m_lastUsedProperties;
        QStringList otf;

        if (glyphRow > -1) {
            if (replace) {
                idx = m_model->index(glyphRow, 0, idx);
                text = m_model->data(idx, Qt::DisplayRole).toString();
                otf.append(m_model->data(idx, KoFontGlyphModel::OpenTypeFeatures).value<QStringList>());
            } else {
                idx = m_charMapModel->index(glyphRow, 0, idx);
                text = m_charMapModel->data(idx, Qt::DisplayRole).toString();
                otf.append(m_charMapModel->data(idx, KoFontGlyphModel::OpenTypeFeatures).value<QStringList>());
            }
        }

        if (!text.isEmpty()) {
            KoSvgTextShape *richText = new KoSvgTextShape();
            if (!otf.isEmpty()) {
                props.setProperty(KoSvgTextProperties::FontFeatureSettingsId, otf);
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
