/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
#include "ChartShapeFactory.h"

// Qt
#include <QStringList>
#include <QStandardItemModel>

// KDE
#include <kiconloader.h>
#include <kpluginfactory.h>
#include <klocale.h>

// Calligra
#include <KoIcon.h>
#include <KoProperties.h>
#include <KoToolRegistry.h>
#include <KoShapeRegistry.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoDocumentResourceManager.h>

// Chart shape
#include "ChartToolFactory.h"
#include "ChartConfigWidget.h"
#include "ChartProxyModel.h"
#include "PlotArea.h"
#include "Axis.h"
#include "Legend.h"
#include "TableSource.h"


using namespace KChart;

K_PLUGIN_FACTORY(ChartShapePluginFactory, registerPlugin<ChartShapePlugin>();)
K_EXPORT_PLUGIN(ChartShapePluginFactory("ChartShape"))

ChartShapePlugin::ChartShapePlugin(QObject * parent, const QVariantList&)
    : QObject(parent)
{
    // Register the chart shape factory.
    KoShapeRegistry::instance()->add(new ChartShapeFactory());

    // Register all tools for the chart shape.
    KoToolRegistry::instance()->add(new ChartToolFactory());
}


ChartShapeFactory::ChartShapeFactory()
    : KoShapeFactoryBase(ChartShapeId, i18n("Chart"))
{
    setXmlElementNames("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0", QStringList("object"));
    setToolTip(i18n("Business charts"));

    KIconLoader::global()->addAppDir("kchart");
    setIconName(koIconNameCStr("kchart"));

    // Default 'app specific' config pages i.e. unless an app defines
    // other config pages, these are used.
    QList<KoShapeConfigFactoryBase*> panelFactories;
    // panelFactories.append(new ChartDataConfigFactory());
    setOptionPanels(panelFactories);
}


bool ChartShapeFactory::supports(const KoXmlElement &element, KoShapeLoadingContext &context) const
{
    if (element.namespaceURI() == "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
        && element.tagName() == "object") {

        QString href = element.attribute("href");
        if (!href.isEmpty()) {
            // check the mimetype
            if (href.startsWith("./")) {
                href.remove(0,2);
            }
            const QString mimetype = context.odfLoadingContext().mimeTypeForPath(href);
            return mimetype.isEmpty() || mimetype == "application/vnd.oasis.opendocument.chart";
        }
    }
    return false;
}

KoShape *ChartShapeFactory::createShapeFromOdf(const KoXmlElement &element,
                                               KoShapeLoadingContext &context)
{
    ChartShape* shape = new ChartShape(context.documentResourceManager());

    if (shape->shapeId().isEmpty())
        shape->setShapeId(id());

    context.odfLoadingContext().styleStack().save();
    bool loaded = shape->loadOdf(element, context);
    context.odfLoadingContext().styleStack().restore();

    if (!loaded) {
        delete shape;
        return 0;
    }

    return shape;
}

