#ifndef KISGAMUTMASKCHOOSER_H
#define KISGAMUTMASKCHOOSER_H

#include <QWidget>

class KoResourceItemChooser;
class KoResource;
class KoGamutMask;

class KisGamutMaskChooser : public QWidget
{
    Q_OBJECT
public:
    explicit KisGamutMaskChooser(QWidget *parent = nullptr);
    ~KisGamutMaskChooser() override;

Q_SIGNALS:
    void sigGamutMaskSelected(KoGamutMask* mask);

private Q_SLOTS:
    void resourceSelected(KoResource* resource);

private:
    KoResourceItemChooser* m_itemChooser;
};

#endif // KISGAMUTMASKCHOOSER_H
