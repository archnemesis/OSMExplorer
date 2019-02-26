#include "aprsfiexplorerplugin.h"
#include "aprsdotficonfigurationdialog.h"
#include "aprsfilocationdataprovider.h"
#include "slippymapwidgetmarkergroup.h"
#include "aprsstationlistform.h"

#include <QSettings>

QString AprsFiExplorerPlugin::name() const
{
    return "aprs.fi";
}

QString AprsFiExplorerPlugin::description() const
{
    return "aprs.fi integration (http://aprs.fi)";
}

QString AprsFiExplorerPlugin::authorName() const
{
    return "Robin Gingras <robin@robingingras.com>";
}

QString AprsFiExplorerPlugin::homepage() const
{
    return "http://www.robingingras.com/osmexplorer/plugins/aprs.fi/";
}

QList<QAction *> AprsFiExplorerPlugin::mapContextMenuActionList()
{
    return QList<QAction *>();
}

QList<QDockWidget *> AprsFiExplorerPlugin::dockWidgetList()
{
    QList<QDockWidget *> ret;

    QDockWidget *stationList = new QDockWidget();
    stationList->setWindowTitle(tr("APRS Stations"));
    stationList->setWidget(new AprsStationListForm());
    ret.append(stationList);

    return ret;
}

QList<SlippyMapWidgetMarker *> AprsFiExplorerPlugin::markerList()
{
    return QList<SlippyMapWidgetMarker*>();
}

QDialog *AprsFiExplorerPlugin::configurationDialog(QWidget *parent)
{
    AprsDotFiConfigurationDialog *dlg =
            new AprsDotFiConfigurationDialog(parent);

    /* Reload configuration after the dialog is accepted */
    connect(
                dlg,
                &AprsDotFiConfigurationDialog::accepted,
                [=](){
        loadConfiguration();
    });

    return dlg;
}

void AprsFiExplorerPlugin::dataProviderPositionUpdated(QString identifier, QPointF position, QHash<QString, QVariant> metadata)
{
    (void)identifier;
    (void)position;
    (void)metadata;
}

void AprsFiExplorerPlugin::loadConfiguration()
{
    QSettings settings;

    if (m_dataProvider == nullptr) {
        m_dataProvider = new AprsFiLocationDataProvider();
        connect(
            m_dataProvider,
            &AprsFiLocationDataProvider::positionUpdated,
            this,
            &AprsFiExplorerPlugin::dataProviderPositionUpdated);
    }

    if (settings.contains("integrations/aprs.fi/apikey")) {
        QString apiUrl = settings.value("integrations/aprs.fi/apiUrl").toString();
        QString apiKey = settings.value("integrations/aprs.fi/apiKey").toString();
        int updateInterval = settings.value("integrations/aprs.fi/updateInterval").toInt();

        if (apiUrl.length() > 0 && apiKey.length() > 0) {
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
            m_dataProvider->start();
        }
        else {
            m_dataProvider->stop();
            m_dataProvider->setApiUrl("");
            m_dataProvider->setApiKey("");
            m_dataProvider->setCallsigns(QStringList());
        }
    }
}
