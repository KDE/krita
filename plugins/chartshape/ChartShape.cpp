/* This file is part of the KDE project

   Copyright 2007 Stefan Nikolaus     <stefan.nikolaus@kdemail.net>
   Copyright 2007-2010 Inge Wallin    <inge@lysator.liu.se>
   Copyright 2007-2008 Johannes Simon <johannes.simon@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


// Own
#include "ChartShape.h"

// Posix
#include <float.h> // For basic data types characteristics.

// Qt
#include <QPointF>
#include <QPainter>
#include <QSizeF>
#include <QTextDocument>
#include <QStandardItemModel>

// KDE
#include <KDebug>
#include <KApplication>
#include <KMessageBox>
#include <KMimeType>
#include <KUrl>

// KDChart
#include <KDChartChart>
#include <KDChartAbstractDiagram>
#include <KDChartCartesianAxis>
#include <KDChartCartesianCoordinatePlane>
#include <KDChartPolarCoordinatePlane>
#include "KDChartConvertions.h"
// Attribute Classes
#include <KDChartDataValueAttributes>
#include <KDChartGridAttributes>
#include <KDChartTextAttributes>
#include <KDChartMarkerAttributes>
#include <KDChartThreeDPieAttributes>
#include <KDChartThreeDBarAttributes>
#include <KDChartThreeDLineAttributes>
// Diagram Classes
#include <KDChartBarDiagram>
#include <KDChartPieDiagram>
#include <KDChartLineDiagram>
#include <KDChartRingDiagram>
#include <KDChartPolarDiagram>

// Calligra
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoStore.h>
#include <KoDocument.h>
#include <KoShapeSavingContext.h>
#include <KoViewConverter.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoGenStyles.h>
#include <KoShapeRegistry.h>
#include <KoToolRegistry.h>
#include <KoTextShapeData.h>
#include <KoTextDocumentLayout.h>
#include <KoOdfReadStore.h>
#include <KoDocumentEntry.h>
#include <KoOdfStylesReader.h>
#include <KoCanvasBase.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoShapeBackground.h>
#include <KoInsets.h>
#include <KoShapeStrokeModel.h>
#include <KoColorBackground.h>
#include <KoShapeStroke.h>
#include <KoOdfWorkaround.h>

// KChart
#include "Axis.h"
#include "DataSet.h"
#include "Legend.h"
#include "PlotArea.h"
#include "Surface.h"
#include "ChartProxyModel.h"
#include "TextLabelDummy.h"
#include "ChartDocument.h"
#include "ChartTableModel.h"
#include "ChartLayout.h"
#include "TableSource.h"
#include "OdfLoadingHelper.h"
#include "SingleModelHelper.h"


// Define the protocol used here for embedded documents' URL
// This used to "store" but KUrl didn't like it,
// so let's simply make it "tar" !
#define STORE_PROTOCOL "tar"
#define INTERNAL_PROTOCOL "intern"

namespace KChart {

/// @see ChartShape::setEnableUserInteraction()
static bool ENABLE_USER_INTERACTION = true;

static const char *ODF_CHARTTYPES[NUM_CHARTTYPES] = {
    "chart:bar",
    "chart:line",
    "chart:area",
    "chart:circle",
    "chart:ring",
    "chart:scatter",
    "chart:radar",
    "chart:filled-radar",
    "chart:stock",
    "chart:bubble",
    "chart:surface",
    "chart:gantt"
};

static const ChartSubtype defaultSubtypes[NUM_CHARTTYPES] = {
    NormalChartSubtype,         // Bar
    NormalChartSubtype,         // Line
    NormalChartSubtype,         // Area
    NoChartSubtype,             // Circle
    NoChartSubtype,             // Ring
    NoChartSubtype,             // Scatter
    NormalChartSubtype,         // Radar
    NormalChartSubtype,         // Filled Radar
    HighLowCloseChartSubtype,   // Stock
    NoChartSubtype,             // Bubble
    NoChartSubtype,             // Surface
    NoChartSubtype              // Gantt
};


void saveOdfFont(KoGenStyle &style, const QFont& font, const QColor& color)
{
    style.addProperty("fo:font-family", font.family(), KoGenStyle::TextType);
    style.addPropertyPt("fo:font-size", font.pointSize(), KoGenStyle::TextType);
    style.addProperty("fo:color", color.isValid() ? color.name() : "#000000", KoGenStyle::TextType);
    int w = font.weight();
    style.addProperty("fo:font-weight", w == 50 ? "normal" : w == 75 ? "bold" : QString::number(qRound(w / 10) * 100), KoGenStyle::TextType);
    style.addProperty("fo:font-style", font.italic() ? "italic" : "normal", KoGenStyle::TextType);
}

QString saveOdfFont(KoGenStyles& mainStyles,
                    const QFont& font,
                    const QColor& color)
{
    KoGenStyle autoStyle(KoGenStyle::ParagraphAutoStyle, "chart", 0);
    saveOdfFont(autoStyle, font, color);
    return mainStyles.insert(autoStyle, "ch");
}


void saveOdfLabel(KoShape *label, KoXmlWriter &bodyWriter,
                  KoGenStyles &mainStyles, LabelType labelType)
{
    // Don't save hidden labels, as that's the way of removing them
    // from a chart.
    if (!label->isVisible())
        return;

    TextLabelData *labelData = qobject_cast<TextLabelData*>(label->userData());
    if (!labelData)
        return;

    if (labelType == FooterLabelType)
        bodyWriter.startElement("chart:footer");
    else if (labelType == SubTitleLabelType)
        bodyWriter.startElement("chart:subtitle");
    else // if (labelType == TitleLabelType)
        bodyWriter.startElement("chart:title");

    bodyWriter.addAttributePt("svg:x", label->position().x());
    bodyWriter.addAttributePt("svg:y", label->position().y());
    bodyWriter.addAttributePt("svg:width", label->size().width());
    bodyWriter.addAttributePt("svg:height", label->size().height());

    // TODO: Save text label color
    QTextCursor cursor(labelData->document());
    QFont labelFont = cursor.charFormat().font();

    KoGenStyle autoStyle(KoGenStyle::ChartAutoStyle, "chart", 0);
    autoStyle.addPropertyPt("style:rotation-angle", 360 - label->rotation());
    saveOdfFont(autoStyle, labelFont, QColor());
    bodyWriter.addAttribute("chart:style-name", mainStyles.insert(autoStyle, "ch"));

    bodyWriter.startElement("text:p");
    bodyWriter.addTextNode(labelData->document()->toPlainText());
    bodyWriter.endElement(); // text:p
    bodyWriter.endElement(); // chart:title/subtitle/footer
}


const char * odfCharttype(int charttype)
{
    Q_ASSERT(charttype < LastChartType);
    if (charttype >= LastChartType || charttype < 0) {
        charttype = 0;
    }
    return ODF_CHARTTYPES[charttype];
}


static const int NUM_DEFAULT_DATASET_COLORS = 12;

static const char *defaultDataSetColors[NUM_DEFAULT_DATASET_COLORS] =
{
    "#004586",
    "#ff420e",
    "#ffd320",
    "#579d1c",
    "#7e0021",
    "#83caff",
    "#314004",
    "#aecf00",
    "#4b1f6f",
    "#ff950e",
    "#c5000b",
    "#0084d1",
};

QColor defaultDataSetColor(int dataSetNum)
{
    dataSetNum %= NUM_DEFAULT_DATASET_COLORS;
    return QColor(defaultDataSetColors[dataSetNum]);
}

// ================================================================
//                     The Private class


class ChartShape::Private
{
public:
    Private(ChartShape *shape);
    ~Private();

    bool loadOdfLabel(KoShape *label, KoXmlElement &labelElement, KoShapeLoadingContext &context);
    void setChildVisible(KoShape *label, bool doShow);

    // The components of a chart
    KoShape   *title;
    KoShape   *subTitle;
    KoShape   *footer;
    Legend    *legend;
    PlotArea  *plotArea;

    // Data
    ChartProxyModel     *proxyModel;	 /// What's presented to KDChart
    QAbstractItemModel  *internalModel;
    TableSource          tableSource;
    SingleModelHelper   *internalModelHelper;

    bool usesInternalModelOnly; /// @see usesInternalModelOnly()

    ChartDocument *document;

    ChartShape *shape;		// The chart that owns this ChartShape::Private

    KoDocumentResourceManager *resourceManager;
};


ChartShape::Private::Private(ChartShape *shape)
    : internalModel(0)
    , internalModelHelper(0)
    , resourceManager(0)

{
    // Register the owner.
    this->shape = shape;

    // Components
    title    = 0;
    subTitle = 0;
    footer   = 0;
    legend   = 0;
    plotArea = 0;

    // Data
    proxyModel    = 0;

    // If not explicitly set otherwise, this chart provides its own data.
    usesInternalModelOnly = true;

    document = 0;
}

ChartShape::Private::~Private()
{
}

bool ChartShape::Private::loadOdfLabel(KoShape *label, KoXmlElement &labelElement, KoShapeLoadingContext &context)
{
    TextLabelData *labelData = qobject_cast<TextLabelData*>(label->userData());
    if (!labelData)
        return false;

    // Following will always return false cause KoTextShapeData::loadOdf will try to load
    // a frame while our text:p is not within a frame. So, let's just not call loadOdf then...
    //label->loadOdf(labelElement, context);

    // 1. set the text
    KoXmlElement  pElement = KoXml::namedItemNS(labelElement, KoXmlNS::text, "p");

    QTextDocument* doc = labelData->document();
    doc->setPlainText(pElement.text());

    // 2. set the position
    QPointF pos = label->position();
    bool posChanged = false;
    if (labelElement.hasAttributeNS(KoXmlNS::svg, "x")) {
        pos.setX(KoUnit::parseValue(labelElement.attributeNS(KoXmlNS::svg, "x", QString())));
        posChanged = true;
    }
    if (labelElement.hasAttributeNS(KoXmlNS::svg, "y")) {
        pos.setY(KoUnit::parseValue(labelElement.attributeNS(KoXmlNS::svg, "y", QString())));
        posChanged = true;
    }
    if (posChanged) {
        label->setPosition(pos);
    }

    // 3. set the styles
    if (labelElement.hasAttributeNS(KoXmlNS::chart, "style-name")) {
        KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
        styleStack.clear();
        context.odfLoadingContext().fillStyleStack(labelElement, KoXmlNS::chart, "style-name", "chart");

        styleStack.setTypeProperties("chart");
        if (styleStack.hasProperty(KoXmlNS::style, "rotation-angle")) {
            qreal rotationAngle = 360 - KoUnit::parseValue(styleStack.property(KoXmlNS::style, "rotation-angle"));
            label->rotate(rotationAngle);
        }

        styleStack.setTypeProperties("text");

        if (styleStack.hasProperty(KoXmlNS::fo, "font-size")) {
            const qreal fontSize = KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "font-size"));
            QFont font = doc->defaultFont();
            font.setPointSizeF(fontSize);
            doc->setDefaultFont(font);
        }

        if (styleStack.hasProperty(KoXmlNS::fo, "font-family")) {
            const QString fontFamily = styleStack.property(KoXmlNS::fo, "font-family");
            QFont font = doc->defaultFont();
            font.setFamily(fontFamily);
            doc->setDefaultFont(font);
        }
    }

    // 4. set the size
    if (labelElement.hasAttributeNS(KoXmlNS::svg, "width")
        && labelElement.hasAttributeNS(KoXmlNS::svg, "height"))
    {
        const qreal width = KoUnit::parseValue(labelElement.attributeNS(KoXmlNS::svg, "width"));
        const qreal height = KoUnit::parseValue(labelElement.attributeNS(KoXmlNS::svg, "height"));
        label->setSize(QSizeF(width, height));
    } else {
        QSizeF size = shape->size();
        QRect r = QFontMetrics(doc->defaultFont()).boundingRect(
                        labelData->shapeMargins().left, labelData->shapeMargins().top,
                        qMax(CM_TO_POINT(5), qreal(size.width() - pos.x() * 2.0 - labelData->shapeMargins().right)),
                        qMax(CM_TO_POINT(0.6), qreal(size.height() - labelData->shapeMargins().bottom)),
                        Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, doc->toPlainText());
        label->setSize(r.size());
    }

    return true;
}

//
// Show a child, which means either the Title, Subtitle, Footer or Axis Title.
//
// If there is too little room, then make space by shrinking the Plotarea.
//
void ChartShape::Private::setChildVisible(KoShape *child, bool doShow)
{
    Q_ASSERT(child);

    if (child->isVisible() == doShow)
        return;

    child->setVisible(doShow);
    // FIXME: Shouldn't there be a KoShape::VisibilityChanged for KoShape::shapeChanged()?
    shape->layout()->scheduleRelayout();
}


// ================================================================
//                         Class ChartShape
// ================================================================


ChartShape::ChartShape(KoDocumentResourceManager *resourceManager)
    : KoFrameShape(KoXmlNS::draw, "object")
    , KoShapeContainer(new ChartLayout)
    , d (new Private(this))
{
    d->resourceManager = resourceManager;
    setShapeId(ChartShapeId);

    // Instantiated all children first
    d->proxyModel = new ChartProxyModel(this, &d->tableSource);

    d->plotArea = new PlotArea(this);
    d->document = new ChartDocument(this);
    d->legend   = new Legend(this);

    // Configure the plotarea.
    // We need this as the very first step, because some methods
    // here rely on the d->plotArea pointer.
    addShape(d->plotArea);
    d->plotArea->plotAreaInit();
    d->plotArea->setZIndex(0);
    setClipped(d->plotArea, true);
    setInheritsTransform(d->plotArea, true);

    // Configure the legend.
    d->legend->setVisible(true);
    d->legend->setZIndex(1);
    setClipped(d->legend, true);
    setInheritsTransform(d->legend, true);

    // A few simple defaults (chart type and subtype in this case)
    setChartType(BarChartType);
    setChartSubType(NormalChartSubtype);

    // Create the Title, which is a standard TextShape.
    KoShapeFactoryBase *textShapeFactory = KoShapeRegistry::instance()->value(TextShapeId);
    if (textShapeFactory)
        d->title = textShapeFactory->createDefaultShape(resourceManager);
    // Potential problem 1) No TextShape installed
    if (!d->title) {
        d->title = new TextLabelDummy;
        if (ENABLE_USER_INTERACTION)
            KMessageBox::error(0, i18n("The plugin needed for displaying text labels in a chart is not available."),
                                   i18n("Plugin Missing"));
    // Potential problem 2) TextShape incompatible
    } else if (dynamic_cast<TextLabelData*>(d->title->userData()) == 0 &&
                ENABLE_USER_INTERACTION)
            KMessageBox::error(0, i18n("The plugin needed for displaying text labels is not compatible with the current version of the chart Flake shape."),
                                   i18n("Plugin Incompatible"));

    // In both cases we need a KoTextShapeData instance to function. This is
    // enough for unit tests, so there has to be no TextShape plugin doing the
    // actual text rendering, we just need KoTextShapeData which is in the libs.
    if (dynamic_cast<TextLabelData*>(d->title->userData()) == 0) {
        TextLabelData *dataDummy = new TextLabelData;
        KoTextDocumentLayout *documentLayout = new KoTextDocumentLayout(dataDummy->document());
        dataDummy->document()->setDocumentLayout(documentLayout);
        d->title->setUserData(dataDummy);
    }

    // Start with a reasonable default size that we can base all following relative
    // positions of chart elements on.
    setSize(QSizeF(CM_TO_POINT(8), CM_TO_POINT(5)));

    // Add the title to the shape
    addShape(d->title);
    QFont font = titleData()->document()->defaultFont();
    font.setPointSizeF(12.0);
    titleData()->document()->setDefaultFont(font);
    titleData()->document()->setHtml("<div align=\"center\">" + i18n("Title") + "</font></div>");
    // Position the title center at the very top.
    d->title->setSize(QSizeF(CM_TO_POINT(5), CM_TO_POINT(0.7)));
    d->title->setPosition(QPointF(size().width() / 2.0 - d->title->size().width() / 2.0, 0.0));
    d->title->setVisible(false);
    d->title->setZIndex(2);
    setClipped(d->title, true);
    setInheritsTransform(d->title, true);

    // Create the Subtitle and add it to the shape.
    if (textShapeFactory)
        d->subTitle = textShapeFactory->createDefaultShape(resourceManager);
    if (!d->subTitle) {
        d->subTitle = new TextLabelDummy;
    }
    if (dynamic_cast<TextLabelData*>(d->subTitle->userData()) == 0) {
        TextLabelData *dataDummy = new TextLabelData;
        KoTextDocumentLayout *documentLayout = new KoTextDocumentLayout(dataDummy->document());
        dataDummy->document()->setDocumentLayout(documentLayout);
        d->subTitle->setUserData(dataDummy);
    }
    addShape(d->subTitle);
    font = subTitleData()->document()->defaultFont();
    font.setPointSizeF(10.0);
    subTitleData()->document()->setDefaultFont(font);
    subTitleData()->document()->setHtml("<div align=\"center\">" + i18n("Subtitle") + "</div>");

    // Position it in the center, just below the title.
    d->subTitle->setSize(QSizeF(CM_TO_POINT(5), CM_TO_POINT(0.6)));
    d->subTitle->setPosition(QPointF(size().width() / 2.0 - d->title->size().width() / 2.0,
                                       d->title->size().height()));
    d->subTitle->setVisible(false);
    d->subTitle->setZIndex(3);
    setClipped(d->subTitle, true);
    setInheritsTransform(d->subTitle, true);

    // Create the Footer and add it to the shape.
    if (textShapeFactory)
        d->footer = textShapeFactory->createDefaultShape(resourceManager);
    if (!d->footer) {
        d->footer = new TextLabelDummy;
    }
    if (dynamic_cast<TextLabelData*>(d->footer->userData()) == 0) {
        TextLabelData *dataDummy = new TextLabelData;
        KoTextDocumentLayout *documentLayout = new KoTextDocumentLayout(dataDummy->document());
        dataDummy->document()->setDocumentLayout(documentLayout);
        d->footer->setUserData(dataDummy);
    }
    addShape(d->footer);
    font = footerData()->document()->defaultFont();
    font.setPointSizeF(10.0);
    footerData()->document()->setDefaultFont(font);
    footerData()->document()->setHtml("<div align=\"center\">" + i18n("Footer") + "</div>");

    // Position the footer in the center, at the bottom.
    d->footer->setSize(QSizeF(CM_TO_POINT(5), CM_TO_POINT(0.6)));
    d->footer->setPosition(QPointF(size().width() / 2.0 - d->footer->size().width() / 2.0,
                                   size().height() - d->footer->size().height()));
    d->footer->setVisible(false);
    d->footer->setZIndex(4);
    setClipped(d->footer, true);
    setInheritsTransform(d->footer, true);

    // Set default contour (for how text run around is done around this shape)
    // to prevent a crash in LO
    setTextRunAroundContour(KoShape::ContourBox);

    // Enable auto-resizing of chart labels
    foreach(KoShape *label, labels()) {
        TextLabelData *labelData = qobject_cast<TextLabelData*>(label->userData());
        KoTextDocument doc(labelData->document());
//FIXME        doc.setResizeMethod(KoTextDocument::AutoResize);
    }

    KoColorBackground *background = new KoColorBackground(Qt::white);
    setBackground(background);

    KoShapeStroke *stroke = new KoShapeStroke(0, Qt::black);
    setStroke(stroke);

    ChartLayout *l = layout();
    l->setPosition(d->plotArea, CenterPosition);
    l->setPosition(d->title,    TopPosition, 0);
    l->setPosition(d->subTitle, TopPosition, 1);
    l->setPosition(d->footer,   BottomPosition, 1);
    l->setPosition(d->legend,   d->legend->legendPosition());
    l->layout();

    requestRepaint();
}

ChartShape::~ChartShape()
{
    delete d->title;
    delete d->subTitle;
    delete d->footer;

    delete d->legend;
    delete d->plotArea;

    delete d->proxyModel;

    delete d->document;

    delete d;
}

ChartProxyModel *ChartShape::proxyModel() const
{
    return d->proxyModel;
}

KoShape *ChartShape::title() const
{
    return d->title;
}

TextLabelData *ChartShape::titleData() const
{
    TextLabelData *data = qobject_cast<TextLabelData*>(d->title->userData());
    return data;
}


KoShape *ChartShape::subTitle() const
{
    return d->subTitle;
}

TextLabelData *ChartShape::subTitleData() const
{
    TextLabelData *data = qobject_cast<TextLabelData*>(d->subTitle->userData());
    return data;
}

KoShape *ChartShape::footer() const
{
    return d->footer;
}

TextLabelData *ChartShape::footerData() const
{
    TextLabelData *data = qobject_cast<TextLabelData*>(d->footer->userData());
    return data;
}

QList<KoShape*> ChartShape::labels() const
{
    QList<KoShape*> labels;
    labels.append(d->title);
    labels.append(d->footer);
    labels.append(d->subTitle);
    foreach(Axis *axis, plotArea()->axes()) {
        labels.append(axis->title());
    }
    return labels;
}

Legend *ChartShape::legend() const
{
    // There has to be a valid legend even, if it's hidden.
    Q_ASSERT(d->legend);
    return d->legend;
}

PlotArea *ChartShape::plotArea() const
{
    return d->plotArea;
}

ChartLayout *ChartShape::layout() const
{
    ChartLayout *l = dynamic_cast<ChartLayout*>(KoShapeContainer::model());
    Q_ASSERT(l);
    return l;
}


void ChartShape::showTitle(bool doShow)
{
    d->setChildVisible(d->title, doShow);
}

void ChartShape::showSubTitle(bool doShow)
{
    d->setChildVisible(d->subTitle, doShow);
}

void ChartShape::showFooter(bool doShow)
{
    d->setChildVisible(d->footer, doShow);
}

QAbstractItemModel *ChartShape::internalModel() const
{
    return d->internalModel;
}

void ChartShape::setInternalModel(QAbstractItemModel *model)
{
    Table *table = d->tableSource.get(model);
    Q_ASSERT(table);
    delete d->internalModelHelper;
    delete d->internalModel;
    d->internalModel = model;
    d->internalModelHelper = new SingleModelHelper(table, d->proxyModel);
}

TableSource *ChartShape::tableSource() const
{
    return &d->tableSource;
}

bool ChartShape::usesInternalModelOnly() const
{
    return d->usesInternalModelOnly;
}

void ChartShape::setUsesInternalModelOnly(bool doesSo)
{
    d->usesInternalModelOnly = doesSo;
}


// ----------------------------------------------------------------
//                         getters and setters


ChartType ChartShape::chartType() const
{
    Q_ASSERT(d->plotArea);
    return d->plotArea->chartType();
}

ChartSubtype ChartShape::chartSubType() const
{
    Q_ASSERT(d->plotArea);
    return d->plotArea->chartSubType();
}

bool ChartShape::isThreeD() const
{
    Q_ASSERT(d->plotArea);
    return d->plotArea->isThreeD();
}

void ChartShape::setSheetAccessModel(QAbstractItemModel *model)
{
    d->tableSource.setSheetAccessModel(model);
}

void ChartShape::reset(const QString &region,
                       bool firstRowIsLabel,
                       bool firstColumnIsLabel,
                       Qt::Orientation dataDirection)
{
    // This method is provided via KoChartInterface, which is
    // used by embedding applications.
    d->usesInternalModelOnly = false;
    d->proxyModel->setFirstRowIsLabel(firstRowIsLabel);
    d->proxyModel->setFirstColumnIsLabel(firstColumnIsLabel);
    d->proxyModel->setDataDirection(dataDirection);
    d->proxyModel->reset(CellRegion(&d->tableSource, region));
}

void ChartShape::setChartType(ChartType type)
{
    Q_ASSERT(d->plotArea);
    d->proxyModel->setDataDimensions(numDimensions(type));
    d->plotArea->setChartType(type);
    emit chartTypeChanged(type);
}

void ChartShape::setChartSubType(ChartSubtype subType)
{
    Q_ASSERT(d->plotArea);
    d->plotArea->setChartSubType(subType);
    emit updateConfigWidget();
}

void ChartShape::setThreeD(bool threeD)
{
    Q_ASSERT(d->plotArea);
    d->plotArea->setThreeD(threeD);
}


// ----------------------------------------------------------------


void ChartShape::paintComponent(QPainter &painter,
                                const KoViewConverter &converter, KoShapePaintingContext &paintContext)
{
    // Only does a relayout if scheduled
    layout()->layout();

    // Paint the background
    if (background()) {
        applyConversion(painter, converter);

        // Calculate the clipping rect
        QRectF paintRect = QRectF(QPointF(0, 0), size());
        painter.setClipRect(paintRect, Qt::IntersectClip);

        QPainterPath p;
        p.addRect(paintRect);
        background()->paint(painter, converter, paintContext, p);
    }
}

void ChartShape::paintDecorations(QPainter &painter,
                                  const KoViewConverter &converter,
                                  const KoCanvasBase *canvas)
{
    // This only is a helper decoration, do nothing if we're already
    // painting handles anyway.
    Q_ASSERT(canvas);
    if (canvas->shapeManager()->selection()->selectedShapes().contains(this))
        return;

    if (stroke())
        return;

    QRectF border = QRectF(QPointF(-1.5, -1.5),
                           converter.documentToView(size()) + QSizeF(1.5, 1.5));

    painter.setPen(QPen(Qt::lightGray, 0));
    painter.drawRect(border);
}


// ----------------------------------------------------------------
//                         Loading and Saving


bool ChartShape::loadEmbeddedDocument(KoStore *store,
                                      const KoXmlElement &objectElement,
                                      const KoOdfLoadingContext &loadingContext)
{
    if (!objectElement.hasAttributeNS(KoXmlNS::xlink, "href")) {
        kError() << "Object element has no valid xlink:href attribute";
        return false;
    }

    QString url = objectElement.attributeNS(KoXmlNS::xlink, "href");

    // It can happen that the url is empty e.g. when it is a
    // presentation:placeholder.
    if (url.isEmpty()) {
        return true;
    }

    QString tmpURL;
    if (url[0] == '#')
        url = url.mid(1);

    if (KUrl::isRelativeUrl(url)) {
        if (url.startsWith("./"))
            tmpURL = QString(INTERNAL_PROTOCOL) + ":/" + url.mid(2);
        else
            tmpURL = QString(INTERNAL_PROTOCOL) + ":/" + url;
    }
    else
        tmpURL = url;

    QString path = tmpURL;
    if (tmpURL.startsWith(INTERNAL_PROTOCOL)) {
        path = store->currentDirectory();
        if (!path.isEmpty() && !path.endsWith('/'))
            path += '/';
        QString relPath = KUrl(tmpURL).path();
        path += relPath.mid(1); // remove leading '/'
    }
    if (!path.endsWith('/'))
        path += '/';

    const QString mimeType = loadingContext.mimeTypeForPath(path);
    //kDebug(35001) << "path for manifest file=" << path << "mimeType=" << mimeType;
    if (mimeType.isEmpty()) {
        //kDebug(35001) << "Manifest doesn't have media-type for" << path;
        return false;
    }

    const bool isOdf = mimeType.startsWith("application/vnd.oasis.opendocument");
    if (!isOdf) {
        tmpURL += "/maindoc.xml";
        //kDebug(35001) << "tmpURL adjusted to" << tmpURL;
    }

    //kDebug(35001) << "tmpURL=" << tmpURL;
    QString errorMsg;
    KoDocumentEntry e = KoDocumentEntry::queryByMimeType(mimeType);
    if (e.isEmpty()) {
        return false;
    }

    bool res = true;
    if (tmpURL.startsWith(STORE_PROTOCOL)
         || tmpURL.startsWith(INTERNAL_PROTOCOL)
         || KUrl::isRelativeUrl(tmpURL))
    {
        if (isOdf) {
            store->pushDirectory();
            Q_ASSERT(tmpURL.startsWith(INTERNAL_PROTOCOL));
            QString relPath = KUrl(tmpURL).path().mid(1);
            store->enterDirectory(relPath);
            res = d->document->loadOasisFromStore(store);
            store->popDirectory();
        } else {
            if (tmpURL.startsWith(INTERNAL_PROTOCOL))
                tmpURL = KUrl(tmpURL).path().mid(1);
            res = d->document->loadFromStore(store, tmpURL);
        }
        d->document->setStoreInternal(true);
    }
    else {
        // Reference to an external document. Hmmm...
        d->document->setStoreInternal(false);
        KUrl url(tmpURL);
        if (!url.isLocalFile()) {
            //QApplication::restoreOverrideCursor();

            // For security reasons we need to ask confirmation if the
            // url is remote.
            int result = KMessageBox::warningYesNoCancel(
                0, i18n("This document contains an external link to a remote document\n%1", tmpURL),
                i18n("Confirmation Required"), KGuiItem(i18n("Download")), KGuiItem(i18n("Skip")));

            if (result == KMessageBox::Cancel) {
                //d->m_parent->setErrorMessage("USER_CANCELED");
                return false;
            }
            if (result == KMessageBox::Yes)
                res = d->document->openUrl(url);
            // and if == No, res will still be false so we'll use a kounavail below
        }
        else
            res = d->document->openUrl(url);
    }

    if (!res) {
        QString errorMessage = d->document->errorMessage();
        return false;
    }
        // Still waiting...
        //QApplication::setOverrideCursor(Qt::WaitCursor);

    tmpURL.clear();

    // see KoDocument::insertChild for an explanation what's going on
    // now :-)
    /*if (parentDocument()) {
        KoDocument *parent = parentDocument();

        KParts::PartManager* manager = parent->manager();
        if (manager && !manager->parts().isEmpty())
        {
            if (!manager->parts().contains(d->document)
                && !parent->isSingleViewMode())
                manager->addPart(d->document, false);
        }
    }*/

    //QApplication::restoreOverrideCursor();

    return res;
}

