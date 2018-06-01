#ifndef OBJECT_H
#define OBJECT_H

#include "Bang/ObjectId.h"
#include "Bang/EventEmitter.h"
#include "Bang/Serializable.h"
#include "Bang/IEventsObject.h"
#include "Bang/IEventsDestroy.h"

NAMESPACE_BANG_BEGIN

class Object : public Serializable,
               public ICloneable,
               public EventListener<IObjectEvents>,
               public EventEmitter<IEventsDestroy>,
               public EventEmitter<IObjectEvents>
{
public:
    const ObjectId& GetObjectId() const;
    void SetEnabled(bool enabled);

    bool IsActive() const;
    bool IsEnabled() const;
    bool IsStarted() const;
    bool IsWaitingToBeDestroyed() const;

    // ICloneable
    virtual void CloneInto(ICloneable *clone) const override;

protected:
    Object() = default;
    virtual ~Object();

    virtual void PreStart();
    virtual void Start();

    virtual void OnPreStart();
    virtual void OnStart();
    virtual void OnEnabled() override;
    virtual void OnDisabled() override;
    virtual void OnDestroy();

    static void DestroyObject(Object *object);

private:
    ObjectId m_objectId;
    bool m_enabled = true;
    bool m_started = false;
    bool m_waitingToBeDestroyed = false;
};


NAMESPACE_BANG_END

#endif // OBJECT_H
