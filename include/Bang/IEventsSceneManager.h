#ifndef IEVENTSSCENEMANAGER_H
#define IEVENTSSCENEMANAGER_H

#include "Bang/IEvents.h"

NAMESPACE_BANG_BEGIN

FORWARD class Path;
FORWARD class Scene;

class IEventsSceneManager
{
    IEVENTS(IEventsSceneManager);

public:
    virtual void OnSceneLoaded(Scene *scene, const Path &sceneFilepath)
    {
        (void) scene;
        (void) sceneFilepath;
    }
};

NAMESPACE_BANG_END

#endif // IEVENTSSCENEMANAGER_H
