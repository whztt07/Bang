#include "Bang/FileTracker.h"

#include "Bang/Time.h"
#include "Bang/UMap.h"
#include "Bang/List.h"
#include "Bang/Debug.h"

USING_NAMESPACE_BANG

FileTracker::FileTracker()
{
}

FileTracker::~FileTracker()
{
}

void FileTracker::TrackPath(const Path &path)
{
    return;

    if (path.IsDir())
    {
        List<Path> dirSubPaths = path.GetSubPaths(Path::FindFlag::RECURSIVE_HIDDEN);
        for (const Path &subPath : dirSubPaths) { TrackPath(subPath); }
    }

    if (path.Exists())
    {
        bool wasBeingTracked = (m_pathsToTrackToModificationTime.ContainsKey(path));
        m_pathsToTrackToModificationTime.Add(path,
                                             path.GetModificationTimeSeconds());
        if (!wasBeingTracked)
        {
            m_pathsJustRecentlyTracked.Add(path);
            EventEmitter<IEventsFileTracker>::
               PropagateToListeners(&IEventsFileTracker::OnPathAdded, path);
        }
    }
}

void FileTracker::UnTrackPath(const Path &path)
{
    if (path.IsDir())
    {
        List<Path> subpaths = path.GetSubPaths(Path::FindFlag::RECURSIVE_HIDDEN);
        for (const Path &subpath : subpaths)
        {
            UnTrackPath(subpath);
        }
    }
    m_pathsToTrackToModificationTime.Remove(path);
}

void FileTracker::Clear()
{
    m_pathsToTrackToModificationTime.Clear();
}

void FileTracker::Update(bool forceCheckNow)
{
    if (NeedsCheck() || forceCheckNow)
    {
        const USet<Path> previousJustRecentlyTrackedPaths =
                                        m_pathsJustRecentlyTracked;
        const UMap<Path, double> previousPathsToTrack =
                                        m_pathsToTrackToModificationTime;

        // Check for removed paths
        for (const auto &previousPathToModTime : previousPathsToTrack)
        {
            const Path &previousPath = previousPathToModTime.first;
            if (!previousPath.Exists())
            {
                EventEmitter<IEventsFileTracker>::
                   PropagateToListeners(&IEventsFileTracker::OnPathRemoved,
                                        previousPath);
                UnTrackPath(previousPath);
            }
        }

        m_pathsJustRecentlyTracked.Clear(); // Clear just tracked paths

        // Check for new paths and add them to track
        for (const auto &pathToModTime : previousPathsToTrack)
        {
            const Path &path = pathToModTime.first;
            TrackPath(path);
        }

        // if (m_pathsToTrackToModificationTime.Size() < 10)
        //     Debug_Peek(m_pathsToTrackToModificationTime);

        // Check for modified paths
        for (const auto &newPathToModTime : m_pathsToTrackToModificationTime)
        {
            const Path &newPath = newPathToModTime.first;
            double epsilon = GetCheckFrequencySeconds() / 2.0;
            bool isRecentlyCreatedPath =
                      previousJustRecentlyTrackedPaths.Contains(newPath);
            ASSERT(epsilon < GetCheckFrequencySeconds());
            if (newPath.GetModificationTimeSeconds() >= (m_lastCheckTime - epsilon) &&
                !isRecentlyCreatedPath)
            {
                EventEmitter<IEventsFileTracker>::
                   PropagateToListeners(&IEventsFileTracker::OnPathModified,
                                        newPath);
            }
        }

        m_lastCheckTime = Time::GetNow_Seconds();
    }
}

void FileTracker::SetCheckFrequencySeconds(float checkFrequencySeconds)
{
    m_checkFrequencySeconds = checkFrequencySeconds;
}

bool FileTracker::NeedsCheck() const
{
    float passedSecondsSinceLastCheck = (Time::GetNow_Seconds() - m_lastCheckTime);
    return (passedSecondsSinceLastCheck > GetCheckFrequencySeconds());
}

float FileTracker::GetCheckFrequencySeconds() const
{
    return m_checkFrequencySeconds;
}
