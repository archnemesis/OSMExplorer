//
// Created by Robin on 11/9/2024.
//

#ifndef OSMEXPLORER_SLIPPYMAPLAYEROBJECTATTACHMENTSPROPERTYPAGE_H
#define OSMEXPLORER_SLIPPYMAPLAYEROBJECTATTACHMENTSPROPERTYPAGE_H

#include <SlippyMap/SlippyMapLayerObjectPropertyPage.h>
#include <SlippyMap/SlippyMapLayerObject.h>


using namespace SlippyMap;


class QLabel;
class QListWidget;
class QPushButton;
class QLineEdit;

class SlippyMapLayerObjectAttachmentsPropertyPage : public SlippyMapLayerObjectPropertyPage
{
    Q_OBJECT
public:
    explicit SlippyMapLayerObjectAttachmentsPropertyPage(const SlippyMapLayerObject::Ptr& object);

protected:

};


#endif //OSMEXPLORER_SLIPPYMAPLAYEROBJECTATTACHMENTSPROPERTYPAGE_H
