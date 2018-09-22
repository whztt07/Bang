#include "Bang/GameObject.h"

#include <iostream>

#include "Bang/GL.h"
#include "Bang/AABox.h"
#include "Bang/Debug.h"
#include "Bang/Scene.h"
#include "Bang/Sphere.h"
#include "Bang/Camera.h"
#include "Bang/MetaNode.h"
#include "Bang/Renderer.h"
#include "Bang/Material.h"
#include "Bang/Component.h"
#include "Bang/Transform.h"
#include "Bang/SceneManager.h"
#include "Bang/RectTransform.h"
#include "Bang/IEventsObject.h"
#include "Bang/ComponentFactory.h"
#include "Bang/GameObjectFactory.h"

USING_NAMESPACE_BANG

GameObject::GameObject(const String &name)
{
    SetName(name);
}

GameObject::~GameObject()
{
    ASSERT(GetChildren().IsEmpty());
    ASSERT(GetComponents().IsEmpty());
    ASSERT(IsWaitingToBeDestroyed());
    SetParent(nullptr);
    ASSERT(GetParent() == nullptr);
}

void GameObject::PreStart()
{
    PropagateToChildren(&GameObject::PreStart);
    PropagateToComponents(&Component::PreStart);
    Object::PreStart();
}

void GameObject::Start()
{
    PropagateToChildren(&GameObject::Start);
    PropagateToComponents(&Component::Start);
    Object::Start();
}

void GameObject::PreUpdate()
{
    if (IsActive())
    {
        PropagateToComponents(&Component::PreUpdate);
        PropagateToChildren(&GameObject::PreUpdate);
    }
}

void GameObject::BeforeChildrenUpdate()
{
    if (IsActive())
    {
        PropagateToComponents(&Component::BeforeChildrenUpdate);
    }
}

void GameObject::Update()
{
    if (IsActive())
    {
        PropagateToComponents(&Component::Update);
        BeforeChildrenUpdate();
        PropagateToChildren(&GameObject::Update);
        AfterChildrenUpdate();
    }
}

void GameObject::AfterChildrenUpdate()
{
    if (IsActive())
    {
        PropagateToComponents(&Component::OnAfterChildrenUpdate);
    }
}

void GameObject::PostUpdate()
{
    if (IsActive())
    {
        PropagateToComponents(&Component::OnPostUpdate);
        PropagateToChildren(&GameObject::PostUpdate);
    }
}

void GameObject::BeforeRender()
{
    if (IsActive())
    {
        PropagateToComponents(&Component::OnBeforeRender);
        PropagateToChildren(&GameObject::BeforeRender);
    }
}

void GameObject::Render(RenderPass renderPass, bool renderChildren)
{
    if (IsActive() && IsVisible())
    {
        PropagateToComponents(&Component::OnRender, renderPass);
        if (renderChildren)
        {
            BeforeChildrenRender(renderPass);
            PropagateToChildren(&GameObject::Render, renderPass, true);
            AfterChildrenRender(renderPass);
        }
    }
}

void GameObject::BeforeChildrenRender(RenderPass renderPass)
{   
    if (IsActive())
    {
        PropagateToComponents(&Component::OnBeforeChildrenRender, renderPass);
    }
}

void GameObject::AfterChildrenRender(RenderPass renderPass)
{
    if (IsActive())
    {
        PropagateToComponents(&Component::OnAfterChildrenRender, renderPass);
    }
}

void GameObject::ChildAdded(GameObject *addedChild, GameObject *parent)
{
    EventEmitter<IEventsChildren>::
          PropagateToListeners(&IEventsChildren::OnChildAdded,
                               addedChild, parent);
    PropagateToArray(&EventListener<IEventsChildren>::OnChildAdded,
                    GetComponents< EventListener<IEventsChildren> >(),
                    addedChild, parent);
}

void GameObject::ChildRemoved(GameObject *removedChild, GameObject *parent)
{
    EventEmitter<IEventsChildren>::
          PropagateToListeners(&EventListener<IEventsChildren>::OnChildRemoved,
                               removedChild, parent);
    PropagateToArray(&EventListener<IEventsChildren>::OnChildRemoved,
                    GetComponents< EventListener<IEventsChildren> >(),
                    removedChild, parent);
}

Component* GameObject::AddComponent(Component *component, int index_)
{
    m_componentsToAdd.PushBack( std::make_pair(component, index_) );
    TryToAddQueuedComponents();
    return component;
}

