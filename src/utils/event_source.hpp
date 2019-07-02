#pragma once


template <typename EvtDesc>
EventSource<EvtDesc>::EventSource()
: m_currentId(0)
{
    for (int i = 0; i < std::tuple_size<EvtDesc>::value; ++i)
        m_silent[i] = false;
}

template <typename EvtDesc>
void EventSource<EvtDesc>::Mute(EventIdT id)
{
    for (auto &[k, v] : m_silent)
        if (id == InvalidEventT || k == id) {
            v = true;
        }
}

template <typename EvtDesc>
void EventSource<EvtDesc>::Unmute(EventIdT id)
{
    for (auto &[k, v] : m_silent)
        if (id == InvalidEventT || k == id)
            v = false;
}

template <typename EvtDesc>
template <EventIdT Id, typename F> EventProxy EventSource<EvtDesc>::Subscribe(const F &f)
{
    auto &cbs = std::get<Id>(m_callbacks);
    cbs[m_currentId] = f;

    return EventProxy { Id, m_currentId++ };
}

template <typename EvtDesc>
template <EventIdT Id> void EventSource<EvtDesc>::Unsubscribe(const EventProxy &ep)
{
    auto &cbs = std::get<Id>(m_callbacks);
    cbs.erase(ep.ConnectionId);
}

template <typename EvtDesc>
template <EventIdT Id, typename... Args> void EventSource<EvtDesc>::EmitEvent(Args &&... args)
{
    if (m_silent[Id])
        return;

    for (auto &[coId, f] : std::get<Id>(m_callbacks))
        f(args...);
}

template <typename T>
EventMute<T>::EventMute(T *obj, EventIdT id)
: EventMute(obj, std::vector<EventIdT>{id})
{
}

template <typename T>
EventMute<T>::EventMute(T *obj, std::vector<EventIdT> ids)
: m_obj(obj), m_ids(ids)
{
    for (auto id : m_ids)
        m_obj->Mute(id);
}

template <typename T>
EventMute<T>::~EventMute()
{
    for (auto id : m_ids)
        m_obj->Unmute(id);
}
