#include <Windows.h>
#include <Lmcons.h>
#include <Sddl.h>

#include "Communication.h"

namespace
{
  std::string AppendErrorCode(const std::string& message)
  {
    std::stringstream stream;
    stream << message << " (Error code: " << GetLastError() << ")";
    return stream.str();
  }

  std::wstring GetUserName()
  {
    const DWORD maxLength = UNLEN + 1;
    std::auto_ptr<wchar_t> buffer(new wchar_t[maxLength]);
    DWORD length = maxLength;
    if (!::GetUserNameW(buffer.get(), &length))
      throw std::runtime_error(AppendErrorCode("Failed to get the current user's name"));
    return std::wstring(buffer.get(), length);
  }
}

const std::wstring Communication::pipeName = L"\\\\.\\pipe\\adblockplusengine_" + GetUserName();

Communication::PipeConnectionError::PipeConnectionError()
    : std::runtime_error(AppendErrorCode("Unable to connect to a named pipe"))
{
}

Communication::Pipe::Pipe(const std::wstring& pipeName, Communication::Pipe::Mode mode)
{
  pipe = INVALID_HANDLE_VALUE;
  if (mode == MODE_CREATE)
  {
    SECURITY_ATTRIBUTES sa;
    memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);

    // Low mandatory label. See http://msdn.microsoft.com/en-us/library/bb625958.aspx
    LPCWSTR accessControlEntry = L"S:(ML;;NW;;;LW)";
    PSECURITY_DESCRIPTOR securitydescriptor;
    ConvertStringSecurityDescriptorToSecurityDescriptorW(accessControlEntry, SDDL_REVISION_1, &securitydescriptor, 0);

    sa.lpSecurityDescriptor = securitydescriptor;
    sa.bInheritHandle = TRUE;

    pipe = CreateNamedPipeW (pipeName.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                  PIPE_UNLIMITED_INSTANCES, Communication::bufferSize, Communication::bufferSize, 0, &sa);
    LocalFree(securitydescriptor);
  }
  else
  {
    if (WaitNamedPipeW(pipeName.c_str(), 5000))
      pipe = CreateFileW(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
  }

  if (pipe == INVALID_HANDLE_VALUE)
    throw PipeConnectionError();

  DWORD pipeMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
  if (!SetNamedPipeHandleState(pipe, &pipeMode, 0, 0))
    throw std::runtime_error("SetNamedPipeHandleState failed: error " + GetLastError());

  if (mode == MODE_CREATE && !ConnectNamedPipe(pipe, 0))
    throw std::runtime_error("Client failed to connect: error " + GetLastError());
}

Communication::Pipe::~Pipe()
{
  CloseHandle(pipe);
}

Communication::InputBuffer Communication::Pipe::ReadMessage()
{
  std::stringstream stream;
  std::auto_ptr<char> buffer(new char[bufferSize]);
  bool doneReading = false;
  while (!doneReading)
  {
    DWORD bytesRead;
    if (ReadFile(pipe, buffer.get(), bufferSize * sizeof(char), &bytesRead, 0))
      doneReading = true;
    else if (GetLastError() != ERROR_MORE_DATA)
    {
      std::stringstream stream;
      stream << "Error reading from pipe: " << GetLastError();
      throw std::runtime_error(stream.str());
    }
    stream << std::string(buffer.get(), bytesRead);
  }
  return Communication::InputBuffer(stream.str());
}

void Communication::Pipe::WriteMessage(Communication::OutputBuffer& message)
{
  DWORD bytesWritten;
  std::string data = message.Get();
  if (!WriteFile(pipe, data.c_str(), data.length(), &bytesWritten, 0))
    throw std::runtime_error("Failed to write to pipe");
}