#include "aprsfiexplorerplugin.h"
#include "aprsdotficonfigurationdialog.h"
#include "aprsfilocationdataprovider.h"
#include "slippymaplayermarker.h"
#include "aprsstationlistform.h"

#include <QDebug>
#include <QSettings>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QByteArray>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

AprsFiExplorerPlugin::AprsFiExplorerPlugin(QObject *parent) :
    QObject (parent),
    m_net (new QNetworkAccessManager())
{
    connect(&m_requestTimer, &QTimer::timeout, this, &AprsFiExplorerPlugin::onRequestTimerUpdate);
    connect(m_net, &QNetworkAccessManager::finished, this, &AprsFiExplorerPlugin::onNetworkRequestFinished);
}

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

QList<QMenu *> AprsFiExplorerPlugin::mainMenuList()
{
    return QList<QMenu*>();
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

QList<SlippyMapLayer *> AprsFiExplorerPlugin::layers()
{
    if (m_layer == nullptr) {
        m_layer = new SlippyMapLayer();
        m_layer->setName(tr("APRS.fi"));
        m_layer->setDescription(tr("APRS.fi points of interest"));
        m_layer->setVisible(true);
    }

    QList<SlippyMapLayer*> list;
    list << m_layer;
    return list;
}

void AprsFiExplorerPlugin::loadConfiguration()
{
    QSettings settings;

    if (settings.contains("integrations/aprs.fi/apikey")) {
        m_apiUrl = settings.value("integrations/aprs.fi/apiUrl").toString();
        m_apiKey = settings.value("integrations/aprs.fi/apiKey").toString();
        m_updateInterval = settings.value("integrations/aprs.fi/updateInterval").toInt();

        m_callsigns.clear();

        if (m_apiUrl.length() > 0 && m_apiKey.length() > 0) {
            int size = settings.beginReadArray("integrations/aprs.fi/callsigns");
            for (int i = 0; i < size; i++) {
                settings.setArrayIndex(i);
                m_callsigns.append(settings.value("callsign").toString());
            }

            m_requestTimer.setInterval(m_updateInterval * 60 * 1000);
            m_requestTimer.setSingleShot(false);
            m_requestTimer.start();

            qDebug() << "Timer started";

            onRequestTimerUpdate();
        }
    }
}

void AprsFiExplorerPlugin::onNetworkRequestFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Unable to fetch APRS data!";
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();

    qDebug() << "Got response:" << data;

    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qCritical() << "JSON Parse Error:" << error.errorString();
        return;
    }

    QJsonObject root = document.object();

    if (!root.contains("result") || root["result"].toString() != "ok") {
        qCritical() << "Request Error!";
        return;
    }

    QJsonArray entries = root["entries"].toArray();

    qDebug() << "Processing" << entries.count() << "records:";

    for (int i = 0; i < entries.count(); i++) {
        QJsonObject entry = entries[i].toObject();
        double lat = entry["lat"].toString().toDouble();
        double lon = entry["lng"].toString().toDouble();
        QPointF point(lon, lat);
        QString ident = entry["name"].toString();

        QHash<QString,QVariant> meta;

        if (entry.contains("class"))
            meta["aprs.fi_class"] = entry["class"].toVariant();
        if (entry.contains("type"))
            meta["aprs.fi_type"] = entry["type"].toVariant();
        if (entry.contains("time"))
            meta["aprs.fi_time"] = entry["time"].toVariant();
        if (entry.contains("lasttime"))
            meta["aprs.fi_lasttime"] = entry["lasttime"].toVariant();
        if (entry.contains("symbol"))
            meta["aprs.fi_symbol"] = entry["symbol"].toVariant();
        if (entry.contains("srccall"))
            meta["aprs.fi_srccall"] = entry["srccall"].toVariant();
        if (entry.contains("dstcall"))
            meta["aprs.fi_dstcall"] = entry["dstcall"].toVariant();
        if (entry.contains("phg"))
            meta["aprs.fi_phg"] = entry["phg"].toVariant();
        if (entry.contains("comment"))
            meta["aprs.fi_comment"] = entry["comment"].toVariant();
        if (entry.contains("path"))
            meta["aprs.fi_path"] = entry["path"].toVariant();

        QStringList info;

        for (QString key : meta.keys()) {
            info << QString("%1: %2").arg(key).arg(meta[key].toString());
        }

        QString infoString = info.join("<br/>");

        SlippyMapLayerMarker *marker;

        if (m_objects.contains(ident)) {
            qDebug() << "Updating marker...";
            marker = dynamic_cast<SlippyMapLayerMarker*>(m_objects[ident]);
            marker->setPosition(point);
            marker->setName(ident);
            marker->setDescription(infoString);
            marker->setVisible(true);
        }
        else {
            qDebug() << "Creating marker...";
            marker = new SlippyMapLayerMarker(point);
            marker->setName(ident);
            marker->setDescription(infoString);
            marker->setEditable(false);
            marker->setColor(Qt::red);
            marker->setMovable(false);
            marker->setVisible(true);
            m_objects[ident] = static_cast<SlippyMapLayerObject*>(marker);
            m_layer->addObject(marker);
        }
    }
}

void AprsFiExplorerPlugin::onRequestTimerUpdate()
{
    qDebug() << "Timer timeout!";

    if (m_callsigns.size() == 0) return;

    QString callsignList = m_callsigns.join(",");
    QString url = QString("%1/get?name=%2&what=loc&apikey=%3&format=json")
            .arg(m_apiUrl)
            .arg(callsignList)
            .arg(m_apiKey);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "OSMExplorer/1.0.0alpha1 (+https://github.com/archnemesis/OSMExplorer)");

    qDebug() << "Requesting data from:" << url;

    m_net->get(req);
}
