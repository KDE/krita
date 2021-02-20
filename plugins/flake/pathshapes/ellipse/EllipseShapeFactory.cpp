/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "EllipseShapeFactory.h"
#include "EllipseShape.h"
#include "EllipseShapeConfigWidget.h"
#include <KoShapeStroke.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoGradientBackground.h>
#include <KoShapeLoadingContext.h>

#include <KoIcon.h>
#include <klocalizedstring.h>

#include "kis_pointer_utils.h"

EllipseShapeFactory::EllipseShapeFactory()
    : KoShapeFactoryBase(EllipseShapeId, i18n("Ellipse"))
{
    setToolTip(i18n("An ellipse"));
    setIconName(koIconNameCStr("ellipse-shape"));
    setFamily("geometric");
    setLoadingPriority(1);

    QList<QPair<QString, QStringList> > elementNamesList;
    elementNamesList.append(qMakePair(QString(KoXmlNS::draw), QStringList("circle")));
    elementNamesList.append(qMakePair(QString(KoXmlNS::draw), QStringList("ellipse")));
    elementNamesList.append(qMakePair(QString(KoXmlNS::svg), QStringList("circle")));
    elementNamesList.append(qMakePair(QString(KoXmlNS::svg), QStringList("ellipse")));
    elementNamesList.append(qMakePair(QString(KoXmlNS::svg), QStringList("sodipodi:arc")));
    elementNamesList.append(qMakePair(QString(KoXmlNS::svg), QStringList("krita:arc")));
    setXmlElements(elementNamesList);
}

KoShape *EllipseShapeFactory::createDefaultShape(KoDocumentResourceManager *) const
{
    EllipseShape *ellipse = new EllipseShape();

    ellipse->setStroke(toQShared(new KoShapeStroke(1.0)));
    ellipse->setShapeId(KoPathShapeId);

    QRadialGradient *gradient = new QRadialGradient(QPointF(0.5, 0.5), 0.5, QPointF(0.25, 0.25));
    gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient->setColorAt(0.0, Qt::white);
    gradient->setColorAt(1.0, Qt::green);
    ellipse->setBackground(QSharedPointer<KoGradientBackground>(new KoGradientBackground(gradient)));

    return ellipse;
}

bool EllipseShapeFactory::supports(const KoXmlElement &e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(context);
    return (e.localName() == "ellipse" || e.localName() == "circle")
           && e.namespaceURI() == KoXmlNS::draw;
}

QList<KoShapeConfigWidgetBase *> EllipseShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase *> panels;
    panels.append(new EllipseShapeConfigWidget());
    return panels;
}
