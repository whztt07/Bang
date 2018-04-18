#ifndef TEXTURECUBEMAP_H
#define TEXTURECUBEMAP_H

#include "Bang/Bang.h"

#include "Bang/GL.h"
#include "Bang/Asset.h"
#include "Bang/Image.h"
#include "Bang/Texture.h"
#include "Bang/ResourceHandle.h"

NAMESPACE_BANG_BEGIN

FORWARD class Path;
FORWARD class XMLNode;

class TextureCubeMap : public Texture
{
    ASSET(TextureCubeMap)

public:
    TextureCubeMap();
    virtual ~TextureCubeMap();

    // Texture
    void CreateEmpty(int size,  GL::ColorComp colorComp, GL::DataType dataType);
    void Resize(int size);
    void Fill(GL::CubeMapDir cubeMapDir,
              const Byte *newData,
              int size,
              GL::ColorComp inputDataColorComp,
              GL::DataType inputDataType);

    void Fill(GL::CubeMapDir cubeMapDir, const Imageb &img);
    void SetImageResource(GL::CubeMapDir cubeMapDir, Imageb *img);
    Imageb ToImage(GL::CubeMapDir cubeMapDir) const;
    RH<Imageb> GetImageResource(GL::CubeMapDir cubeMapDir) const;

    // Serializable
    virtual void ImportXML(const XMLNode &xmlInfo) override;
    virtual void ExportXML(XMLNode *xmlInfo) const override;

    // Resource
    virtual void Import(const Path &textureCubeMapFilepath) override;
    void Import(const Image<Byte> &topImage,   const Image<Byte> &botImage,
                const Image<Byte> &leftImage,  const Image<Byte> &rightImage,
                const Image<Byte> &frontImage, const Image<Byte> &backImage);

    // GLObject
    GL::BindTarget GetGLBindTarget() const override;

private:
    static const std::array<GL::CubeMapDir, 6> AllCubeMapDirs;
    std::array<RH<Imageb>, 6> m_imageResources;

    void CreateEmpty(int width, int height,
                     GL::ColorComp colorComp,
                     GL::DataType dataType) override;
    void Resize(int width, int height) override;

    static unsigned int GetDirIndex(GL::CubeMapDir dir);
};

NAMESPACE_BANG_END

#endif // TEXTURECUBEMAP_H
