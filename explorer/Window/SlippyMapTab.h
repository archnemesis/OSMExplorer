//
// Created by Robin on 11/7/2024.
//

#ifndef OSMEXPLORER_SLIPPYMAPTAB_H
#define OSMEXPLORER_SLIPPYMAPTAB_H

#include <QWidget>

namespace SlippyMap
{
    class SlippyMapWidget;
    class SlippyMapLayerManager;
}

namespace Explorer
{

    class SlippyMapTab : public QWidget
    {
        Q_OBJECT
    public:
        explicit SlippyMapTab(QWidget *parent = nullptr);

        SlippyMap::SlippyMapWidget *slippyMap();

    protected:
        void setupUi();
        void setupSlippyMap();

        SlippyMap::SlippyMapWidget *m_slippyMap;
        SlippyMap::SlippyMapLayerManager *m_layerManager;
    };

} // Explorer

#endif //OSMEXPLORER_SLIPPYMAPTAB_H
