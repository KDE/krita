#ifndef KIS_SPLAT_GENERATOR_STRATEGY_H
#define KIS_SPLAT_GENERATOR_STRATEGY_H

#include <KoRTree.h>
#include "kis_splat.h"
#include "kis_wetmap.h"

class KisSplatGeneratorStrategy
{
public:
    virtual ~KisSplatGeneratorStrategy(){}
    virtual void generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, KoColor color) = 0;

};

class KisSimpleBrushGenerator : public KisSplatGeneratorStrategy
{
public:
    void generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, KoColor color) override;
};

class KisWetOnDryGenerator : public KisSplatGeneratorStrategy
{
public:
    void generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, KoColor color) override;
};

class KisCrunchyGenerator : public KisSplatGeneratorStrategy
{
public:
    void generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, KoColor color) override;
};

class KisWetOnWetGenerator : public KisSplatGeneratorStrategy
{
public:
    void generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, KoColor color) override;
};

class KisBlobbyGenerator : public KisSplatGeneratorStrategy
{
public:
    void generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, KoColor color) override;
};

#endif // KIS_SPLAT_GENERATOR_STRATEGY_H