void GameObject::AddChild(GameObject *child, int index, bool keepWorldTransform)
{
    ChildToAdd childToAdd;
    childToAdd.child = child;
    childToAdd.index = index;
    childToAdd.keepWorldTransform = keepWorldTransform;
    m_childrenToAdd.PushBack(childToAdd);
    TryToAddQueuedChildren();
}

void GameObject::AddChild_(GameObject *child,
                           int index_,
                           bool keepWorldTransform)
{
    ASSERT( child );
    ASSERT( child != this );

    int index = (index_ != -1 ? index_ : GetChildren().Size());
    ASSERT(index >= 0 && index <= GetChildren().Size());
    if (child->GetParent() != this) // Parent change
    {
        Matrix4 prevWorldTransform = Matrix4::Identity;
        if (keepWorldTransform && child->GetTransform())
        {
            prevWorldTransform = child->GetTransform()->GetLocalToWorldMatrix();
        }

        GameObject *oldParent = child->GetParent();

        child->p_parent = this;
        if (keepWorldTransform && GetTransform() && child->GetTransform())
        {
            child->GetTransform()->FillFromMatrix(GetTransform()->
                                                  GetLocalToWorldMatrixInv() *
                                                  prevWorldTransform);
        }

        m_children.Insert(child, index);
        ChildAdded(child, this);

        EventEmitter<IEventsChildren>::
               PropagateToListeners(&IEventsChildren::OnParentChanged,
                                    oldParent, this);
        PropagateToArray(&EventListener<IEventsChildren>::OnParentChanged,
                        GetComponents<EventListener<IEventsChildren>>(),
                        oldParent,
                        this);
    }
    else // Its a movement
    {
        int oldIndex = GetChildren().IndexOf(this);
        if (oldIndex != index)
        {
            index =  (oldIndex < index) ? (index-1) : index;
            m_children.Remove(child);
            m_children.Insert(child, index);
        }
    }
}

void GameObject::RemoveChild(GameObject *child)
{
    int i = m_children.IndexOf(child);
    if (i >= 0)
    {
        m_children[i] = nullptr;
        TryToClearDeletedChildren();
        ChildRemoved(child, this);
    }
}

Component *GameObject::AddComponent_(Component *component, int index_)
{
    ASSERT(m_componentsIterationDepth == 0);
    if (component && !GetComponents().Contains(component))
    {
        Transform *transformComp = FastDynamicCast<Transform*>(component);
        if (transformComp)
        {
            ASSERT_SOFT_MSG(!HasComponent<Transform>(),
                            "A GameObject can not have more than one transform");
        }

        const int index = (index_ != -1 ? index_ : GetComponents().Size());
        m_components.Insert(component, index);

        if (transformComp)
        {
            p_transform = transformComp;
            if (RectTransform *rt = FastDynamicCast<RectTransform*>(component))
            {
                p_rectTransform = rt;
            }
        }

        component->SetGameObject(this);

        EventEmitter<IEventsComponent>::PropagateToListeners(
                    &IEventsComponent::OnComponentAdded, component, index);
    }
    return component;
}

void GameObject::RemoveComponent(Component *component)
{
    int i = m_components.IndexOf(component);
    if (i >= 0)
    {
        m_components[i] = nullptr;

        EventEmitter<IEventsComponent>::PropagateToListeners(
                    &IEventsComponent::OnComponentRemoved, component, this);

        if (component == p_transform)
        {
            p_transform = nullptr;
        }

        if (component == p_rectTransform)
        {
            p_rectTransform = nullptr;
        }
    }
}

void GameObject::OnEnabled(Object *object)
{
    Object::OnEnabled(object);
    PropagateToArray(&EventListener<IEventsObject>::OnEnabled,
                    GetComponents<EventListener<IEventsObject>>(), object);
    PropagateToChildren(&EventListener<IEventsObject>::OnEnabled, object);
}
void GameObject::OnDisabled(Object *object)
{
    Object::OnDisabled(object);
    PropagateToArray(&EventListener<IEventsObject>::OnDisabled,
                    GetComponents<EventListener<IEventsObject>>(), object);
    PropagateToChildren(&EventListener<IEventsObject>::OnDisabled, object);
}

