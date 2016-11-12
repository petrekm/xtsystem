
namespace XTSystem
{
    class ProcessInfo
    {
    };

	class Process
	{
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			String command;
			
			public: void SetCommand(const String &acommand)
			{
				command = acommand;
			}

			public: void Start()
			{
				Char *cmd = new Char[command.Length()+1];
				_tcsncpy_s(cmd, command.Length() + 1, command.c_str(), command.Length() + 1);

				ZeroMemory(&si, sizeof(si));
				si.cb = sizeof(si);
				ZeroMemory(&pi, sizeof(pi));

				// Start the child process. 
				if (!CreateProcess(NULL,   // No module name (use command line)
					cmd,        // Command line
					NULL,           // Process handle not inheritable
					NULL,           // Thread handle not inheritable
					FALSE,          // Set handle inheritance to FALSE
					0,              // No creation flags
					NULL,           // Use parent's environment block
					NULL,           // Use parent's starting directory 
					&si,            // Pointer to STARTUPINFO structure
					&pi)           // Pointer to PROCESS_INFORMATION structure
					)
				{
					delete []cmd;
					throw Exception(String::Format(L"CreateProcess failed ({0}).", GetLastError()));
				}

				// Wait until child process exits.
				WaitForSingleObject(pi.hProcess, INFINITE);

				// Close process and thread handles. 
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);

				delete []cmd;
			}
	};
}
