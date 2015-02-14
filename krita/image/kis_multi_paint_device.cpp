
#include "kis_multi_paint_device.h"

#include "kis_node.h"
#include "kis_datamanager.h"

struct KisMultiPaintDevice::Context {
    int id;
    KisDataManagerSP dataManager;

    Context(int id, KisDataManagerSP dataManager) : id(id), dataManager(dataManager) {}
};

struct KisMultiPaintDevice::Private {
    QHash<int, Context*> storedContexts;
    Context *currentContext;

    int nextFreeId;

    Private() : currentContext(0), nextFreeId(1) {}
};

KisMultiPaintDevice::KisMultiPaintDevice(const KoColorSpace *colorSpace, const QString &name)
    : KisPaintDevice(colorSpace, name)
    , m_d(new Private())
{
    init();
}

KisMultiPaintDevice::KisMultiPaintDevice(KisNodeWSP parent, const KoColorSpace *colorSpace, KisDefaultBoundsBaseSP defaultBounds, const QString &name)
    : KisPaintDevice(parent, colorSpace, defaultBounds, name)
    , m_d(new Private())
{
    init();
}

KisMultiPaintDevice::KisMultiPaintDevice(const KisPaintDevice &rhs)
    : KisPaintDevice(rhs)
    , m_d(new Private())
{
    init();
}

KisMultiPaintDevice::~KisMultiPaintDevice()
{
    QHash<int, Context*>::const_iterator it = m_d->storedContexts.constBegin();
    while (it != m_d->storedContexts.constEnd()) {
        delete it.value();
        ++it;
    }

    delete m_d;
}

void KisMultiPaintDevice::init()
{
    m_d->currentContext = createContext(dataManager());
}

int KisMultiPaintDevice::newContext()
{
    return createContext(new KisDataManager(pixelSize(), defaultPixel()))->id;
}

int KisMultiPaintDevice::newContext(int parentId)
{
    Context *parentContext = m_d->storedContexts.value(parentId, 0);
    KisDataManagerSP newDataManager = new KisDataManager(*parentContext->dataManager);
    return createContext(newDataManager)->id;
}

KisMultiPaintDevice::Context * KisMultiPaintDevice::createContext(KisDataManagerSP dataManager)
{
    int id = m_d->nextFreeId++;
    Context *newContext = new Context(id, dataManager);

    m_d->storedContexts.insert(id, newContext);

    return newContext;
}

void KisMultiPaintDevice::switchContext(int id)
{
    Context *context = m_d->storedContexts.value(id, 0);
    if (context == 0) return;

    m_d->currentContext = context;
    setDataManager(context->dataManager);
}

void KisMultiPaintDevice::dropContext(int id)
{
    // TODO
}

int KisMultiPaintDevice::currentContext()
{
    return m_d->currentContext->id;
}


#include "kis_multi_paint_device.moc"