bool ChartShape::loadOdf(const KoXmlElement &element,
                         KoShapeLoadingContext &context)
{
    //struct Timer{QTime t;Timer(){t.start();} ~Timer(){qDebug()<<">>>>>"<<t.elapsed();}} timer;

    // Load common attributes of (frame) shapes.  If you change here,
    // don't forget to also change in saveOdf().
    loadOdfAttributes(element, context, OdfAllAttributes);
    return loadOdfFrame(element, context);
}

// Used to load the actual contents from the ODF frame that surrounds
// the chart in the ODF file.
bool ChartShape::loadOdfFrameElement(const KoXmlElement &element,
                                     KoShapeLoadingContext &context)
{
    if (element.tagName() == "object")
        return loadEmbeddedDocument(context.odfLoadingContext().store(),
                                    element,
                                    context.odfLoadingContext());

    qWarning() << "Unknown frame element <" << element.tagName() << ">";
    return false;
}

bool ChartShape::loadOdfChartElement(const KoXmlElement &chartElement,
                                     KoShapeLoadingContext &context)
{
    // Use a helper-class created on the stack to be sure a we always leave
    // this method with a call to endLoading proxyModel()->endLoading()
    struct ProxyModelLoadState {
        ChartProxyModel *m;
        ProxyModelLoadState(ChartProxyModel *m) : m(m) { m->beginLoading(); }
        ~ProxyModelLoadState() { m->endLoading(); }
    };
    ProxyModelLoadState proxyModelLoadState(proxyModel());

    // The shared data will automatically be deleted in the destructor
    // of KoShapeLoadingContext
    OdfLoadingHelper *helper = new OdfLoadingHelper;
    helper->tableSource = &d->tableSource;
    helper->chartUsesInternalModelOnly = d->usesInternalModelOnly;

    // Get access to sheets in KSpread
    QAbstractItemModel *sheetAccessModel = 0;
    if (resourceManager() &&
         resourceManager()->hasResource(75751149)) { // duplicated from kspread
        QVariant var = resourceManager()->resource(75751149);
        sheetAccessModel = static_cast<QAbstractItemModel*>(var.value<void*>());
        if (sheetAccessModel) {
            // We're embedded in KSpread, which means KSpread provides the data
            d->usesInternalModelOnly = false;
            d->tableSource.setSheetAccessModel(sheetAccessModel);
            helper->chartUsesInternalModelOnly = d->usesInternalModelOnly;
        }
    }
    context.addSharedData(OdfLoadingHelperId, helper);

    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.clear();
    if (chartElement.hasAttributeNS(KoXmlNS::chart, "style-name")) {
        context.odfLoadingContext().fillStyleStack(chartElement, KoXmlNS::chart, "style-name", "chart");
        styleStack.setTypeProperties("graphic");
    }
    // Also load the size here as it, if specified here, overwrites the frame's size,
    // See ODF specs for chart:chart element for more details.
    loadOdfAttributes(chartElement, context,
                      OdfAdditionalAttributes | OdfMandatories | OdfCommonChildElements | OdfStyle | OdfSize);

#ifndef NWORKAROUND_ODF_BUGS
    if (!background()) {
        const QColor color = KoOdfWorkaround::fixMissingFillColor(chartElement, context);
        if (color.isValid()) // invalid color means do not set KoColorBackground but be transparent instead
            setBackground(new KoColorBackground(color));
    }
#endif

    // Check if we're loading an embedded document
    if (!chartElement.hasAttributeNS(KoXmlNS::chart, "class")) {
        kDebug(35001) << "Error: Embedded document has no chart:class attribute.";
        return false;
    }

    Q_ASSERT(d->plotArea);


    // 1. Load the chart type.
    const QString chartClass = chartElement.attributeNS(KoXmlNS::chart, "class", QString());
    KChart::ChartType chartType = KChart::BarChartType;
    // Find out what charttype the chart class corresponds to.
    bool  knownType = false;
    for (int type = 0; type < (int)LastChartType; ++type) {
        if (chartClass == ODF_CHARTTYPES[(ChartType)type]) {
            //kDebug(35001) <<"found chart of type" << chartClass;

            chartType = (ChartType)type;
            // Set the dimensionality of the data points, we can not call
            // setType here as we bubble charts requires that the datasets already exist
            proxyModel()->setDataDimensions(numDimensions(chartType));
            knownType = true;
            break;
        }
    }

    // If we can't find out what charttype it is, we might as well end here.
    if (!knownType) {
        // FIXME: Find out what the equivalent of
        //        KoDocument::setErrorMessage() is for KoShape.
        //setErrorMessage(i18n("Unknown chart type %1" ,chartClass));
        return false;
    }

    // 2. Load the data
//     int dimensions = numDimensions(chartType);
//     qDebug() << "DIMENSIONS" << dimensions;
//     d->proxyModel->setDataDimensions(dimensions);
//     qDebug() << d->proxyModel->dataSets().count();
    KoXmlElement  dataElem = KoXml::namedItemNS(chartElement, KoXmlNS::table, "table");
    if (!dataElem.isNull()) {
        if (!loadOdfData(dataElem, context))
            return false;
    }

    // 3. Load the plot area (this is where the meat is!).
    KoXmlElement  plotareaElem = KoXml::namedItemNS(chartElement, KoXmlNS::chart, "plot-area");

    if (!plotareaElem.isNull()) {
        d->plotArea->setChartType(chartType);
        d->plotArea->setChartSubType(chartSubType());
        if (!d->plotArea->loadOdf(plotareaElem, context))
            return false;
//         d->plotArea->setChartType(chartType);
//         d->plotArea->setChartSubType(chartSubType());
    }

    // 4. Load the title.
    KoXmlElement titleElem = KoXml::namedItemNS(chartElement,
                                                 KoXmlNS::chart, "title");
    d->setChildVisible(d->title, !titleElem.isNull());
    if (!titleElem.isNull()) {
        if (!d->loadOdfLabel(d->title, titleElem, context))
            return false;
    }

    // 5. Load the subtitle.
    KoXmlElement subTitleElem = KoXml::namedItemNS(chartElement, KoXmlNS::chart, "subtitle");
    d->setChildVisible(d->subTitle, !subTitleElem.isNull());
    if (!subTitleElem.isNull()) {
        if (!d->loadOdfLabel(d->subTitle, subTitleElem, context))
            return false;
    }

    // 6. Load the footer.
    KoXmlElement footerElem = KoXml::namedItemNS(chartElement, KoXmlNS::chart, "footer");
    d->setChildVisible(d->footer, !footerElem.isNull());
    if (!footerElem.isNull()) {
        if (!d->loadOdfLabel(d->footer, footerElem, context))
            return false;
    }

    // 7. Load the legend.
    KoXmlElement legendElem = KoXml::namedItemNS(chartElement, KoXmlNS::chart, "legend");
    d->setChildVisible(d->legend, !legendElem.isNull());
    if (!legendElem.isNull()) {
        if (!d->legend->loadOdf(legendElem, context))
            return false;
    }

    // 8. Sets the chart type
    setChartType(chartType);

    d->legend->update();

    requestRepaint();

    return true;
}

