//
// Created by robin on 10/30/2024.
//

#ifndef SLIPPYMAPLAYERMARKERPROPERTYPAGE_H
#define SLIPPYMAPLAYERMARKERPROPERTYPAGE_H

#include "SlippyMapLayerObjectPropertyPage.h"

class QLineEdit;
class QTabWidget;
class QLabel;
class QPlainTextEdit;
class QCheckBox;


class SlippyMapLayerMarkerPropertyPage : public SlippyMapLayerObjectPropertyPage {
    Q_OBJECT
public:
    explicit SlippyMapLayerMarkerPropertyPage(SlippyMapLayerObject *object);

private:
    void setupUi();

    QLineEdit *m_name;
    QLineEdit *m_latitude;
    QLineEdit *m_longitude;
    QLineEdit *m_radius;
    QLineEdit *m_color;
    QCheckBox *m_visibility;
    QPlainTextEdit *m_description;
    QTabWidget *m_tabWidget;
};



#endif //SLIPPYMAPLAYERMARKERPROPERTYPAGE_H
