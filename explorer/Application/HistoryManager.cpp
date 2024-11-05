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
        m_historyStack.remove(m_historyIndex + 1, m_historyStack.count() - (m_historyIndex + 1));
        emit redoHistoryCleared();
    }

    m_historyStack.push(event);
    m_historyIndex++;
    emit undoEventAdded(event);
}

void HistoryManager::addEvent(const QString& description, SlippyMapLayerObject::Ptr object)
{
    HistoryEvent event;
    event.description = description;
    event.original = object;
    event.copy = SlippyMapLayerObject::Ptr(object->clone());

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

void HistoryManager::clearUndoHistory()
{
    for (const auto& event: m_historyStack) {
        switch (event.action) {
            // delete entries carry the object that was deleted
            // in case the user decides to undo it, so remove them
            // for good

            case HistoryManager::DeleteObject:
                break;
        }
    }
}
