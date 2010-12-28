#ifndef KOTOSCONTAINERMODEL_H
#define KOTOSCONTAINERMODEL_H

class KoTosContainerModel : public KoShapeContainerModel
{
public:
    KoTosContainerModel();
    ~KoTosContainerModel();

    virtual void add(KoShape *shape);
    virtual void remove(KoShape *shape);
    virtual void setClipped(const KoShape *shape, bool clipping);
    virtual bool isClipped(const KoShape *shape) const;
    virtual void setInheritsTransform(const KoShape *shape, bool inherit);
    virtual bool inheritsTransform(const KoShape *shape) const;
    virtual bool isChildLocked(const KoShape *child) const;
    virtual int count() const;
    virtual QList<KoShape*> shapes() const;
    virtual void containerChanged(KoShapeContainer *container, KoShape::ChangeType type);

private:
    KoShape *m_textShape;
};

#endif /* KOTOSCONTAINERMODEL_H */
