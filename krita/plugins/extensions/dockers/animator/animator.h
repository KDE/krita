#ifndef _ANIMATOR_H_
#define _ANIMATOR_H_

#include <QVariant>
#include <QObject>

class KisView2;

/**
 * The animator plugin class
 */
class AnimatorPlugin : public QObject
{
    Q_OBJECT
    public:
        AnimatorPlugin(QObject *parent, const QVariantList &);
        virtual ~AnimatorPlugin();
    private:
        KisView2* m_view;
};

#endif