bool ChartShape::loadOdfData(const KoXmlElement &tableElement,
                             KoShapeLoadingContext &context)
{
    // There is no table element to load
    if (tableElement.isNull() || !tableElement.isElement())
        return true;

    // An internal model might have been set before in ChartShapeFactory.
    if (d->internalModel) {
        Table *oldInternalTable = d->tableSource.get(d->internalModel);
        Q_ASSERT(oldInternalTable);
        d->tableSource.remove(oldInternalTable->name());
    }

    // FIXME: Make model->loadOdf() return a bool, and use it here.
    // Create a table with data from document, add it as table source
    // and reset the proxy only with data from this new table.
    ChartTableModel *internalModel = new ChartTableModel;
    internalModel->loadOdf(tableElement, context);

    QString tableName = tableElement.attributeNS(KoXmlNS::table, "name");
    d->tableSource.add(tableName, internalModel);
    // TODO: d->tableSource.setAvoidNameClash(tableName)
    setInternalModel(internalModel);

    return true;
}

void ChartShape::saveOdf(KoShapeSavingContext & context) const
{
    Q_ASSERT(d->plotArea);

    KoXmlWriter&  bodyWriter = context.xmlWriter();

    // Check if we're saving to a chart document. If not, embed a
    // chart document.  ChartShape::saveOdf() will then be called
    // again later, when the current document saves the embedded
    // documents.
    //
    // FIXME: The check isEmpty() fixes a crash that happened when a
    //        chart shape was saved from Words.  There are two
    //        problems with this fix:
    //        1. Checking the tag hierarchy is hardly the right way to do this
    //        2. The position doesn't seem to be saved yet.
    //
    //        Also, I have to check with the other apps, e.g. kspread,
    //        if it works there too.
    //
    QList<const char*>  tagHierarchy = bodyWriter.tagHierarchy();
    if (tagHierarchy.isEmpty()
        || QString(tagHierarchy.last()) != "office:chart")
    {
        bodyWriter.startElement("draw:frame");
        // See also loadOdf() in loadOdfAttributes.
        saveOdfAttributes(context, OdfAllAttributes);

        bodyWriter.startElement("draw:object");
        context.embeddedSaver().embedDocument(bodyWriter, d->document);
        bodyWriter.endElement(); // draw:object

        bodyWriter.endElement(); // draw:frame
        return;
    }

    KoGenStyles&  mainStyles(context.mainStyles());

    bodyWriter.startElement("chart:chart");

    saveOdfAttributes(context, OdfSize);

    KoGenStyle style(KoGenStyle::GraphicAutoStyle, "chart");
    bodyWriter.addAttribute("chart:style-name", saveStyle(style, context));

    // 1. Write the chart type.
    bodyWriter.addAttribute("chart:class", ODF_CHARTTYPES[d->plotArea->chartType() ]);

    // 2. Write the title.
    if (d->title->isVisible())
        saveOdfLabel(d->title, bodyWriter, mainStyles, TitleLabelType);

    // 3. Write the subtitle.
    if (d->subTitle->isVisible())
        saveOdfLabel(d->subTitle, bodyWriter, mainStyles, SubTitleLabelType);

    // 4. Write the footer.
    if (d->footer->isVisible())
        saveOdfLabel(d->footer, bodyWriter, mainStyles, FooterLabelType);

    // 5. Write the legend.
    if (d->legend->isVisible())
        d->legend->saveOdf(context);

    // 6. Write the plot area (this is where the real action is!).
    d->plotArea->saveOdf(context);

    // 7. Save the data
    saveOdfData(bodyWriter, mainStyles);

    bodyWriter.endElement(); // chart:chart
}