void GameObject::TryToAddQueuedChildren()
{
    if (m_childrenIterationDepth == 0)
    {
        for (const auto &childToAddStruct : m_childrenToAdd)
        {
            GameObject *childToAdd = childToAddStruct.child;
            int idx = childToAddStruct.index;
            bool keepWorldTransform = childToAddStruct.keepWorldTransform;
            AddChild_(childToAdd, idx, keepWorldTransform);
        }
        m_childrenToAdd.Clear();
    }
}

void GameObject::TryToAddQueuedComponents()
{
    if (m_componentsIterationDepth == 0)
    {
        for (const auto &pair : m_componentsToAdd)
        {
            Component *compToAdd = pair.first;
            uint idx = pair.second;
            AddComponent_(compToAdd, idx);
        }
        m_componentsToAdd.Clear();
    }
}

void GameObject::TryToClearDeletedChildren()
{
    Array<GameObject*> newChildren;
    for (GameObject *child : m_children)
    {
        if (child)
        {
            newChildren.PushBack(child);
        }
    }
    m_children = newChildren;
}

void GameObject::TryToClearDeletedComponents()
{
    Array<Component*> newComponents;
    for (Component *comp : m_components)
    {
        if (comp)
        {
            newComponents.PushBack(comp);
        }
    }
    m_components = newComponents;
}

void GameObject::DestroyImmediate(GameObject *gameObject)
{
    ASSERT(gameObject);

    if (!gameObject->IsWaitingToBeDestroyed())
    {
        Object::PropagateObjectDestruction(gameObject);

        while (!gameObject->GetChildren().IsEmpty())
        {
            if (GameObject *go = gameObject->GetChildren().Back())
            {
                ASSERT(!go->IsWaitingToBeDestroyed());
                GameObject::DestroyImmediate(go);
            }
            gameObject->TryToClearDeletedChildren();
        }

        while (!gameObject->GetComponents().IsEmpty())
        {
            if (Component *comp = gameObject->GetComponents().Back())
            {
                ASSERT(!comp->IsWaitingToBeDestroyed());
                Component::DestroyImmediate(comp);
            }
            gameObject->TryToClearDeletedComponents();
        }

        delete gameObject;
    }
}

void GameObject::Destroy(GameObject *gameObject)
{
    if (Scene *scene = SceneManager::GetActiveScene())
    {
        scene->AddGameObjectToDestroyDelayed(gameObject);
        gameObject->SetParent(nullptr);
    }
    else
    {
        GameObject::DestroyImmediate(gameObject);
    }
}

bool GameObject::IsEnabled(bool recursive) const
{
    if (!recursive)
    {
        return Object::IsEnabled();
    }
    else
    {
        return IsEnabled(false) &&
               (!GetParent() || GetParent()->IsEnabled(true));
    }
}

const Array<Component *> &GameObject::GetComponents() const
{
    return m_components;
}

Component *GameObject::GetComponentByGUID(const GUID &guid) const
{
    for (Component *comp : GetComponents())
    {
        if (comp->GetGUID() == guid)
        {
            return comp;
        }
    }
    return nullptr;
}

Scene* GameObject::GetScene() const
{
    if (Scene *scene = DCAST<Scene*>( const_cast<GameObject*>(this) ))
    {
        return scene;
    }

    GameObject *parent = GetParent();
    return parent ? parent->GetScene() : nullptr;
}

Transform *GameObject::GetTransform() const
{
    return p_transform;
}
RectTransform *GameObject::GetRectTransform() const
{
    return p_rectTransform;
}

void GameObject::SetName(const String &name)
{
    if (name != GetName())
    {
        String oldName = GetName();
        m_name = name;
        EventEmitter<IEventsName>::PropagateToListeners(
                    &IEventsName::OnNameChanged, this, oldName, GetName());
    }
}

const String& GameObject::GetName() const
{
    return m_name;
}

GameObject *GameObject::Find(const String &name)
{
    Scene *scene = SceneManager::GetActiveScene();
    return scene ? scene->FindInChildren(name) : nullptr;
}

GameObject *GameObject::FindInChildren(const GUID &guid, bool recursive)
{
    for (GameObject *child : GetChildren())
    {
        if (child)
        {
            if (GameObject *found = child->FindInChildrenAndThis(guid, recursive))
            {
                return found;
            }
        }
    }
    return nullptr;
}

GameObject *GameObject::FindInChildren(const String &name, bool recursive)
{
    for (GameObject *child : GetChildren())
    {
        if (child)
        {
            if (GameObject *found = child->FindInChildrenAndThis(name, recursive))
            {
                return found;
            }
        }
    }
    return nullptr;
}

