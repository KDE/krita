/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoSvgTextLoader.h"
#include "KoSvgTextShape_p.h"


struct KoSvgTextLoader::Private {

    Private(KoSvgTextShape *shape)
        : shape(shape)
        , currentNode(shape->d->textData.childEnd())
    {
        shape->d->isLoading = true;
        shape->d->textData.erase(shape->d->textData.childBegin(), shape->d->textData.childEnd());
        currentNode = shape->d->textData.childEnd();
    }

    KoSvgTextShape *shape;
    KisForest<KoSvgTextContentElement>::child_iterator currentNode;
};

KoSvgTextLoader::KoSvgTextLoader(KoSvgTextShape *shape)
    : d(new Private(shape))
{

}

KoSvgTextLoader::~KoSvgTextLoader()
{
    // run clean-up after parsing to remove empty spans and the like.
    d->shape->d->updateShapeGroup();
    d->shape->cleanUp();
    d->shape->d->isLoading = false;
    d->shape->relayout();
}

void KoSvgTextLoader::enterNodeSubtree()
{
    if (KisForestDetail::isEnd(d->currentNode)) {
        nextNode();
    }
    d->currentNode = childEnd(d->currentNode);
}

void KoSvgTextLoader::leaveNodeSubtree()
{
    d->currentNode = KisForestDetail::parent(d->currentNode);
}

void KoSvgTextLoader::nextNode()
{
    d->currentNode = d->shape->d->textData.insert(KisForestDetail::siblingEnd(d->currentNode), KoSvgTextContentElement());
}

bool KoSvgTextLoader::loadSvg(const QDomElement &element, SvgLoadingContext &context, bool root)
{
    if (KisForestDetail::isEnd(d->currentNode)) {
        nextNode();
    }
    return d->currentNode->loadSvg(element, context, root);
}

bool KoSvgTextLoader::loadSvgText(const QDomText &text, SvgLoadingContext &context)
{
    if (KisForestDetail::isEnd(d->currentNode)) {
        nextNode();
    }
    return d->currentNode->loadSvgTextNode(text, context);
}

void KoSvgTextLoader::setStyleInfo(KoShape *s)
{
    if (!KisForestDetail::isEnd(d->currentNode)) {
        // find closest parent stroke and fill so we can check for inheritance.
        QSharedPointer<KoShapeBackground> parentBg;
        KoShapeStrokeModelSP parentStroke;
        for (auto it = KisForestDetail::hierarchyBegin(d->currentNode); it != KisForestDetail::hierarchyEnd(d->currentNode); it++) {
            if (it->properties.hasProperty(KoSvgTextProperties::FillId)) {
                parentBg = it->properties.background();
                break;
            }
        }
        for (auto it = KisForestDetail::hierarchyBegin(d->currentNode); it != KisForestDetail::hierarchyEnd(d->currentNode); it++) {
            if (it->properties.hasProperty(KoSvgTextProperties::StrokeId)) {
                parentStroke = it->properties.stroke();
                break;
            }
        }

        if (!s->inheritBackground()) {
            if ((parentBg && !parentBg->compareTo(s->background().data()))
                    || (!parentBg && s->background())) {
                d->currentNode->properties.setProperty(KoSvgTextProperties::FillId,
                                                       QVariant::fromValue(KoSvgText::BackgroundProperty(s->background())));
            }
        }
        if (!s->inheritStroke()) {
            if ((parentStroke && (!parentStroke->compareFillTo(s->stroke().data()) || !parentStroke->compareStyleTo(s->stroke().data())))
                    || (!parentStroke && s->stroke())) {
                d->currentNode->properties.setProperty(KoSvgTextProperties::StrokeId,
                                                       QVariant::fromValue(KoSvgText::StrokeProperty(s->stroke())));
            }
        }
        d->currentNode->properties.setProperty(KoSvgTextProperties::Opacity,
                                               s->transparency());
        d->currentNode->properties.setProperty(KoSvgTextProperties::Visibility,
                                               s->isVisible());
        if (!s->inheritPaintOrder()) {
            d->currentNode->properties.setProperty(KoSvgTextProperties::PaintOrder,
                                                   QVariant::fromValue(s->paintOrder()));
        }
    }
}

void KoSvgTextLoader::setTextPathOnCurrentNode(KoShape *s)
{
    if (!KisForestDetail::isEnd(d->currentNode) && s) {
        if (s->name().isEmpty()) {
            int count = 0;
            while(s->name().isEmpty()) {
                QString newName("path"+QString::number(count));
                bool nameIsUnique = true;
                Q_FOREACH(KoShape *shape, d->shape->d->textPaths) {
                    if (shape->name() == newName) {
                        nameIsUnique = false;
                    }
                }
                if(nameIsUnique) {
                    s->setName(newName);
                }
                count+=1;
            }
        }
        d->currentNode->textPathId = s->name();
        s->addDependee(d->shape);
        d->shape->d->textPaths.append(s);
    }
}