static void saveOdfDataRow(KoXmlWriter &bodyWriter, QAbstractItemModel *table, int row)
{
    bodyWriter.startElement("table:table-row");
    const int cols = table->columnCount();
    for (int col = 0; col < cols; ++col) {
        //QVariant value(internalModel.cellVal(row, col));
        QModelIndex  index = table->index(row, col);
        QVariant     value = table->data(index);

        bool ok;
        double val = value.toDouble(&ok);
        if (ok) {
            value = val;
        }

        QString  valType;
        QString  valStr;

        switch (value.type()) {
        case QVariant::Invalid:
            break;
        case QVariant::String:
            valType = "string";
            valStr  = value.toString();
            break;
        case QVariant::Double:
            valType = "float";
            valStr  = QString::number(value.toDouble(), 'g', DBL_DIG);
            break;
        case QVariant::DateTime:

            valType = "date";
            valStr  = ""; /* like in saveXML, but why? */
            break;
        default:
            kDebug(35001) <<"ERROR: cell" << row <<"," << col
                          << " has unknown type." << endl;
        }

        // Add the value type and the string to the XML tree.
        bodyWriter.startElement("table:table-cell");
        if (!valType.isEmpty()) {
            bodyWriter.addAttribute("office:value-type", valType);
            if (value.type() == QVariant::Double)
                bodyWriter.addAttribute("office:value", valStr);

            bodyWriter.startElement("text:p");
            bodyWriter.addTextNode(valStr);
            bodyWriter.endElement(); // text:p
        }

        bodyWriter.endElement(); // table:table-cell
    }

    bodyWriter.endElement(); // table:table-row
}

