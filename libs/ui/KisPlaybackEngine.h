#ifndef KISPLAYBACKENGINE_H
#define KISPLAYBACKENGINE_H

#include <QObject>
#include "KoCanvasObserverBase.h"
#include <kritaui_export.h>

#include <QScopedPointer>

class KRITAUI_EXPORT KisPlaybackEngine : public QObject, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    explicit KisPlaybackEngine(QObject *parent = nullptr);
    ~KisPlaybackEngine();

    QSharedPointer<class KisPlaybackHandle> leaseHandle(class KoCanvasBase* canvas);
    void returnHandle(KoCanvasBase* canvas);

protected Q_SLOTS:
    void setCanvas(KoCanvasBase* canvas) override;
    void unsetCanvas() override;

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISPLAYBACKENGINE_H
