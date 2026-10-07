
/*
 *  SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QEvent>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPalette>
#include <QSet>

#include <KoColorSpaceRegistry.h>

#include <kis_preset_live_preview_view.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include "kis_paintop_settings.h"
#include <strokes/freehand_stroke.h>
#include <strokes/KisFreehandStrokeInfo.h>
#include "KisAsynchronousStrokeUpdateHelper.h"
#include <kis_brush.h>
#include <KisGlobalResourcesInterface.h>
#include "kis_transaction.h"
#include <KoCanvasResourceProvider.h>
#include <KoCompositeOpRegistry.h>
#include "KisBrushStrokePreviewCache.h"


static bool needsBlendingModeOverride(const QString &compositeOp)
{
    //Brush presets with Erase blending mode have blank stroke previews without this
    static const QSet<QString> invisibleModes = {COMPOSITE_ERASE};
    return invisibleModes.contains(compositeOp);
}


KisPresetLivePreviewView::KisPresetLivePreviewView(QWidget *parent)
    : QGraphicsView(parent),
      m_updateCompressor(DefaultUpdateDelayMs, KisSignalCompressor::FIRST_ACTIVE)
{
    connect(&m_updateCompressor, SIGNAL(timeout()), SLOT(updateStroke()));
}

KisPresetLivePreviewView::~KisPresetLivePreviewView()
{
    delete m_noPreviewText;
    delete m_brushPreviewScene;
}

void KisPresetLivePreviewView::setup(KoCanvasResourceProvider* resourceManager)
{
    m_resourceManager = resourceManager;

    // initializing to nullptr helps check later if they actually have something in them
    m_noPreviewText = nullptr;
    m_sceneImageItem = nullptr;

    setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );


    // layer image needs to be big enough to get an entire stroke of data
    m_canvasSize.setWidth(this->width());
    m_canvasSize.setHeight(this->height());

    m_canvasCenterPoint.setX(m_canvasSize.width()*0.5);
    m_canvasCenterPoint.setY(m_canvasSize.height()*0.5);

    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    m_image = new KisImage(0, m_canvasSize.width(), m_canvasSize.height(), m_colorSpace, "stroke sample image");


    m_layer = new KisPaintLayer(m_image, "livePreviewStrokeSample", OPACITY_OPAQUE_U8, m_colorSpace);

    // set scene for the view
    m_brushPreviewScene = new QGraphicsScene();
    setScene(m_brushPreviewScene);

}

void KisPresetLivePreviewView::setCurrentPreset(KisPaintOpPresetSP preset)
{
    m_currentPreset = preset;
}

void KisPresetLivePreviewView::requestUpdateStroke()
{
    m_updateCompressor.start();
}

void KisPresetLivePreviewView::requestUpdateStrokeImmediate()
{
    if (m_previewGenerationInProgress) {
        m_updateCompressor.start();
        return;
    }
    updateStroke();
}

void KisPresetLivePreviewView::setCachePreviewMode(bool enabled)
{
    m_cachePreviewMode = enabled;
}

KoCanvasResourceProvider* KisPresetLivePreviewView::resourceManager() const
{
    return m_resourceManager;
}

void KisPresetLivePreviewView::updateStroke()
{
    if (!m_currentPreset) {
        return;
    }

    if (m_previewGenerationInProgress) {
        m_updateCompressor.start();
        return;
    }

    m_previewPresetId = m_currentPreset->resourceId();

    // do not paint a stroke if we are any of these engines (they have some issue currently)
    if (KisBrushStrokePreviewCache::isNoPreviewEngine(m_currentPreset->paintOp().id())) {
        paintBackground(m_cachePreviewMode);
        slotPreviewGenerationCompleted();
        return;
    }

    paintBackground(m_cachePreviewMode);
    setupAndPaintStroke();
}

void KisPresetLivePreviewView::slotPreviewGenerationCompleted()
{
    m_previewGenerationInProgress = false;

    m_lastPreviewImage = m_layer->paintDevice()->convertToQImage(0, m_image->bounds());

    if (!m_cachePreviewMode) {
        // only add the object once...then just update the pixmap so we can move the preview around
        if (!m_sceneImageItem) {
            m_sceneImageItem = m_brushPreviewScene->addPixmap(QPixmap::fromImage(m_lastPreviewImage));
        } else {
            m_sceneImageItem->setPixmap(QPixmap::fromImage(m_lastPreviewImage));
        }
    }

    // Emit preview for caching.
    const int presetId = m_previewPresetId >= 0
        ? m_previewPresetId
        : (m_currentPreset ? m_currentPreset->resourceId() : -1);
    if (presetId >= 0 && !m_lastPreviewImage.isNull()) {
        Q_EMIT sigPreviewImageReady(presetId, m_lastPreviewImage);
    }

    m_previewPresetId = -1;
}

void KisPresetLivePreviewView::paintStripedBackground()
{
    const int grayStrips = 20;
    const float sectionPercent = 1.0f / grayStrips;
    const int imageWidth = m_layer->image()->width();
    const int imageHeight = m_layer->image()->height();

    for (int i = 0; i < grayStrips; i++) {
        KoColor fillColor(m_layer->paintDevice()->colorSpace());
        fillColor.fromQColor((i % 2) ? QColor(80, 80, 80) : QColor(140, 140, 140));

        const QRect fillRect(imageWidth * sectionPercent * i,
                             0,
                             imageWidth * (sectionPercent * i + sectionPercent),
                             imageHeight);

        KisTransaction t(m_layer->paintDevice());
        m_layer->paintDevice()->fill(fillRect, fillColor);
        t.end();
    }
}

void KisPresetLivePreviewView::paintBackground(bool cacheMode)
{
    // clean up "no preview" text object if it exists. we will add it later if we need it
    if (m_noPreviewText) {
        this->scene()->removeItem(m_noPreviewText);
        m_noPreviewText = nullptr;
    }

    const QString paintOpId = m_currentPreset->paintOp().id();

    if (cacheMode) {
        if (KisBrushStrokePreviewCache::isNoPreviewEngine(paintOpId)) {
            m_layer->paintDevice()->clear();
            m_paintColor = KoColor(QColor(Qt::white), m_colorSpace);
            return;
        }

        if (KisBrushStrokePreviewCache::needsStripedBackground(paintOpId)) {
            paintStripedBackground();
        } else {
            m_layer->paintDevice()->clear();
        }

        m_paintColor = KoColor(QColor(Qt::white), m_colorSpace);
        return;
    }

    const QColor backgroundColor = palette().color(QPalette::Window);
    const QColor textColor = palette().color(QPalette::Text);

    if (KisBrushStrokePreviewCache::needsStripedBackground(paintOpId)) {
        paintStripedBackground();
        m_paintColor = KoColor(textColor, m_colorSpace);
    }
    else if (KisBrushStrokePreviewCache::isNoPreviewEngine(paintOpId)) {
        KisTransaction t(m_layer->paintDevice());
        m_layer->paintDevice()->fill(m_image->bounds(), KoColor(backgroundColor, m_colorSpace));
        t.end();

        m_paintColor = KoColor(textColor, m_colorSpace);

        QFont font;
        font.setPixelSize(14);
        font.setBold(false);

        m_noPreviewText = this->scene()->addText(i18n("No Preview for this engine"),font);
        m_noPreviewText->setDefaultTextColor(textColor);
        m_noPreviewText->setPos(50, this->height()/4);

        return;
    }
    else {
        KisTransaction t(m_layer->paintDevice());
        m_layer->paintDevice()->fill(m_image->bounds(), KoColor(backgroundColor, m_colorSpace));
        t.end();

        m_paintColor = KoColor(textColor, m_colorSpace);
    }
}

class NotificationStroke : public QObject, public KisSimpleStrokeStrategy
{
  Q_OBJECT
public:
    NotificationStroke()
        : KisSimpleStrokeStrategy(QLatin1String("NotificationStroke"))
    {
        setClearsRedoOnStart(false);
        this->enableJob(JOB_INIT, true, KisStrokeJobData::BARRIER);
        this->enableJob(JOB_CANCEL, true, KisStrokeJobData::BARRIER);
    }

    void initStrokeCallback() override {
        Q_EMIT timeout();
    }

    void cancelStrokeCallback() override {
        Q_EMIT cancelled();
    }

Q_SIGNALS:
    void timeout();
    void cancelled();
};

void KisPresetLivePreviewView::setupAndPaintStroke()
{
    // limit the brush stroke size. larger brush strokes just don't look good and are CPU intensive
    // we are making a proxy preset and setting it to the painter...otherwise setting the brush size of the original preset
    // will fire off signals that make this run in an infinite loop
    qreal previewSize = qBound(3.0, m_currentPreset->settings()->paintOpSize(), 25.0 ); // constrain live preview brush size
    //Except for the sketchbrush where it determines the history.
    if (m_currentPreset->paintOp().id() == "sketchbrush" ||
            m_currentPreset->paintOp().id() == "spraybrush") {
        previewSize = qMax(3.0, m_currentPreset->settings()->paintOpSize());
    }


    KisPaintOpPresetSP proxy_preset = m_currentPreset->clone().dynamicCast<KisPaintOpPreset>();
    KisPaintOpSettingsSP settings = proxy_preset->settings();
    settings->setPaintOpSize(previewSize);

    const QString compositeOp = settings->paintOpCompositeOp();
    if (needsBlendingModeOverride(compositeOp)) {
        settings->setPaintOpCompositeOp(COMPOSITE_OVER);
    }

    int maxTextureSize = 200;
    int textureOffsetX = settings->getInt("Texture/Pattern/MaximumOffsetX")*2;
    int textureOffsetY = settings->getInt("Texture/Pattern/MaximumOffsetY")*2;
    double textureScale = settings->getDouble("Texture/Pattern/Scale");
    if ( textureOffsetX*textureScale> maxTextureSize || textureOffsetY*textureScale > maxTextureSize) {
        int maxSize = qMax(textureOffsetX, textureOffsetY);
        double result = qreal(maxTextureSize) / maxSize;
        settings->setProperty("Texture/Pattern/Scale", result);
    }
    if (proxy_preset->paintOp().id() == "spraybrush") {

        QDomElement element;
        QDomDocument d;
        QString brushDefinition = settings->getString("brush_definition");
        if (!brushDefinition.isEmpty()) {
            d.setContent(brushDefinition);
            element = d.firstChildElement("Brush");

            KisBrushSP brush = KisBrush::fromXML(element, KisGlobalResourcesInterface::instance());

            qreal width = brush->image().width();
            qreal scale = brush->scale();
            qreal diameterToBrushRatio = 1.0;
            qreal diameter = settings->getInt("Spray/diameter");
            //hack, 1000 being the maximum possible brushsize.
            if (brush->filename().endsWith(".svg", Qt::CaseInsensitive)) {
                diameterToBrushRatio = diameter/(1000.0*scale);
                scale = 25.0 / 1000.0;
            } else {
                if (width * scale > 25.0) {
                    diameterToBrushRatio = diameter / (width * scale);
                    scale = 25.0 / width;

                    if (!settings->getBool("SprayShape/proportional")) {
                        settings->setProperty("SprayShape/width", qRound(scale * settings->getInt("SprayShape/width")));
                        settings->setProperty("SprayShape/height", qRound(scale * settings->getInt("SprayShape/height")));
                    }
                }
            }
            settings->setProperty("Spray/diameter", int(25.0 * diameterToBrushRatio));

            brush->setScale(scale);
            d.clear();
            element = d.createElement("Brush");
            brush->toXML(d, element);
            d.appendChild(element);
            settings->setProperty("brush_definition", d.toString());
        }
    }

    proxy_preset->setSettings(settings);

    KisResourcesSnapshotSP resources =
            new KisResourcesSnapshot(m_image,
                                     m_layer, m_resourceManager,
                                     0, {},
                                     proxy_preset);
    resources->setOpacity(settings->paintOpOpacity());
    resources->setMirroring(false, false); // ignore mirroring in toolbar

    if (needsBlendingModeOverride(compositeOp)) {
        resources->setCompositeOpId(COMPOSITE_OVER);
    }

    resources->setFGColorOverride(m_paintColor);
    KisFreehandStrokeInfo *strokeInfo = new KisFreehandStrokeInfo();

    KisStrokeStrategy *stroke =
            new FreehandStrokeStrategy(resources, strokeInfo, kundo2_noi18n("temp_stroke"));

    KisStrokeId strokeId = m_image->startStroke(stroke);

    if (proxy_preset->paintOp().id() == "mypaintbrush") {

        m_curvePointPI1.setCurrentTime(123);
        m_curvePointPI2.setCurrentTime(1230);
    }

    // paint the stroke. The sketchbrush gets a different shape than the others to show how it works
    if (proxy_preset->paintOp().id() == "sketchbrush"
            || proxy_preset->paintOp().id() == "curvebrush"
            || proxy_preset->paintOp().id() == "particlebrush") {
        qreal startX = m_canvasCenterPoint.x() - (this->width()*0.4);
        qreal endX   = m_canvasCenterPoint.x() + (this->width()*0.4);
        qreal middle = m_canvasCenterPoint.y();
        KisPaintInformation pointOne;
        pointOne.setPressure(0.0);
        pointOne.setPos(QPointF(startX, middle));
        KisPaintInformation pointTwo;
        pointTwo.setPressure(0.0);
        pointTwo.setPos(QPointF(startX, middle));
        int repeats = 8;

        for (int i = 0; i < repeats; i++) {
            pointOne.setPos(pointTwo.pos());
            pointOne.setPressure(pointTwo.pressure());

            pointTwo.setPressure((1.0/repeats)*(i+1));
            qreal xPos = ((1.0/repeats) * (i+1) * (endX-startX) )+startX;
            pointTwo.setPos(QPointF(xPos, middle));

            qreal offset = (this->height()/(repeats*1.5))*(i+1);
            qreal handleY = middle + offset;
            if (i%2 == 0) {
                handleY = middle - offset;
            }

            m_image->addJob(strokeId,
                            new FreehandStrokeStrategy::Data(0,
                                                             pointOne,
                                                             QPointF(pointOne.pos().x(),
                                                                     handleY),
                                                             QPointF(pointTwo.pos().x(),
                                                                     handleY),
                                                             pointTwo));
            m_image->addJob(strokeId, new KisAsynchronousStrokeUpdateHelper::UpdateData(true));
        }

    } else {

        // paint an S curve
        m_curvePointPI1.setPos(QPointF(m_canvasCenterPoint.x() - (this->width()*0.45),
                                       m_canvasCenterPoint.y() + (this->height()*0.2)));
        m_curvePointPI1.setPressure(0.0);


        m_curvePointPI2.setPos(QPointF(m_canvasCenterPoint.x() + (this->width()*0.4),
                                       m_canvasCenterPoint.y() - (this->height()*0.2)   ));

        m_curvePointPI2.setPressure(1.0);

        m_image->addJob(strokeId,
                        new FreehandStrokeStrategy::Data(0,
                                                         m_curvePointPI1,
                                                         QPointF(m_canvasCenterPoint.x(),
                                                                 m_canvasCenterPoint.y()-this->height()),
                                                         QPointF(m_canvasCenterPoint.x(),
                                                                 m_canvasCenterPoint.y()+this->height()),
                                                         m_curvePointPI2));
        m_image->addJob(strokeId, new KisAsynchronousStrokeUpdateHelper::UpdateData(true));
    }
    m_image->endStroke(strokeId);

    m_previewGenerationInProgress = true;

    NotificationStroke *notificationStroke = new NotificationStroke();
    connect(notificationStroke, SIGNAL(timeout()), SLOT(slotPreviewGenerationCompleted()));
    KisStrokeId notificationId = m_image->startStroke(notificationStroke);
    m_image->endStroke(notificationId);


    // TODO: if we don't have any regressions because of it until 4.2.8, then
    //       just remove this code.
    // even though the brush is cloned, the proxy_preset still has some connection to the original preset which will mess brush sizing
    // we need to return brush size to normal.The normal brush sends out a lot of extra signals, so keeping the proxy for now
    //proxy_preset->settings()->setPaintOpSize(originalPresetSize);

}

void KisPresetLivePreviewView::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::PaletteChange && !m_cachePreviewMode) {
        if (m_currentPreset) {
            requestUpdateStroke();
        }
    }
}

#include "kis_preset_live_preview_view.moc"
