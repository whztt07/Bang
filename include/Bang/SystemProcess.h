#ifndef SYSTEMPROCESS_H
#define SYSTEMPROCESS_H

#ifdef __linux__
#include <unistd.h>
#elif _WIN32
#endif

#include "Bang/Math.h"
#include "Bang/List.h"
#include "Bang/String.h"

NAMESPACE_BANG_BEGIN

class SystemProcess
{
public:
    using FileDescriptor = int;

    enum Channel
    {
        #ifdef __linux__
        StandardIn    = STDIN_FILENO,
        StandardOut   = STDOUT_FILENO,
        StandardError = STDERR_FILENO
        #elif _WIN32
        StandardIn = 0, StandardOut = 1, StandardError = 2
        #endif
    };

    SystemProcess();
    ~SystemProcess();

    bool Start(const String &command,
               const List<String> &extraArgs = {});
    bool StartDettached(const String &command,
                        const List<String> &extraArgs = {});
    bool WaitUntilFinished(float seconds = Math::Infinity<float>());
    void Close();

    void Write(const String &str);
    void CloseWriteChannel();

    String ReadStandardOutput();
    String ReadStandardError();
    String ReadFileDescriptor(FileDescriptor fd);

    int GetExitCode() const;
    bool FinishedOk() const;

private:
    int m_childPID = 0;
    int m_exitCode = -1;
    String m_readOutputWhileWaiting = "";
    String m_readErrorWhileWaiting  = "";

    FileDescriptor m_oldFileDescriptors[3];
    FileDescriptor m_childToParentOutFD[2];
    FileDescriptor m_childToParentErrFD[2];
    FileDescriptor m_parentToChildFD[2];

    String ReadStandardOutputRaw();
    String ReadStandardErrorRaw();
};

NAMESPACE_BANG_END

#endif // SYSTEMPROCESS_H
