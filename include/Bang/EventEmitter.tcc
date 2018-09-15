#pragma once

#include "Bang/EventEmitter.h"

USING_NAMESPACE_BANG

template<class T>
EventEmitter<T>::~EventEmitter()
{
    ++m_iterationDepth;
    for (EventListener<T> *listener : GetListeners())
    {
        if (listener)
        {
            UnRegisterListener(listener);
        }
    }
}

template<class T>
void EventEmitter<T>::SetEmitEvents(bool emitEvents)
{
    m_emitEvents = emitEvents;
}

template<class T>
bool EventEmitter<T>::IsEmittingEvents() const
{
    return m_emitEvents;
}

template <class T>
void EventEmitter<T>::RegisterListener(EventListener<T> *listener)
{
    if (!m_listeners.Contains(listener))
    {
        m_listeners.PushBack(listener);
        listener->AddEmitter(this);
    }
}

template <class T>
void EventEmitter<T>::UnRegisterListener(EventListener<T> *listener)
{
    if (m_iterationDepth == 0)
    {
        m_listeners.Remove(listener);
    }
    else
    {
        MarkListenerAsDeleted(listener);
    }
    listener->RemoveEmitter(this);
}

template<class T>
void EventEmitter<T>::MarkListenerAsDeleted(EventListener<T> *listener)
{
    for (int i = 0; i < GetListeners().Size(); ++i)
    {
        if (GetListeners()[i] == listener)
        {
            GetListeners()[i] = nullptr;
        }
    }
}

template<class T>
void EventEmitter<T>::ClearDeletedListeners()
{
    ASSERT(m_iterationDepth == 0);
    GetListeners().RemoveAll(nullptr);
}

template<class T>
Array<EventListener<T>*> &EventEmitter<T>::GetListeners()
{
    return m_listeners;
}

template<class T>
void EventEmitter<T>::PropagateToListeners_(
              std::function<void(EventListener<T>*)> listenerCall) const
{
    if (GetListeners().Size() > 0 && IsEmittingEvents())
    {
        ++m_iterationDepth;

        for (int i = 0; i < GetListeners().Size(); ++i)
        {
            if (EventListener<T> *listener = GetListeners()[i])
            {
                if (listener->IsReceivingEvents())
                {
                    listenerCall(listener);
                }
            }
        }

        if (--m_iterationDepth == 0)
        {
            EventEmitter<T> *ncThis = const_cast<EventEmitter<T>*>(this);
            ncThis->ClearDeletedListeners();
        }
    }
}

template <class T>
template<class TFunction, class... Args>
void EventEmitter<T>::
PropagateToListeners(const TFunction &func, const Args&... args) const
{
    PropagateToListeners_([&](EventListener<T> *listener)
                          {
                              (listener->*func)(args...);
                          });
}

template<class T>
template<class TResult, class TFunction, class... Args>
Array<TResult>
EventEmitter<T>::PropagateToListenersAndGatherResult(const TFunction &func,
                                                     const Args&... args) const
{
    Array<TResult> gatheredResult;
    PropagateToListeners_(
             [&](EventListener<T> *listener)
             {
                 gatheredResult.PushBack( (listener->*func)(args...) );
             });
    return gatheredResult;
}

template<class T>
const Array<EventListener<T>*>& EventEmitter<T>::GetListeners() const
{
    return m_listeners;
}

