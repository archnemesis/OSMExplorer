#include "aprsfiexplorerplugin.h"
#include "aprsdotficonfigurationdialog.h"
#include "aprsfilocationdataprovider.h"

#include <QSettings>

AprsFiExplorerPlugin::AprsFiExplorerPlugin()
{
    m_markerGroup = new MapMarkerModel::MarkerGroup("aprs.fi");
}

QString AprsFiExplorerPlugin::name() const
{
    return "aprs.fi";
}

QString AprsFiExplorerPlugin::description() const
{
    return "aprs.fi integration (http://aprs.fi)";
}

QList<QAction *> AprsFiExplorerPlugin::mapContextMenuActionList()
{
    return QList<QAction *>();
}

QList<MapMarkerModel::MarkerGroup *> AprsFiExplorerPlugin::markerGroupList()
{
    QList<MapMarkerModel::MarkerGroup *> list;
    list.append(m_markerGroup);
    return list;
}

QDialog *AprsFiExplorerPlugin::configurationDialog(QWidget *parent)
{
    AprsDotFiConfigurationDialog *dlg =
            new AprsDotFiConfigurationDialog(parent);
    return dlg;
}

QList<LocationDataProvider *> AprsFiExplorerPlugin::locationDataProviderList()
{
    QSettings settings;
    QList<LocationDataProvider *> providers;

    if (settings.contains("integrations/aprs.fi/apikey")) {
        if (m_dataProvider == nullptr) {
            m_dataProvider = new AprsFiLocationDataProvider();
        }

        QString apiUrl = settings.value("integrations/aprs.fi/apiUrl").toString();
        QString apiKey = settings.value("integrations/aprs.fi/apiKey").toString();
        int updateInterval = settings.value("integrations/aprs.fi/updateInterval").toInt();

        QStringList callsigns;
        int size = settings.beginReadArray("integrations/aprs.fi/callsigns");
        for (int i = 0; i < size; i++) {
            settings.setArrayIndex(i);
            callsigns.append(settings.value("callsign").toString());
        }

        m_dataProvider->setApiUrl(apiUrl);
        m_dataProvider->setApiKey(apiKey);
        m_dataProvider->setUpdateInterval(updateInterval);
        m_dataProvider->setCallsigns(callsigns);

        providers.append(m_dataProvider);

        connect(
            m_dataProvider,
            &LocationDataProvider::positionUpdated,
            this,
            &AprsFiExplorerPlugin::dataProviderPositionUpdated);
    }

    return providers;
}

void AprsFiExplorerPlugin::dataProviderPositionUpdated(QString identifier, QPointF position, QHash<QString, QVariant> metadata)
{
    for (SlippyMapWidgetMarker *marker : m_markerGroup->markers()) {
        if (marker->label() == identifier) {
            marker->setPosition(position);
            return;
        }
    }

    m_markerGroup->addMarker(
                new SlippyMapWidgetMarker(
                    position,
                    identifier));
}
