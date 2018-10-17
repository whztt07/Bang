#include "Bang/AudioPlayerRunnable.h"

#include "Bang/ALAudioSource.h"
#include "Bang/AudioClip.h"
#include "Bang/AudioManager.h"
#include "Bang/EventEmitter.tcc"
#include "Bang/IEventsDestroy.h"

using namespace Bang;

AudioPlayerRunnable::AudioPlayerRunnable(AudioClip *clip,
                                         ALAudioSource *alAudioSource,
                                         float delayInSeconds)
{
    p_audioClip = clip;
    p_alAudioSource = alAudioSource;
    m_delayInSeconds = delayInSeconds;
    SetAutoDelete(true);
}

AudioPlayerRunnable::~AudioPlayerRunnable()
{
    AudioManager::GetInstance()->OnAudioFinishedPlaying(this);
    EventEmitter<IEventsDestroy>::PropagateToListeners(
        &IEventsDestroy::OnDestroyed, this);

    if (p_alAudioSource->m_autoDelete)
    {
        delete p_alAudioSource;
    }
}

void AudioPlayerRunnable::Resume()
{
    p_alAudioSource->Play();
}

void AudioPlayerRunnable::Pause()
{
    p_alAudioSource->Pause();
}

void AudioPlayerRunnable::Stop()
{
    m_forceExit = true;
    p_alAudioSource->Stop();
}

AudioClip *AudioPlayerRunnable::GetAudioClip() const
{
    return p_audioClip;
}

ALAudioSource *AudioPlayerRunnable::GetALAudioSource() const
{
    return p_alAudioSource;
}

void AudioPlayerRunnable::Run()
{
    if (!p_audioClip->IsLoaded())
    {
        return;
    }

    if (m_delayInSeconds > 0.0f)  // Wait delay
    {
        Thread::SleepCurrentThread(m_delayInSeconds);
    }

    p_alAudioSource->Play();  // Play and wait until source is stopped
    do
    {
        Thread::SleepCurrentThread(0.3f);
    } while (!m_forceExit && !p_alAudioSource->IsStopped());
}
