#include "Bang/AnimatorStateMachine.h"

#include "Bang/AnimatorStateMachineLayer.h"
#include "Bang/AnimatorStateMachineNode.h"
#include "Bang/AnimatorStateMachineTransition.h"
#include "Bang/AnimatorStateMachineTransitionCondition.h"
#include "Bang/AnimatorStateMachineVariable.h"
#include "Bang/MetaFilesManager.h"
#include "Bang/MetaNode.h"

using namespace Bang;

AnimatorStateMachine::AnimatorStateMachine()
{
    EventEmitter<IEventsAnimatorStateMachine>::RegisterListener(this);
}

AnimatorStateMachine::~AnimatorStateMachine()
{
}

AnimatorStateMachineLayer *AnimatorStateMachine::CreateNewLayer()
{
    AnimatorStateMachineLayer *layer = new AnimatorStateMachineLayer();
    layer->SetStateMachine(this);
    layer->SetLayerName("New Layer");
    m_layers.PushBack(layer);
    return layer;
}

void AnimatorStateMachine::RemoveLayer(AnimatorStateMachineLayer *layer)
{
    if (m_layers.Contains(layer))
    {
        m_layers.Remove(layer);

        EventEmitter<IEventsAnimatorStateMachine>::PropagateToListeners(
            &IEventsAnimatorStateMachine::OnLayerRemoved, this, layer);
    }
}

AnimatorStateMachineVariable *AnimatorStateMachine::CreateNewVariable()
{
    AnimatorStateMachineVariable *var = new AnimatorStateMachineVariable();
    var->SetStateMachine(this);

    String varName = "NewVariable";
    varName = Path::GetDuplicateString(varName, GetVariablesNames());
    var->SetName(varName);

    m_variables.PushBack(var);

    return var;
}

AnimatorStateMachineVariable *AnimatorStateMachine::CreateOrGetVariable(
    const String &varName)
{
    AnimatorStateMachineVariable *var = GetVariable(varName);
    if (!var)
    {
        var = CreateNewVariable();
        var->SetName(varName);
    }
    return var;
}

void AnimatorStateMachine::OnVariableNameChanged(
    AnimatorStateMachineVariable *variable,
    const String &prevVariableName,
    const String &nextVariableName)
{
    BANG_UNUSED(variable);
    for (AnimatorStateMachineLayer *layer : GetLayers())
    {
        for (AnimatorStateMachineNode *node : layer->GetNodes())
        {
            for (AnimatorStateMachineTransition *trans : node->GetTransitions())
            {
                for (auto transCond : trans->GetTransitionConditions())
                {
                    if (transCond->GetVariableName() == prevVariableName)
                    {
                        transCond->SetVariableName(nextVariableName);
                    }
                }
            }
        }
    }
}

void AnimatorStateMachine::SetVariableFloat(const String &varName,
                                            const float value)
{
    AnimatorStateMachineVariable *var = CreateOrGetVariable(varName);
    var->SetType(AnimatorStateMachineVariable::Type::FLOAT);
    var->SetValueFloat(value);
}

void AnimatorStateMachine::SetVariableBool(const String &varName,
                                           const bool value)
{
    AnimatorStateMachineVariable *var = CreateOrGetVariable(varName);
    var->SetType(AnimatorStateMachineVariable::Type::BOOL);
    var->SetValueBool(value);
}

void AnimatorStateMachine::RemoveVariable(AnimatorStateMachineVariable *var)
{
    RemoveVariable(m_variables.IndexOf(var));
}

void AnimatorStateMachine::RemoveVariable(uint varIdx)
{
    if (varIdx < m_variables.Size())
    {
        delete m_variables[varIdx];
        m_variables.RemoveByIndex(varIdx);
    }
}

float AnimatorStateMachine::GetVariableFloat(const String &varName) const
{
    if (AnimatorStateMachineVariable *var = GetVariable(varName))
    {
        return var->GetValueBool();
    }
    return 0.0f;
}

bool AnimatorStateMachine::GetVariableBool(const String &varName) const
{
    if (AnimatorStateMachineVariable *var = GetVariable(varName))
    {
        return var->GetValueBool();
    }
    return false;
}

AnimatorStateMachineVariable *AnimatorStateMachine::GetVariable(
    const String &varName) const
{
    for (AnimatorStateMachineVariable *var : m_variables)
    {
        if (var->GetName() == varName)
        {
            return var;
        }
    }
    return nullptr;
}

const Array<AnimatorStateMachineVariable *>
    &AnimatorStateMachine::GetVariables() const
{
    return m_variables;
}

const Array<AnimatorStateMachineLayer *> &AnimatorStateMachine::GetLayers()
    const
{
    return m_layers;
}

void AnimatorStateMachine::Clear()
{
    while (!m_layers.IsEmpty())
    {
        RemoveLayer(m_layers.Back());
    }

    while (!m_variables.IsEmpty())
    {
        RemoveVariable(m_variables.Size() - 1);
    }
}

Array<String> AnimatorStateMachine::GetVariablesNames() const
{
    Array<String> varNames;
    for (AnimatorStateMachineVariable *var : m_variables)
    {
        varNames.PushBack(var->GetName());
    }
    return varNames;
}

void AnimatorStateMachine::Import(const Path &resourceFilepath)
{
    BANG_UNUSED(resourceFilepath);
    Clear();
}

void AnimatorStateMachine::ImportMeta(const MetaNode &metaNode)
{
    Resource::ImportMeta(metaNode);

    const auto &layersMetaNodes = metaNode.GetChildren("Layers");
    for (const MetaNode &layerMetaNode : layersMetaNodes)
    {
        AnimatorStateMachineLayer *layer = CreateNewLayer();
        layer->ImportMeta(layerMetaNode);
    }

    const auto &varsMetaNodes = metaNode.GetChildren("Variables");
    for (const MetaNode &varMetaNode : varsMetaNodes)
    {
        AnimatorStateMachineVariable *var = CreateNewVariable();
        var->ImportMeta(varMetaNode);
    }
}

void AnimatorStateMachine::ExportMeta(MetaNode *metaNode) const
{
    Resource::ExportMeta(metaNode);

    for (const AnimatorStateMachineLayer *layer : GetLayers())
    {
        MetaNode layerMeta = layer->GetMeta();
        metaNode->AddChild(layerMeta, "Layers");
    }

    for (const AnimatorStateMachineVariable *var : GetVariables())
    {
        MetaNode varMeta = var->GetMeta();
        metaNode->AddChild(varMeta, "Variables");
    }
}
