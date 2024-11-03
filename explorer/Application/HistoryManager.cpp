//
// Created by Robin on 11/2/2024.
//

#include "HistoryManager.h"

#include <QDebug>

HistoryManager::HistoryManager(QObject *parent) :
    QObject(parent),
    m_historyIndex(-1)
{

}

void HistoryManager::addEvent(const HistoryManager::HistoryEvent &event)
{
    //
    // if we go to add an event and we're not at the top of the
    // stack, then we delete everything ahead and make this the
    // last entry.
    //
    if ((m_historyIndex + 1) < m_historyStack.count()) {
        //
        // first we need to make sure that we clean up orphaned
        // objects in the future
        //
        for (int i = (m_historyIndex + 1); i < m_historyStack.count(); i++) {
            const HistoryEvent& futureEvent = m_historyStack.at(i);
            // the user undid an add object, but the object still
            // exists in memory, but we delete it because we're deleting
            // the future
            if (futureEvent.action == AddObject)
                delete futureEvent.original;
            else if (futureEvent.action == ModifyObject)
                delete futureEvent.copy;
            else if (futureEvent.action == AddLayer)
                delete futureEvent.layer;

        }

        m_historyStack.remove(m_historyIndex + 1, m_historyStack.count() - (m_historyIndex + 1));
        emit redoHistoryCleared();
    }

    m_historyStack.push(event);
    m_historyIndex++;
    emit undoEventAdded(event);
}

void HistoryManager::addEvent(const QString& description, SlippyMapLayerObject *object)
{
    HistoryEvent event;
    event.description = description;
    event.original = object;
    event.copy = object->clone();

    addEvent(event);
}

HistoryManager::HistoryEvent HistoryManager::undoEvent()
{
    return m_historyStack.at(m_historyIndex--);
}

HistoryManager::HistoryEvent HistoryManager::redoEvent()
{
    return m_historyStack.at(++m_historyIndex);
}

int HistoryManager::count() const
{
    return m_historyStack.count();
}

int HistoryManager::undoCount() const
{
    return m_historyIndex + 1;
}

int HistoryManager::redoCount() const
{
    if ((m_historyIndex + 1) == count()) return 0;
    else return count() - (m_historyIndex + 1);
}

QString HistoryManager::currentUndoDescription()
{
    Q_ASSERT(m_historyStack.count() > 0);
    Q_ASSERT(m_historyIndex < m_historyStack.count());

    return m_historyStack.at(m_historyIndex).description;
}

QString HistoryManager::currentRedoDescription()
{
    Q_ASSERT(m_historyStack.count() > 0);
    Q_ASSERT((m_historyIndex + 1) < m_historyStack.count());

    return m_historyStack.at(m_historyIndex + 1).description;
}