GameObject *GameObject::FindInChildrenAndThis(const GUID &guid, bool recursive)
{
    if (GetGUID() == guid)
    {
        return this;
    }

    for (GameObject *child : GetChildren())
    {
        if (child && child->GetGUID() == guid)
        {
            return child;
        }
        else if (recursive)
        {
            if (GameObject *found = child->FindInChildren(guid, true))
            {
                return found;
            }
        }
    }
    return nullptr;
}

GameObject *GameObject::FindInChildrenAndThis(const String &name, bool recursive)
{
    if (GetName() == name)
    {
        return this;
    }

    for (GameObject *child : GetChildren())
    {
        if (child && child->GetName() == name)
        {
            return child;
        }
        else if (recursive)
        {
            GameObject *found = child->FindInChildren(name, true);
            if (found)
            {
                return found;
            }
        }
    }
    return nullptr;
}

GameObject *GameObject::FindInAncestors(const String &name, bool broadSearch)
{
    return GetParent() ? GetParent()->FindInAncestorsAndThis(name, broadSearch) :
                         nullptr;
}

GameObject *GameObject::FindInAncestorsAndThis(const String &name,
                                               bool broadSearch)
{
    if (GetName() == name)
    {
        return this;
    }

    if (broadSearch)
    {
        if (GetParent())
        {
            for (GameObject *siblingOrThis : GetParent()->GetChildren())
            {
                if (siblingOrThis->GetName() == name)
                {
                    return this;
                }

                if (GameObject *found = siblingOrThis->FindInAncestors(name, true))
                {
                    return found;
                }
            }
        }
    }

    return GetParent() ? GetParent()->FindInAncestorsAndThis(name, false) :
                         nullptr;
}

void GameObject::SetVisible(bool visible)
{
    if (visible != IsVisible())
    {
        m_visible = visible;
        EventEmitter<IEventsGameObjectVisibilityChanged>::PropagateToListeners(
            &IEventsGameObjectVisibilityChanged::OnVisibilityChanged, this);
    }
}

GameObject *GameObject::GetChild(const GUID &guid) const
{
    for (GameObject *go : GetChildren())
    {
        if (go && go->GetGUID() == guid)
        {
            return go;
        }
    }
    return nullptr;
}

GameObject *GameObject::GetChild(const String &name) const
{
    for (GameObject *child : GetChildren())
    {
        if (child && child->GetName() == name)
        {
            return child;
        }
    }
    return nullptr;
}

const Array<GameObject *> &GameObject::GetChildren() const
{
    return m_children;
}

GameObject *GameObject::GetChild(uint index) const
{
    if (index >= GetChildren().Size())
    {
        return nullptr;
    }
    return GetChildren()[index];
}

void GameObject::GetChildrenRecursively(Array<GameObject*> *children) const
{
    for (GameObject *child : GetChildren())
    {
        if (child)
        {
            children->PushBack(child);
            child->GetChildrenRecursively(children);
        }
    }
}

Array<GameObject*> GameObject::GetChildrenRecursively() const
{
    Array<GameObject*> children;
    GetChildrenRecursively(&children);
    return children;
}

bool GameObject::IsChildOf(const GameObject *parent, bool recursive) const
{
    if (!GetParent())
    {
        return false;
    }

    if (recursive)
    {
        return IsChildOf(parent, false) ||
               GetParent()->IsChildOf(parent, true);
    }
    return GetParent() == parent;
}

bool GameObject::IsVisible() const
{
    return m_visible;
}

bool GameObject::IsVisible(bool recursive) const
{
    if (recursive)
    {
        return IsVisible(false) &&
              (!GetParent() || GetParent()->IsVisible(recursive));
    }
    return IsVisible();
}

void GameObject::SetParent(GameObject *newParent,
                           int index,
                           bool keepWorldTransform)
{
    if (newParent != GetParent()) // Parent change
    {
        if (GetParent())
        {
            GetParent()->RemoveChild(this);
            p_parent = nullptr;
        }

        if (newParent)
        {
            newParent->AddChild(this, index, keepWorldTransform);
        }
    }
    else if (GetParent()) // Movement in same parent
    {
        ASSERT(newParent == GetParent());
        if (GetParent()->GetChildren().IndexOf(this) != index)
        {
            GetParent()->AddChild(this, index, keepWorldTransform);
        }
    }
}

GameObject *GameObject::GetParent() const
{
    return p_parent;
}

