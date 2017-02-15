#undef GetCurrentDirectory
#undef SetCurrentDirectory
#undef CreateDirectory

namespace XTSystem
{
    namespace IO
    {
        class Directory
        {
            public: static std::vector<String> GetDirectories(const String &path, const String &searchPattern)
                    {
                        return GetFilesSelected(path, searchPattern, &IsDirectoryPriv);
                    }
            public: static std::vector<String> GetDirectories(const String &path)
                    {
                        return GetDirectories(path, _T("*"));
                    }
            public: static std::vector<String> GetFiles(const String &path, const String &searchPattern)
                    {
                        return GetFilesSelected(path, searchPattern, &IsFileOrLinkPriv);
                    }
            public: static std::vector<String> GetFiles(const String &path)
                {
                    return GetFiles(path, _T("*"));
                }

			public: static bool Exists(const String &path)
			{
				DWORD ftyp = GetFileAttributesW(path.c_str());
				if (ftyp == INVALID_FILE_ATTRIBUTES)
					return false;  //something is wrong with your path!

				if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
					return true;   // this is a directory!

				return false;    // this is not a directory!
			}

			public: static String GetCurrentDirectory_()
			{
				TCHAR path[MAX_PATH + 1] = L"";
				DWORD len = GetCurrentDirectoryW(MAX_PATH, path);

				String value(path);
				return value;
			}

			public: static void SetCurrentDirectory_(const String &path)
			{
				BOOL ok = SetCurrentDirectoryW(path.c_str());
				if (ok != TRUE)
				{
					throw Exception(L"Can't set current directory");
				}

			}
			//public: static String GetDirectoryRoot(const String &path)
			//{

			//}

			public: static /*DirectoryInfo*/ void CreateDirectory_(const String &path)
			{
				BOOL ok = CreateDirectoryW(path.c_str(), NULL);
				if (ok != TRUE)
				{
					throw Exception(L"Can't set current directory");
				}
			}

			public: static void Delete(const String &path)
			{
				BOOL ok = RemoveDirectory(path.c_str());
			}
			//public: static void Delete(const String &path, bool recursive)
			//{

			//}

/*			private: bool deleteDirectory(std::string& directoryname, int flags)
			{
				if (directoryname.at(directoryname.size() - 1) != '\\') directoryname += '\\';

				if ((flags & CONTENTS) == CONTENTS)
				{
					WIN32_FIND_DATAA fdata;
					HANDLE dhandle;
					//BUG 1: Adding a extra \ to the directory name..
					directoryname += "*";
					dhandle = FindFirstFileA(directoryname.c_str(), &fdata);
					//BUG 2: Not checking for invalid file handle return from FindFirstFileA
					if (dhandle != INVALID_HANDLE_VALUE)
					{
						// Loop through all the files in the main directory and delete files & make a list of directories
						while (true)
						{
							if (FindNextFileA(dhandle, &fdata))
							{
								std::string     filename = fdata.cFileName;
								if (filename.compare("..") != 0)
								{
									//BUG 3: caused by BUG 1 - Removing too many characters from string.. removing 1 instead of 2
									std::string filelocation = directoryname.substr(0, directoryname.size() - 1) + filename;

									// If we've encountered a directory then recall this function for that specific folder.

									//BUG 4: not really a bug, but spurious function call - we know its a directory from FindData already, use it.
									if ((fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
										DeleteFileA(filelocation.c_str());
									else
										deleteDirectory(filelocation, DIRECTORY_AND_CONTENTS);
								}
							}
							else if (GetLastError() == ERROR_NO_MORE_FILES)    break;
						}
						directoryname = directoryname.substr(0, directoryname.size() - 2);
						//BUG 5: Not closing the FileFind with FindClose - OS keeps handles to directory open.  MAIN BUG
						FindClose(dhandle);
					}
				}
				if ((flags & DIRECTORY) == DIRECTORY)
				{
					HANDLE DirectoryHandle;
					DirectoryHandle = CreateFileA(directoryname.c_str(),
						FILE_LIST_DIRECTORY,
						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
						NULL);
					//BUG 6: Not checking CreateFileA for invalid handle return.
					if (DirectoryHandle != INVALID_HANDLE_VALUE)
					{

						bool DeletionResult = (RemoveDirectoryA(directoryname.c_str()) != 0) ? true : false;
						CloseHandle(DirectoryHandle);
						return DeletionResult;
					}
					else
					{
						return true;
					}
				}

				return true;
			}
*/

			//public: static DirectoryInfo CreateDirectory(const String &path, DirectorySecurity directorySecurity);


