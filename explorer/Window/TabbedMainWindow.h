//
// Created by Robin on 11/6/2024.
//

#ifndef OSMEXPLORER_TABBEDMAINWINDOW_H
#define OSMEXPLORER_TABBEDMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QTabWidget;
QT_END_NAMESPACE

namespace Explorer
{
    class SlippyMapTab;

    class TabbedMainWindow : public QMainWindow
    {
    Q_OBJECT
    public:
        explicit TabbedMainWindow(QMainWindow *parent = nullptr);

    protected:
        void setupUi();
        void setupFirstTab();
        void addTab(const QString& title);
        void removeTab(SlippyMapTab *tab);

        QTabWidget *m_tabWidget = nullptr;
        QList<SlippyMapTab*> m_slippyTabs;

    protected slots:
        void onSlippyMapCenterChanged(double latitude, double longitude);
        void onTabWidgetTabCloseRequested(int index);
    };
}


#endif //OSMEXPLORER_TABBEDMAINWINDOW_H
