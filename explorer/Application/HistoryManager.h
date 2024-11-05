//
// Created by Robin on 11/2/2024.
//

#ifndef OSMEXPLORER_HISTORYMANAGER_H
#define OSMEXPLORER_HISTORYMANAGER_H

#include <QObject>
#include <QStack>
#include <SlippyMap/SlippyMapLayerObject.h>
#include <SlippyMap/SlippyMapLayer.h>

using namespace SlippyMap;

class HistoryManager : public QObject
{
    Q_OBJECT
public:
    enum HistoryAction {
        AddObject,
        DeleteObject,
        ModifyObject,
        AddLayer,
        DeleteLayer,
        ModifyLayer,
    };

    struct HistoryEvent {
        QString description;
        HistoryAction action;
        SlippyMapLayerObject::Ptr original;
        SlippyMapLayerObject::Ptr copy;
        SlippyMapLayer::Ptr layer;
    };

    explicit HistoryManager(QObject *parent = nullptr);
    void addEvent(const HistoryEvent& event);
    void addEvent(const QString& description, SlippyMapLayerObject::Ptr object);

    /**
     * Clears the undo history.
     */
    void clearUndoHistory();

    /**
     * Return the description text of the current undo item.
     * @return
     */
    QString currentUndoDescription();

    /**
     * Return the description of the current redo item.
     */
    QString currentRedoDescription();

    /**
     * Returns the event at our pointer and moves back in time.
     * @return
     */
    HistoryEvent undoEvent();

    /**
     * Returns the event in front of our pointer and moves forward
     * in time.
     * @return
     */
    HistoryEvent redoEvent();

    /**
     * How many actions on the stack total.
     * @return
     */
    int count() const;

    /**
     * Returns how many undo actions are on the stack.
     * @return
     */
    int undoCount() const;

    /**
     * Returns how many redo actions are on the stack.
     */
    int redoCount() const;

signals:
    void undoEventAdded(HistoryEvent event);
    void redoHistoryCleared();


protected:
    QStack<HistoryEvent> m_historyStack;
    int m_historyIndex;
    int m_historyCount;
};


#endif //OSMEXPLORER_HISTORYMANAGER_H
