//
// Created by Robin on 11/3/2024.
//

#ifndef OSMEXPLORER_DATABASEOBJECTPROPERTYPAGE_H
#define OSMEXPLORER_DATABASEOBJECTPROPERTYPAGE_H

#include <SlippyMap/SlippyMapLayerObjectPropertyPage.h>

class QLineEdit;
class QLabel;

class DatabaseObjectPropertyPage : public SlippyMapLayerObjectPropertyPage
{
    Q_OBJECT
public:
    explicit DatabaseObjectPropertyPage(SlippyMap::SlippyMapLayerObject::Ptr object);
    QString tabTitle() override;
    void setupUi() override;
    void save() override;

public slots:
    void updateUi() override;

private:
    QLineEdit *m_id;
    QLabel *m_syncLabel;
};


#endif //OSMEXPLORER_DATABASEOBJECTPROPERTYPAGE_H