void GameObject::SetDontDestroyOnLoad(bool dontDestroyOnLoad)
{
    m_dontDestroyOnLoad = dontDestroyOnLoad;
}

int GameObject::GetIndexInsideParent() const
{
    if (GameObject *parent = GetParent())
    {
        return parent->GetChildren().IndexOf(const_cast<GameObject*>(this));
    }
    return -1;
}

bool GameObject::IsDontDestroyOnLoad() const
{
    return m_dontDestroyOnLoad;
}

AARect GameObject::GetBoundingViewportRect(Camera *cam, bool includeChildren) const
{
    AABox bbox = GetAABBoxWorld(includeChildren);
    return cam->GetViewportBoundingAARectNDC(bbox);
}

AABox GameObject::GetLocalAABBox(bool includeChildren) const
{
    AABox aabBox = AABox::Empty;
    Array<Renderer*> rends = GetComponents<Renderer>();
    for (Renderer *rend : rends)
    {
        if (rend && rend->IsEnabled() && rend->GetActiveMaterial())
        {
            RenderPass rp = rend->GetActiveMaterial()->GetRenderPass();
            if (rp == RenderPass::SCENE || rp == RenderPass::SCENE_TRANSPARENT)
            {
                const AABox rendAABox = rend->GetAABBox();
                aabBox = AABox::Union(aabBox, rendAABox);
            }
        }
    }

    if (includeChildren)
    {
        for (GameObject *child : GetChildren())
        {
            AABox aabBoxChild = child->GetLocalAABBox(true);
            if (aabBoxChild != AABox::Empty)
            {
                Matrix4 mat;
                const Transform *childT = child->GetTransform();
                if (childT)
                {
                    mat = childT->GetLocalToParentMatrix();
                }
                aabBoxChild = mat * aabBoxChild;
                aabBox = AABox::Union(aabBox, aabBoxChild);
            }
        }
    }

    return aabBox;
}

AABox GameObject::GetAABBoxWorld(bool includeChildren) const
{
    AABox b = GetLocalAABBox(includeChildren);
    if (b != AABox::Empty)
    {
        Matrix4 mat = Matrix4::Identity;
        if (GetTransform())
        {
            mat = GetTransform()->GetLocalToWorldMatrix();
        }
        b = mat * b;
    }
    return b;
}

Sphere GameObject::GetLocalBoundingSphere(bool includeChildren) const
{
    return Sphere::FromBox(GetLocalAABBox(includeChildren));
}

Sphere GameObject::GetBoundingSphere(bool includeChildren) const
{
    return Sphere::FromBox(GetAABBoxWorld(includeChildren));
}

void GameObject::PropagateToChildren(std::function<void(GameObject*)> func)
{
    ++m_childrenIterationDepth;

    const Array<GameObject*>& children = GetChildren();
    for (GameObject *child : children)
    {
        if (child && child->IsEnabled())
        {
            func(child);
        }
    }

    if (--m_childrenIterationDepth == 0)
    {
        TryToAddQueuedChildren();
        TryToClearDeletedChildren();
    }
}

void GameObject::PropagateToComponents(std::function<void(Component*)> func)
{
    ++m_componentsIterationDepth;

    const Array<Component*>& components = GetComponents();
    for (Component *comp : components)
    {
        if (comp && comp->IsEnabled())
        {
            func(comp);
        }
    }

    if (--m_componentsIterationDepth == 0)
    {
        TryToAddQueuedComponents();
        TryToClearDeletedComponents();
    }
}

template<class T>
bool CanEventBePropagatedToGameObject(const GameObject *go)
{
    return go->IsEnabled() && go->T::IsReceivingEvents();
}

template<class T>
bool CanEventBePropagatedToComponent(const Component *comp)
{
    return comp->IsEnabled() && comp->T::IsReceivingEvents();
}

void GameObject::CloneInto(ICloneable *clone) const
{
    Object::CloneInto(clone);

    GameObject *go = SCAST<GameObject*>(clone);
    go->SetName(m_name);
    go->SetParent(nullptr);

    for (GameObject *child : GetChildren())
    {
        if (child && !child->GetHideFlags().IsOn(HideFlag::DONT_CLONE))
        {
            GameObject *childClone = child->Clone();
            childClone->SetParent(go);
        }
    }

    for (Component *comp : GetComponents())
    {
        if (comp && !comp->GetHideFlags().IsOn(HideFlag::DONT_CLONE))
        {
            go->AddComponent(comp->Clone());
        }
    }
}

