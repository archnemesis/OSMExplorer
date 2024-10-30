#ifndef SLIPPYMAPSHAPEPROPERTYPAGE_H
#define SLIPPYMAPSHAPEPROPERTYPAGE_H

#include <QWidget>
#include <SlippyMap/SlippyMapLayerObject.h>

using namespace SlippyMap;

class SlippyMapLayerObjectPropertyPage : public QWidget
{
    Q_OBJECT
public:
    explicit SlippyMapLayerObjectPropertyPage(SlippyMapLayerObject *object);
    ~SlippyMapLayerObjectPropertyPage() override;
    virtual QString tabTitle();

protected:
    SlippyMapLayerObject *m_object;
};

#endif // SLIPPYMAPSHAPEPROPERTYPAGE_H