            private: static bool IsFileOrLinkPriv(WIN32_FIND_DATA *ffd) { return (ffd->dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY)!=0;}
            private: static bool IsDirectoryPriv(WIN32_FIND_DATA *ffd) { return (ffd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0;}
            private: static std::vector<String> GetFilesSelected(const String &path, const String &searchPattern, bool (*predicate)(WIN32_FIND_DATA *))
                     {
                         std::vector<String> vec;

                         String searchSpecification = IO::Path::Combine(path, searchPattern);

                         WIN32_FIND_DATA ffd;
                         //LARGE_INTEGER filesize;
                         HANDLE hFind = INVALID_HANDLE_VALUE;
                         //DWORD dwError=0;

                         if (searchSpecification.Length()>=MAX_PATH) return vec;    // error

                         hFind = FindFirstFile(searchSpecification.c_str(), &ffd);

                         if (INVALID_HANDLE_VALUE == hFind) return vec;

                         do
                         {
                             if (predicate(&ffd)) vec.push_back(IO::Path::Combine(path,String(ffd.cFileName)));
                         } while (FindNextFile(hFind, &ffd) != 0);

                         return vec;
                     }
        };

/*		
        class DirectoryObsolete
        {
            //public: static DateTime GetCreationTime(const String &path);
            //public: static DateTime GetCreationTimeUtc(const String &path);
            //public: static DateTime GetLastAccessTime(const String &path);
            //public: static DateTime GetLastAccessTimeUtc(const String &path);
            //public: static DateTime GetLastWriteTime(const String &path);
            //public: static DateTime GetLastWriteTimeUtc(const String &path);
            //public: static DirectoryInfo CreateDirectory(const String &path);
            //public: static DirectoryInfo CreateDirectory(const String &path, DirectorySecurity directorySecurity);
            //public: static DirectoryInfo GetParent(const String &path);
            //public: static DirectorySecurity GetAccessControl(const String &path);
            //public: static DirectorySecurity GetAccessControl(const String &path, AccessControlSections includeSections);
            //public: static IEnumerable<String> EnumerateDirectories(const String &path);
            //public: static IEnumerable<String> EnumerateDirectories(const String &path, const String &searchPattern);
            //public: static IEnumerable<String> EnumerateDirectories(const String &path, const String &searchPattern, SearchOption searchOption);
            //public: static IEnumerable<String> EnumerateFileSystemEntries(const String &path);
            //public: static IEnumerable<String> EnumerateFileSystemEntries(const String &path, const String &searchPattern);
            //public: static IEnumerable<String> EnumerateFileSystemEntries(const String &path, const String &searchPattern, SearchOption searchOption);
            //public: static IEnumerable<String> EnumerateFiles(const String &path);
            //public: static IEnumerable<String> EnumerateFiles(const String &path, const String &searchPattern);
            //public: static IEnumerable<String> EnumerateFiles(const String &path, const String &searchPattern, SearchOption searchOption);
            //public: static bool Exists(const String &path);
            //public: static String GetCurrentDirectory();
            //public: static String GetDirectoryRoot(const String &path);

			private: static std::vector<String> GetFilesSelected(const String &path, bool (*predicate)(dirent::wdirent*))
					{
						    dirent::WDIR *dir = dirent::wopendir (path.c_str());
							if (dir == NULL) throw Exception(_T("Invalid path!"));

							std::vector<String> vec;
							struct dirent::wdirent *ent;
							while ((ent = dirent::wreaddir(dir)) != NULL) 
							{
								if (predicate(ent))	vec.push_back(Path::Combine(path,String(ent->d_name)));
							}
							dirent::wclosedir(dir);
							return vec;
					}

			private: static bool IsFileOrLinkPriv(dirent::wdirent* ent) { return ent->d_type==DT_REG || ent->d_type==DT_LNK; }
			private: static bool IsDirectoryPriv(dirent::wdirent* ent) { return ent->d_type==DT_DIR;}

            public: static std::vector<String> GetDirectories(const String &path)
					{
						return GetFilesSelected(path, &IsDirectoryPriv);
					}
            public: static std::vector<String> GetDirectories(const String &path, const String &searchPattern)
					{
						return GetFilesSelected(path, &IsDirectoryPriv);
					}
            //public: static String[] GetDirectories(const String &path, const String &searchPattern, SearchOption searchOption);
            //public: static String[] GetFileSystemEntries(const String &path);
            //public: static String[] GetFileSystemEntries(const String &path, const String &searchPattern);
            //public: static String[] GetFileSystemEntries(const String &path, const String &searchPattern, SearchOption searchOption);
		
			public: static std::vector<String> GetFiles(const String &path)
					{
						return GetFilesSelected(path, &IsFileOrLinkPriv);
					}

			//public: static String[] GetFiles(const String &path, const String &searchPattern);
            //public: static String[] GetFiles(const String &path, const String &searchPattern, SearchOption searchOption);
            //public: static String[] GetLogicalDrives();
            //public: static void Delete(const String &path);
            //public: static void Delete(const String &path, bool recursive);
            //public: static void Move(String sourceDirName, String destDirName);
            //public: static void SetAccessControl(const String &path, DirectorySecurity directorySecurity);
            //public: static void SetCreationTime(const String &path, DateTime creationTime);
            //public: static void SetCreationTimeUtc(const String &path, DateTime creationTimeUtc);
            //public: static void SetCurrentDirectory(const String &path);
            //public: static void SetLastAccessTime(const String &path, DateTime lastAccessTime);
            //public: static void SetLastAccessTimeUtc(const String &path, DateTime lastAccessTimeUtc);
            //public: static void SetLastWriteTime(const String &path, DateTime lastWriteTime);
            //public: static void SetLastWriteTimeUtc(const String &path, DateTime lastWriteTimeUtc);

        };
*/
    }
}


#define GetCurrentDirectory GetCurrentDirectoryW
#define SetCurrentDirectory SetCurrentDirectoryW
#define CreateDirectory CreateDirectoryW

