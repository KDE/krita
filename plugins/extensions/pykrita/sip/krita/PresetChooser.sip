class PresetChooser : public QWidget /NoDefaultCtors/
{
%TypeHeaderCode
#include "PresetChooser.h"
%End

public:
    PresetChooser(QWidget *parent = 0);

public Q_SLOTS:

    void setCurrentPreset(Resource *resource);
    Resource *currentPreset() const /Factory/;

Q_SIGNALS:

    void presetSelected(Resource resource) /Factory/;
    void presetClicked(Resource resource) /Factory/;

};
