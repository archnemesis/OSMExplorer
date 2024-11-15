//
// Created by Robin on 11/7/2024.
//

#include "SlippyMapTab.h"
#include <QHBoxLayout>
#include <SlippyMap/SlippyMapWidget.h>
#include <SlippyMap/SlippyMapWidgetLayer.h>
#include <SlippyMap/SlippyMapLayerManager.h>

namespace Explorer
{
    SlippyMapTab::SlippyMapTab(QWidget *parent) : QWidget(parent)
    {
        m_slippyMap = new SlippyMap::SlippyMapWidget();

        setupUi();
        setupSlippyMap();
    }

    void SlippyMapTab::setupUi()
    {
        auto *layout = new QHBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_slippyMap);
        setLayout(layout);
    }

    void SlippyMapTab::setupSlippyMap()
    {
        m_layerManager = new SlippyMap::SlippyMapLayerManager();
        m_slippyMap->setLayerManager(m_layerManager);

        auto *layer = new SlippyMap::SlippyMapWidgetLayer();
        layer->setName("OpenStreetMaps");
        layer->setDescription("Example OpenStreetMaps layer");
        layer->setTileUrl("https://tile.openstreetmap.org/%1/%2/%3.png");
        m_slippyMap->addLayer(layer);

    }

    SlippyMap::SlippyMapWidget *SlippyMapTab::slippyMap()
    {
        return m_slippyMap;
    }
} // Explorer