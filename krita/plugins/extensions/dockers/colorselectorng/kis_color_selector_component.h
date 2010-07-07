#ifndef KIS_COLOR_SELECTOR_COMPONENT_H
#define KIS_COLOR_SELECTOR_COMPONENT_H

#include <QWidget>

class KoColorSpace;
class KisColorSelectorBase;

class KisColorSelectorComponent : public QWidget
{
    Q_OBJECT
public:
    explicit KisColorSelectorComponent(KisColorSelectorBase* parent);
protected:
    const KoColorSpace* colorSpace() const;
    
private:
    KisColorSelectorBase* m_parent;

};

#endif // KIS_COLOR_SELECTOR_COMPONENT_H
