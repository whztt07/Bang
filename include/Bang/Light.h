#ifndef LIGHT_H
#define LIGHT_H

#include "BangMath/AARect.h"
#include "Bang/Array.h"
#include "Bang/AssetHandle.h"
#include "Bang/BangDefines.h"
#include "BangMath/Color.h"
#include "Bang/Component.h"
#include "Bang/ComponentMacros.h"
#include "Bang/MetaNode.h"
#include "Bang/String.h"
#include "BangMath/Vector2.h"

namespace Bang
{
class Camera;
class GameObject;
class Serializable;
class Renderer;
class ShaderProgram;
class Texture;

class Light : public Component
{
    COMPONENT_ABSTRACT(Light)

public:
    void SetShadowExponentConstant(float exponentConstant);
    void SetShadowSoftness(uint shadowSoftness);
    void SetCastShadows(bool castShadows);
    void SetColor(const Color &color);
    void SetIntensity(float intensity);
    void SetShadowBias(float shadowBias);
    void SetShadowStrength(float shadowStrength);
    void SetShadowHighBitDepth(bool highBitDepth);
    void SetShadowMapSize(const Vector2i &shadowMapSize);

    const Color &GetColor() const;
    float GetIntensity() const;
    float GetShadowBias() const;
    bool GetCastShadows() const;
    uint GetShadowSoftness() const;
    float GetShadowStrength() const;
    bool GetShadowHighBitDepth() const;
    float GetShadowExponentConstant() const;
    ShaderProgram *GetShadowMapShaderProgram() const;
    virtual float GetShadowMapNearDistance() const;
    virtual float GetShadowMapFarDistance() const;
    const Vector2i &GetShadowMapSize() const;
    virtual Texture *GetShadowMapTexture() const;

    void RenderShadowMaps(GameObject *go);

    // Serializable
    void Reflect() override;

protected:
    Light();
    virtual ~Light() override;

    void SetShadowMapShaderProgram(ShaderProgram *sp);
    void SetLightScreenPassShaderProgram(ShaderProgram *sp);
    Array<Renderer *> GetShadowCastersIn(GameObject *go) const;
    virtual void SetUniformsBeforeApplyingLight(ShaderProgram *sp) const;
    virtual void OnShadowHighBitDepthChanged();

private:
    float m_intensity = 1.0f;
    Color m_color = Color::White();

    uint m_shadowSoftness = 0;
    bool m_castShadows = false;
    float m_shadowBias = 0.003f;
    float m_shadowStrength = 1.0f;
    bool m_shadowHighBitDepth = false;
    float m_shadowExponentConstant = 10.0f;
    Vector2i m_shadowMapSize = Vector2i(256);

    AH<Material> p_shadowMapMaterial;
    AH<ShaderProgram> p_lightScreenPassShaderProgram;

    void SetShadowLightCommonUniforms(ShaderProgram *sp) const;
    void ApplyLight(Camera *camera, const AARect &renderRect) const;
    virtual AARect GetRenderRect(Camera *camera) const;
    virtual void RenderShadowMaps_(GameObject *go) = 0;

    friend class GEngine;
};
}

#endif  // LIGHT_H
