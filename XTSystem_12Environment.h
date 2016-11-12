
namespace XTSystem
{
	class Environment
	{
	public: 
		static String CurrentDirectory()
		{
			Char buff[2000];
			GetCurrentDirectory(2000,buff);
			return String(buff);
		}

		static String OSVersion()
		{
			OSVERSIONINFO osvi;
			//BOOL bIsWindowsXPorLater;
			ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
#pragma warning(disable: 4996)
			GetVersionEx(&osvi);
#pragma warning(default: 4996)
			return String::Format(_T("Microsoft Windows NT {0}.{1}.{2} {3}"), osvi.dwMajorVersion,osvi.dwMinorVersion, osvi.dwBuildNumber, String(osvi.szCSDVersion));
			//return String(_T("Microsoft Windows NT 6.1.7601 Service Pack 1"));
		}

		static int ProcessorCount()
		{
			SYSTEM_INFO sysinfo;
			GetSystemInfo( &sysinfo );
			return sysinfo.dwNumberOfProcessors;
		}

		static String MachineName()
		{
			DWORD len;
			Char buff[200];
			GetComputerName(buff,&len);
			return String(buff);

		}

		static String NewLine()
		{
			return _T("\r\n");
		}
		
	};
}

