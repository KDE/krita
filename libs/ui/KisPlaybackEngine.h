#ifndef KISPLAYBACKENGINE_H
#define KISPLAYBACKENGINE_H

#include <QObject>
#include "KoCanvasObserverBase.h"
#include <kritaui_export.h>

#include <QScopedPointer>


enum PlaybackMode {
    PUSH,
    PULL
};

class KRITAUI_EXPORT KisPlaybackEngine : public QObject, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    explicit KisPlaybackEngine(QObject *parent = nullptr);
    ~KisPlaybackEngine();

    /** @brief leaseHandle method
     * Used by canvas to hold onto a handle that communicates to the KisPlaybackEngine
     * so that playback settings can be changed on the fly.
     */
    QSharedPointer<class KisPlaybackHandle> leaseHandle(class KoCanvasBase* canvas);
    void returnHandle(KoCanvasBase* canvas);

Q_SIGNALS:
    void sigChangeActiveCanvasFrame(int p_frame);

protected Q_SLOTS:
    void setCanvas(KoCanvasBase* canvas) override;
    void unsetCanvas() override;

private:
    void setupPlaybackMode(PlaybackMode p_mode);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISPLAYBACKENGINE_H
