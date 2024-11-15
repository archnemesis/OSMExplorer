//
// Created by Robin on 11/6/2024.
//

#include "TabbedMainWindow.h"
#include "SlippyMapTab.h"

#include <QDebug>
#include <QTabWidget>
#include <QHBoxLayout>

#include <SlippyMap/SlippyMapWidget.h>


using namespace Explorer;

TabbedMainWindow::TabbedMainWindow(QMainWindow *parent) :
    QMainWindow(parent)
{
    setupUi();
    setupFirstTab();
}

void Explorer::TabbedMainWindow::setupUi()
{
    m_tabWidget = new QTabWidget();
    m_tabWidget->setTabsClosable(true);

    connect(m_tabWidget,
            &QTabWidget::tabCloseRequested,
            this,
            &TabbedMainWindow::onTabWidgetTabCloseRequested);

    setCentralWidget(m_tabWidget);
}

void TabbedMainWindow::setupFirstTab()
{
    addTab(tr("New Workspace"));
}

void TabbedMainWindow::addTab(const QString &title)
{
    auto *tab = new SlippyMapTab();
    m_slippyTabs.append(tab);
    m_tabWidget->addTab(tab, title);

    connect(tab->slippyMap(),
            &SlippyMap::SlippyMapWidget::centerChanged,
            this,
            &TabbedMainWindow::onSlippyMapCenterChanged);
}

void TabbedMainWindow::onSlippyMapCenterChanged(double latitude, double longitude)
{
    auto *mapWidget = qobject_cast<SlippyMap::SlippyMapWidget*>(sender());
    qDebug() << "Map center changed:" << latitude, longitude;
}

void TabbedMainWindow::removeTab(SlippyMapTab *tab)
{
    int index = m_slippyTabs.indexOf(tab);
    m_tabWidget->removeTab(index);
    m_slippyTabs.removeOne(tab);
    tab->deleteLater();
}

void TabbedMainWindow::onTabWidgetTabCloseRequested(int index)
{
    m_tabWidget->removeTab(index);
    m_slippyTabs.removeAt(index);
}