void ChartShape::saveOdfData(KoXmlWriter &bodyWriter, KoGenStyles &mainStyles) const
{
    Q_UNUSED(mainStyles);

    // FIXME: Move this method to a sane place
    QAbstractItemModel *internalModel = d->internalModel;
    Table *internalTable = d->tableSource.get(internalModel);
    Q_ASSERT(internalTable);

    // Only save the data if we actually have some.
    if (!internalModel)
        return;

    const int rows = internalModel->rowCount();
    const int cols = internalModel->columnCount();

    bodyWriter.startElement("table:table");
    bodyWriter.addAttribute("table:name", internalTable->name());

    // Exactly one header column, always.
    bodyWriter.startElement("table:table-header-columns");
    bodyWriter.startElement("table:table-column");
    bodyWriter.endElement(); // table:table-column
    bodyWriter.endElement(); // table:table-header-columns

    // Then "cols" columns
    bodyWriter.startElement("table:table-columns");
    bodyWriter.startElement("table:table-column");
    bodyWriter.addAttribute("table:number-columns-repeated", cols);
    bodyWriter.endElement(); // table:table-column
    bodyWriter.endElement(); // table:table-columns

    int row = 0;

    bodyWriter.startElement("table:table-header-rows");
    if (rows > 0)
        saveOdfDataRow(bodyWriter, internalModel, row++);
    bodyWriter.endElement(); // table:table-header-rows

    // Here start the actual data rows.
    bodyWriter.startElement("table:table-rows");
    //QStringList::const_iterator rowLabelIt = m_rowLabels.begin();
    for (; row < rows ; ++row)
        saveOdfDataRow(bodyWriter, internalModel, row);

    bodyWriter.endElement(); // table:table-rows
    bodyWriter.endElement(); // table:table
}

void ChartShape::update() const
{
    KoShape::update();
}

void ChartShape::relayout() const
{
    Q_ASSERT(d->plotArea);
    d->plotArea->relayout();
    KoShape::update();
}

void ChartShape::requestRepaint() const
{
    Q_ASSERT(d->plotArea);
    d->plotArea->requestRepaint();
}

KoDocumentResourceManager *ChartShape::resourceManager() const
{
    return d->resourceManager;
}

void ChartShape::setEnableUserInteraction(bool enable)
{
    ENABLE_USER_INTERACTION = enable;
}

} // Namespace KChart
