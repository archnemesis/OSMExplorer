//
// Created by robin on 10/4/24.
//

#ifndef PANELAPP_RAINVIEWERSLIPPYMAPTILELAYER_H
#define PANELAPP_RAINVIEWERSLIPPYMAPTILELAYER_H

#include <SlippyMap/SlippyMapWidgetLayer.h>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
QT_END_NAMESPACE

using namespace SlippyMap;

class RainViewerSlippyMapTileLayer : public SlippyMapWidgetLayer
{
    Q_OBJECT
public:
    explicit RainViewerSlippyMapTileLayer(QObject *parent = nullptr);
    bool isVisible() override;
    void update();
    void nextFrame();
    void previousFrame();

    private slots:
        void nam_onFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_nam;
    bool m_ready;
    int m_currentFrame{};
    QList<QString> m_radarFrameUrls;
};


#endif //PANELAPP_RAINVIEWERSLIPPYMAPTILELAYER_H