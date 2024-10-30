#include "SlippyMapLayerObjectPropertyPage.h"


SlippyMapLayerObjectPropertyPage::SlippyMapLayerObjectPropertyPage(SlippyMapLayerObject *object)
{
    m_object = object;
}

SlippyMapLayerObjectPropertyPage::~SlippyMapLayerObjectPropertyPage()
{

}

QString SlippyMapLayerObjectPropertyPage::tabTitle()
{
    if (m_object)
        return m_object->label();
}