KoShape *ChartShapeFactory::createDefaultShape(KoDocumentResourceManager *documentResources) const
{
    ChartShape* shape = new ChartShape(documentResources);
    ChartProxyModel *proxyModel = shape->proxyModel();

    // Fill cells with data.
    QStandardItemModel  *m_chartData = new QStandardItemModel();
    Table *internalTable = shape->tableSource()->add("internal-model", m_chartData);
    Q_ASSERT(!shape->internalModel());
    // setInternalModel() assumes that m_chartData has already been added to shape->tableSource().
    shape->setInternalModel(m_chartData);
    // TODO (not implemented yet)
    // shape->tableSource()->setRenameOnNameClash(internalTable);
    m_chartData->setRowCount(4);
    m_chartData->setColumnCount(5);

    // Vertical header data
    m_chartData->setData(m_chartData->index(1, 0), i18n("January"));
    m_chartData->setData(m_chartData->index(2, 0), i18n("July"));
    m_chartData->setData(m_chartData->index(3, 0), i18n("December"));

    // Horizontal header data
    m_chartData->setData(m_chartData->index(0, 1), i18n("Column %1", 1));
    m_chartData->setData(m_chartData->index(0, 2), i18n("Column %1", 2));
    m_chartData->setData(m_chartData->index(0, 3), i18n("Column %1", 3));
    m_chartData->setData(m_chartData->index(0, 4), i18n("Column %1", 4));

    // First row
    m_chartData->setData(m_chartData->index(1, 1), 5.7);
    m_chartData->setData(m_chartData->index(1, 2), 3.4);
    m_chartData->setData(m_chartData->index(1, 3), 1.2);
    m_chartData->setData(m_chartData->index(1, 4), 8.4);

    // Second row
    m_chartData->setData(m_chartData->index(2, 1), 2.1);
    m_chartData->setData(m_chartData->index(2, 2), 6.5);
    m_chartData->setData(m_chartData->index(2, 3), 0.9);
    m_chartData->setData(m_chartData->index(2, 4), 1.5);

    // Third row
    m_chartData->setData(m_chartData->index(3, 1), 7.9);
    m_chartData->setData(m_chartData->index(3, 2), 3.5);
    m_chartData->setData(m_chartData->index(3, 3), 8.6);
    m_chartData->setData(m_chartData->index(3, 4), 4.3);

    proxyModel->setFirstRowIsLabel(true);
    proxyModel->setFirstColumnIsLabel(true);
    proxyModel->reset(CellRegion(internalTable, QRect(1, 1, 5, 4)));

    const QSizeF shapeSize = shape->size();

    QPointF  plotAreaPos(0.0, 0.0);
    QSizeF   plotAreaSize(shapeSize);
    Legend *legend = shape->legend();
    legend->rebuild();          // Implies update()

    QPointF  legendPos(0.0, 0.0);
    QSizeF   legendSize = legend->size();
    legendPos.ry() = shapeSize.height() / 2.0 - legendSize.height() / 2.0;
    plotAreaSize.rwidth() -= legendSize.width();

    Axis    *xAxis      = shape->plotArea()->xAxis();
    KoShape *xAxisTitle = xAxis->title();
    if (xAxisTitle) {
        xAxis->setTitleText(i18n("Month"));
        xAxisTitle->setPosition(QPointF(shapeSize.width() / 2.0 - xAxisTitle->size().width() / 2.0,
                                        shapeSize.height() - xAxisTitle->size().height()));
        plotAreaSize.rheight() -= xAxisTitle->size().height();
    }

    Axis    *yAxis      = shape->plotArea()->yAxis();
    KoShape *yAxisTitle = yAxis->title();
    if (yAxisTitle) {
        yAxis->setTitleText(i18n("Growth in %"));
        yAxisTitle->setPosition(QPointF(-yAxisTitle->size().width() / 2.0 + yAxisTitle->size().height() / 2.0,
                                        shapeSize.height() / 2.0 - yAxisTitle->size().height() / 2.0));

        plotAreaPos.rx() += yAxisTitle->size().height();
        legendPos.rx() += yAxisTitle->size().height();
        plotAreaSize.rwidth() -= yAxisTitle->size().height();
    }

    if (legend) {
        legendPos.rx() += plotAreaSize.width();
        legend->setPosition(legendPos);
    }

    shape->plotArea()->setPosition(plotAreaPos);
    shape->plotArea()->setSize(plotAreaSize );

    return shape;
}

QList<KoShapeConfigWidgetBase*> ChartShapeFactory::createShapeOptionPanels()
{
    return QList<KoShapeConfigWidgetBase*>();
}

void ChartShapeFactory::newDocumentResourceManager(KoDocumentResourceManager *manager) const
{
    Q_UNUSED(manager);
}

#include "ChartShapeFactory.moc"
