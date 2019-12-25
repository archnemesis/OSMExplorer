#ifndef SLIPPYMAPLAYERMARKERPROPERTYPAGE_H
#define SLIPPYMAPLAYERMARKERPROPERTYPAGE_H

#include "slippymaplayerobjectpropertypage.h"

class QPlainTextEdit;
class QLineEdit;
class SlippyMapLayerObject;

class SlippyMapLayerMarkerPropertyPage : public SlippyMapLayerObjectPropertyPage
{
    Q_OBJECT
public:
    SlippyMapLayerMarkerPropertyPage(QWidget *parent = nullptr);
    SlippyMapLayerMarkerPropertyPage(SlippyMapLayerObject *object, QWidget *parent = nullptr);
    virtual QString tabTitle();
protected:
    QLineEdit *m_lneX;
    QLineEdit *m_lneY;
    QLineEdit *m_lneTitle;
    QPlainTextEdit *m_pteDescription;
};

#endif // SLIPPYMAPLAYERMARKERPROPERTYPAGE_H
