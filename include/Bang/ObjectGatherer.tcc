#ifndef OBJECTGATHERER_TCC
#define OBJECTGATHERER_TCC

#include "Bang/Component.h"
#include "Bang/GameObject.h"
#include "Bang/ObjectGatherer.h"

NAMESPACE_BANG_BEGIN

template<class ObjectType, bool RECURSIVE>
void ObjectGatherer<ObjectType, RECURSIVE>::SetRoot(GameObject *root)
{
    p_root = root;
    RegisterEventsAndGather( GetRoot() );
}

template<class ObjectType, bool RECURSIVE>
GameObject* ObjectGatherer<ObjectType, RECURSIVE>::GetRoot() const
{
    return p_root;
}

template <class ObjectType, bool RECURSIVE>
const List<ObjectType*>&
ObjectGatherer<ObjectType, RECURSIVE>::GetList() const
{
    return m_gatheredObjects;
}

template <class ObjectType, bool RECURSIVE>
void ObjectGatherer<ObjectType, RECURSIVE>::
RegisterEventsAndGather(GameObject *go)
{
    if (!m_processedGameObjects.Contains(go))
    {
        m_processedGameObjects.Add(go);

        if (RECURSIVE)
        {
            go->EventEmitter<IEventsChildren>::RegisterListener(this);
        }
        go->EventEmitter<IEventsDestroy>::RegisterListener(this);
        go->EventEmitter<IEventsComponent>::RegisterListener(this);

        List<ObjectType*> goObjectsOfTheType;
        GatherObjectsOfTheType(&goObjectsOfTheType, go);
        for (ObjectType *obj : goObjectsOfTheType)
        {
            m_gatheredObjects.PushBack(obj);
        }
    }
}


template<class ObjectType, bool RECURSIVE>
void ObjectGatherer<ObjectType, RECURSIVE>::
UnRegisterEventsAndRemoveObjects(GameObject *go)
{
    ASSERT(m_processedGameObjects.Contains(go));
    m_processedGameObjects.Remove(go);

    if (RECURSIVE)
    {
        go->EventEmitter<IEventsChildren>::UnRegisterListener(this);
    }
    go->EventEmitter<IEventsDestroy>::UnRegisterListener(this);
    go->EventEmitter<IEventsComponent>::UnRegisterListener(this);

    List<ObjectType*> goObjectsOfTheType;
    GatherObjectsOfTheType(&goObjectsOfTheType, go);
    for (ObjectType *obj : goObjectsOfTheType)
    {
        m_gatheredObjects.Remove(obj);
    }
}

template<class ObjectType, bool RECURSIVE>
void ObjectGatherer<ObjectType, RECURSIVE>::
GatherObjectsOfTheType(List<ObjectType*> *gatheredObjects, GameObject *go)
{
    if (ObjectType* obj = DCAST<ObjectType*>(go))
    {
        gatheredObjects->PushBack(obj);
    }

    for (Component *comp : go->GetComponents())
    {
        if (ObjectType *obj = DCAST<ObjectType*>(comp))
        {
            gatheredObjects->PushBack(obj);
        }
    }

    if (RECURSIVE)
    {
        for (GameObject *child : go->GetChildren())
        {
            GatherObjectsOfTheType(gatheredObjects, child);
        }
    }
}

template<class ObjectType, bool RECURSIVE>
void ObjectGatherer<ObjectType, RECURSIVE>::
OnChildAdded(GameObject *addedChild, GameObject*)
{
    ASSERT(!RECURSIVE);
    RegisterEventsAndGather(addedChild);
}

template<class ObjectType, bool RECURSIVE>
void ObjectGatherer<ObjectType, RECURSIVE>::
OnChildRemoved(GameObject *removedChild, GameObject*)
{
    ASSERT(!RECURSIVE);
    UnRegisterEventsAndRemoveObjects(removedChild);
}

template<class ObjectType, bool RECURSIVE>
void ObjectGatherer<ObjectType, RECURSIVE>::
OnComponentAdded(Component *addedComponent, int)
{
    if (ObjectType *obj = DCAST<ObjectType*>(addedComponent))
    {
        m_gatheredObjects.PushBack(obj);
    }
}

template<class ObjectType, bool RECURSIVE>
void ObjectGatherer<ObjectType, RECURSIVE>::
OnComponentRemoved(Component *removedComponent, GameObject *previousGameObject)
{
    if (ObjectType *obj = DCAST<ObjectType*>(removedComponent))
    {
        m_gatheredObjects.Remove(obj);
    }
}

template<class ObjectType, bool RECURSIVE>
void ObjectGatherer<ObjectType, RECURSIVE>::
ObjectGatherer::OnDestroyed(EventEmitter<IEventsDestroy> *object)
{
    ASSERT(object == GetRoot());

    GameObject *go = DCAST<GameObject*>(object);

    List<GameObject*> childrenRecursiveAndGo = go->GetChildrenRecursively();
    childrenRecursiveAndGo.PushFront(go);
    for (GameObject *childOrGo : childrenRecursiveAndGo)
    {
        if (RECURSIVE)
        {
            childOrGo->EventEmitter<IEventsChildren>::UnRegisterListener(this);
        }
        childOrGo->EventEmitter<IEventsDestroy>::UnRegisterListener(this);
        childOrGo->EventEmitter<IEventsComponent>::UnRegisterListener(this);
    }
    m_gatheredObjects.Clear();
    SetRoot(nullptr);
}

NAMESPACE_BANG_END

#endif // OBJECTGATHERER_TCC
