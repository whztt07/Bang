#include "Bang/Object.h"

#include "Bang/Debug.h"

USING_NAMESPACE_BANG

Object::~Object()
{
    ASSERT( IsWaitingToBeDestroyed() );
}

bool Object::IsActive() const
{
    return IsStarted() && IsEnabled() && !IsWaitingToBeDestroyed();
}

void Object::PreStart()
{
    if (!IsStarted())
    {
        OnPreStart();
    }
}

void Object::Start()
{
    if (!IsStarted())
    {
        OnStart();
        m_started = true;
        EventEmitter<IObjectEvents>::
                PropagateToListeners(&IObjectEvents::OnStarted);
    }
}

void Object::OnPreStart() {}
void Object::OnStart() {}
void Object::OnEnabled()
{
    EventEmitter<IObjectEvents>::
            PropagateToListeners(&IObjectEvents::OnEnabled);
}
void Object::OnDisabled()
{
    EventEmitter<IObjectEvents>::
            PropagateToListeners(&IObjectEvents::OnDisabled);
}
void Object::OnDestroy() {}

void Object::DestroyObject(Object *object)
{
    if (!object->IsWaitingToBeDestroyed())
    {
        object->m_waitingToBeDestroyed = true;

        object->OnDestroy();
        object->EventEmitter<IEventsDestroy>::
                PropagateToListeners(&IEventsDestroy::OnDestroyed, object);
    }
}

const ObjectId& Object::GetObjectId() const
{
    return m_objectId;
}

void Object::SetEnabled(bool enabled)
{
    if (enabled != IsEnabled())
    {
        m_enabled = enabled;
        if (IsEnabled()) { OnEnabled(); } else { OnDisabled(); }
    }
}

bool Object::IsEnabled() const { return m_enabled; }
bool Object::IsStarted() const { return m_started; }
bool Object::IsWaitingToBeDestroyed() const { return m_waitingToBeDestroyed; }

void Object::CloneInto(ICloneable *clone) const
{
    Object *obj = Cast<Object*>(clone);
    obj->SetEnabled( IsEnabled() );
}


