
#ifndef KIS_MULTI_PAINT_DEVICE_IMPL_H_
#define KIS_MULTI_PAINT_DEVICE_IMPL_H_

#include <QObject>
#include <QRect>
#include <QVector>

#include "kis_paint_device.h"

#include <krita_export.h>

/** KisMultiPaintDevice is a version of KisPaintDevice capable of storing
 *  multiple different contents and switching between them.
 */

class KRITAIMAGE_EXPORT KisMultiPaintDevice
        : public KisPaintDevice
{
    Q_OBJECT

public:
    KisMultiPaintDevice(const KoColorSpace * colorSpace, const QString& name = QString());
    KisMultiPaintDevice(KisNodeWSP parent, const KoColorSpace * colorSpace, KisDefaultBoundsBaseSP defaultBounds = 0, const QString& name = QString());

    KisMultiPaintDevice(const KisPaintDevice& rhs);

    ~KisMultiPaintDevice();

    /**
     * Create a new context.
     *
     * Note: the current context of the device remains unchanged.
     * Use switchContext(id) if needed.
     *
     * @return id of new context
     */
    int newContext();

    /**
     * Create a new context as a copy of the given context. The two will
     * initially contain the same data, but modifications are not shared.
     *
     * Note: the current context of the device remains unchanged.
     * Use switchContext(id) if needed.
     *
     * @param parentId context identifier of the copied context
     * @return id of new context
     */
    int newContext(int parentId);

    /**
     * Switch to a different context using an id from newContext().
     * @param id context identifier of the context to switch to
     */
    void switchContext(int id);

    /**
     * Remove the context from saved contexts.
     *
     * If the context is currently active, the device will switch to
     * another context.
     *
     * However, if the context is the only remaining context, this
     * function will do nothing.
     *
     * @param id context identifier of the context to drop
     */
    void dropContext(int id);

    /**
     * Get the id of the current context.
     */
    int currentContext();

private:
    struct Private;
    Private * const m_d;

    struct Context;
    Context *createContext(KisDataManagerSP dataManager);

    void init();
};

#endif