void GameObject::Print(const String &indent) const
{
    String indent2 = indent; indent2 += "   ";
    Debug_Log(indent << this);
    for (GameObject *child : GetChildren())
    {
        if (child)
        {
            child->Print(indent2);
        }
    }
}

String GameObject::ToStringStructure(bool recursive, const String &indent)
{
    std::ostringstream oss;
    oss << indent << " - " << GetName() << " " << GetComponents();
    if (recursive)
    {
        oss << "\n";
        for (GameObject *child : GetChildren())
        {
            if (child)
            {
                oss << child->ToStringStructure(true, indent + "  ");
            }
        }
    }
    return String(oss.str());
}

String GameObject::ToString() const
{
    std::ostringstream oss;
    oss << "GameObject: " << GetName() << "(" << ((void*)this) << ")";
    return oss.str();
}

void GameObject::ImportMeta(const MetaNode &metaNode)
{
    Serializable::ImportMeta(metaNode);

    if (metaNode.Contains("Enabled"))
    {
        SetEnabled( metaNode.Get<bool>("Enabled") );
    }

    if (metaNode.Contains("Visible"))
    {
        SetVisible( metaNode.Get<bool>("Visible") );
    }

    if (metaNode.Contains("Name"))
    {
        SetName( metaNode.Get<String>("Name") );
    }

    if (metaNode.Contains("DontDestroyOnLoad"))
    {
        SetDontDestroyOnLoad( metaNode.Get<bool>("DontDestroyOnLoad") );
    }

    USet<GameObject*> childrenToRemove;
    for (GameObject *child : GetChildren())
    {
        childrenToRemove.Add(child);
    }

    USet<Component*> componentsToRemove;
    for (Component *comp : GetComponents())
    {
        componentsToRemove.Add(comp);
    }

    for (const MetaNode& childMeta : metaNode.GetChildren())
    {
        const GUID guid = childMeta.Get<GUID>("GUID");
        const String& tagName = childMeta.GetName();

        if (tagName == GameObject::GetClassNameStatic())
        {
            GameObject *child = nullptr;
            for (GameObject *containedChild : GetChildren())
            {
                // See if it is already contained
                if (containedChild && containedChild->GetGUID() == guid)
                {
                    child = containedChild;
                    childrenToRemove.Remove(child);
                    break;
                }
            }

            if (!child)
            {
                child = GameObjectFactory::CreateGameObject(false);
            }
            else
            {
                child->SetParent(nullptr); // To reorder
            }
            child->SetParent(this);
            child->ImportMeta(childMeta);
        }
        else
        {
            Component *comp = nullptr;
            for (Component *containedComp : GetComponents())
            {
                // See if it is already contained
                if (containedComp && containedComp->GetGUID() == guid)
                {
                    comp = containedComp;
                    componentsToRemove.Remove(comp);
                    break;
                }
            }

            if (!comp)
            {
                comp = ComponentFactory::Create(tagName);
                AddComponent(comp);
            }
            else
            {
                RemoveComponent(comp); // To reorder
                AddComponent(comp);
            }
            comp->ImportMeta(childMeta);
        }
    }

    // Remove non existing gameObjects and components in meta
    for (GameObject *childToRemove : childrenToRemove)
    {
        if (childToRemove)
        {
            GameObject::DestroyImmediate(childToRemove);
        }
    }

    for (Component *compToRemove : componentsToRemove)
    {
        if (compToRemove)
        {
            Component::DestroyImmediate(compToRemove);
        }
    }
}

void GameObject::ExportMeta(MetaNode *metaNode) const
{
    Serializable::ExportMeta(metaNode);

    metaNode->Set("Enabled", IsEnabled());
    metaNode->Set("Visible", IsVisible());
    metaNode->Set("Name", GetName());
    metaNode->Set("DontDestroyOnLoad", IsDontDestroyOnLoad());

    for (Component *c : GetComponents())
    {
        if (c && c->GetHideFlags().IsOff(HideFlag::DONT_SERIALIZE))
        {
            MetaNode compMeta;
            c->ExportMeta(&compMeta);
            metaNode->AddChild(compMeta);
        }
    }

    for (GameObject *child : GetChildren())
    {
        if (child && child->GetHideFlags().IsOff(HideFlag::DONT_SERIALIZE))
        {
            MetaNode childMeta;
            child->ExportMeta(&childMeta);
            metaNode->AddChild(childMeta);
        }
    }
}
