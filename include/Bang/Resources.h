#ifndef RESOURCES_H
#define RESOURCES_H

#include <sstream>

#include "Bang/UMap.h"
#include "Bang/USet.h"
#include "Bang/Asset.h"
#include "Bang/Paths.h"
#include "Bang/TypeMap.h"
#include "Bang/Resource.h"
#include "Bang/Serializable.h"
#include "Bang/XMLNodeReader.h"
#include "Bang/ResourceHandle.h"
#include "Bang/ImportFilesManager.h"

NAMESPACE_BANG_BEGIN

FORWARD class Asset;
FORWARD class MeshFactory;
FORWARD class MaterialFactory;
FORWARD class ShaderProgramFactory;

class Resources
{
public:
    Resources();
    virtual ~Resources();

    template <class ResourceClass>
    static RH<ResourceClass> Load(const Path &filepath);

    template <class ResourceClass>
    static RH<ResourceClass> Load(const String &filepath);

    template <class ResourceClass>
    static RH<ResourceClass> Load(const GUID &guid);

    static RH<Resource> LoadFromExtension(const Path &filepath);

    static void Import(Resource *res);

    template<class ResourceClass, class ...Args>
    static RH<ResourceClass> Create(const Args&... args);
    template<class ResourceClass, class ...Args>
    static RH<ResourceClass> Create(const GUID &guid, const Args&... args);
    template<class ResourceClass, class ...Args>
    static RH<ResourceClass> CreateInnerResource(
                                        const GUID &baseGUID,
                                        const GUID::GUIDType embeddedFileGUID,
                                        const Args&... args);

    template<class ResourceClass>
    static RH<ResourceClass> Clone(const RH<ResourceClass> &src);

    template <class ResourceClass>
    static Array<ResourceClass*> GetAll();
    static Array<Resource*> GetAllResources();
    static void PrintAll();

    static void CreateResourceXMLAndImportFile(const Resource *resource,
                                               const Path &exportFilepath);

    struct ResourceEntry : public IToString
    {
        Resource *resource = nullptr;
        uint usageCount = 0; // Number of RH's using this resource entry
        String ToString() const override
        {
            return "RE(" + String(resource) + ", " + String(usageCount) + ")";
        }
    };

    static void Add(const TypeId &resTypeId, Resource *res);

    static bool IsEmbeddedResource(const GUID &guid);
    static bool IsEmbeddedResource(const Path &resourcePath);

    static void SetPermanent(Resource *resource, bool permanent);
    static bool IsPermanent(Resource *resource);
    static void SetPermanent(const Path &resourcePath, bool permanent);
    static bool IsPermanent(const Path &resourcePath);

    static void RegisterResourceUsage(const TypeId &resTypeId,
                                      Resource *resource);
    static void UnRegisterResourceUsage(const TypeId &resTypeId,
                                        Resource *resource);

    static void Remove(const TypeId &resTypeId, const GUID &guid);

    template<class ResourceClass, class ...Args>
    static ResourceClass* _Create(const Args&... args);
    template<class ResourceClass, class ...Args>
    static ResourceClass* _Create(const GUID &guid, const Args&... args);

    template<class ResourceClass, class ...Args>
    static typename std::enable_if<T_SUBCLASS(ResourceClass, Asset),
           ResourceClass*>::type _JustCreate(const Args&... args);

    template<class ResourceClass, class ...Args>
    static typename std::enable_if<T_NOT_SUBCLASS(ResourceClass, Asset),
           ResourceClass*>::type _JustCreate(const Args&... args);

    template<class ResourceClass>
    static bool Contains(const GUID &guid);
    static bool Contains(const TypeId &resourceClassTypeId, const GUID &guid);

    static void Destroy(Resource *resource);

    template<class ResourceClass>
    static ResourceClass* GetCached(const GUID &guid);
    template<class ResourceClass>
    static ResourceClass* GetCached(const Path &path);

    static Array<Resource*> GetCached(const GUID &guid);
    static Array<Resource*> GetCached(const Path &path);
    static Resource* GetCached(const TypeId &resTypeId, const GUID &guid);

    static Path GetResourcePath(const Resource *resource);

    MeshFactory *GetMeshFactory() const;
    MaterialFactory *GetMaterialFactory() const;
    ShaderProgramFactory *GetShaderProgramFactory() const;

    virtual void Init();
    virtual Array<Path> GetLookUpPaths() const;

    void Destroy();

    static Resources* GetInstance();

private:
    USet<Path> m_permanentResourcesPaths;
    USet<Resource*> m_permanentResources;
    TypeMap< UMap<GUID, ResourceEntry> > m_resourcesCache;

    MeshFactory *m_meshFactory = nullptr;
    MaterialFactory *m_materialFactory = nullptr;
    ShaderProgramFactory *m_shaderProgramFactory = nullptr;

    virtual MeshFactory* CreateMeshFactory() const;

    friend class Window;
    friend class GUIDManager;
    friend class IResourceHandle;
};

NAMESPACE_BANG_END

#include "Resources.tcc"

#endif // RESOURCES_H
