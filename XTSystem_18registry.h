


namespace Microsoft
{

	namespace Win32
	{
		enum RegistryValueKind
		{
			None = -1,
			Unknown = 0,
			String_reg = 1,
			ExpandString = 2,
			Binary = 3,
			DWord = 4,
			MultiString = 7,
			QWord = 11,
		};

		union convert_BYTE_DWORD
		{
			DWORD dword_value;
			BYTE bytes[sizeof(DWORD)];
		};

		class RegistryObject
		{
			public:
				RegistryObject(const RegistryObject &ro):length(ro.length),kind(ro.kind)
				{
					data = new BYTE[length];
					memcpy(data,ro.data,length*sizeof(BYTE));
				}
			~RegistryObject() { if (NULL!=data) delete []data; }
			static RegistryObject Create(DWORD _type, BYTE *_data, DWORD _length)
			{
				if (_type==REG_DWORD) return RegistryObject(RegistryValueKind::DWord, _data, _length); 
				//if (_type==REG_QWORD) return RegistryObject(RegistryValueKind::QWord, _data,sizeof(QWORD)); 
				if (_type==REG_SZ) return RegistryObject(RegistryValueKind::String_reg,_data,_length); 
				if (_type==REG_MULTI_SZ) return RegistryObject(RegistryValueKind::MultiString,_data,_length); 
				if (_type==REG_BINARY) return RegistryObject(RegistryValueKind::Binary,_data,_length); 
				if (_type==REG_EXPAND_SZ) return RegistryObject(RegistryValueKind::ExpandString,_data,_length); 


				throw XTSystem::Exception(_T("Wrong registry type"));
			}
			operator int()
			{
				if (kind == RegistryValueKind::DWord) 
				{
					return (int)convertToDWORD();
				}
				throw XTSystem::Exception(_T("can't convert to int"));
			}
			private: RegistryObject(RegistryValueKind _kind, BYTE *_data, DWORD _length): kind(_kind), length(_length), data(_data) 
			{	
			}

			RegistryValueKind kind;
			DWORD length;
			BYTE *data;
		public: 
			RegistryValueKind Kind() const { return kind; }
			XTSystem::String ToString() const
			{
				if (kind == RegistryValueKind::DWord) 
				{ 
					DWORD d = convertToDWORD();
					if ((int)d<0)
					{
						return XTSystem::String::Format(_T("{0}"),(int)d);
					}
					return XTSystem::String::Format(_T("{0}"),d);
				}
				if (kind == RegistryValueKind::String_reg) 
				{ 
					XTSystem::Char *str = (XTSystem::Char *) (data);
					return XTSystem::String::Format(_T("{0}"),str);
				}
					
				return _T("");
			}
		private: DWORD convertToDWORD() const
				 {
					convert_BYTE_DWORD convert;
					for (int i=0;i<sizeof(DWORD);++i) convert.bytes[i] = data[i];
					return convert.dword_value;
				 }

		};

		class RegistryKey
		{
			public: ~RegistryKey()
					{
						Close();
					}
			private: RegistryKey(HKEY _hKey, const XTSystem::String &_name, const XTSystem::String &_relativePathToParent, bool _writable = false)
			{
				hKey = _hKey;
				name = _name;
				relativePathToParent = _relativePathToParent;
				writable = _writable;
				//GetInfo();
			}

			HKEY hKey;
			XTSystem::String name;
			XTSystem::String relativePathToParent;
			bool writable;
			//public: SafeRegistryHandle Handle() { get; }
			public: XTSystem::String Name() { return name; }
			public: XTSystem::String RegistryKeyPath() { return relativePathToParent; }
			public: int SubKeyCount() 
					{ 
						DWORD dwNumberOfSubKeys(0);
						DWORD retCode = RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwNumberOfSubKeys, 
														NULL, NULL, NULL, NULL, NULL, NULL, NULL);
						if (retCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't get number of subkeys"));
						return dwNumberOfSubKeys;
					}
			public: int ValueCount() 
					{ 
						DWORD dwNumberOfValues(0);
						DWORD retCode = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, &dwNumberOfValues,
														NULL, NULL, NULL, NULL, NULL);
						if (retCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't get number of values"));
						return dwNumberOfValues;
					}

			//public: RegistryView View { get; }

			public: void Close()
					{
						if (NULL!=hKey) RegCloseKey(hKey);
						hKey = NULL;
					}
			public:
				RegistryKey *OpenSubKey(const XTSystem::String &path, bool writable = false)
				{
					HKEY hSubKey;
					REGSAM mode = (writable)?KEY_WRITE|KEY_READ:KEY_READ;
					if( RegOpenKeyEx( hKey,path.c_str(), 0, mode, &hSubKey) == ERROR_SUCCESS)
				    {
						XTSystem::String name = GetNameFromRegistryPath(path);
						XTSystem::String relpathtoparrent = relativePathToParent + _T('\\') + path;
						return new RegistryKey(hSubKey,name,relpathtoparrent,writable);
				    }
					throw XTSystem::Exception(_T("Can't find registry key"));
					return NULL;
				}

			public:
				RegistryKey *CreateSubKey(const XTSystem::String &subkey)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));

					HKEY hSubKey;
					LONG retcode = RegCreateKeyEx(hKey, subkey.c_str(), 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, NULL );
					if (retcode != ERROR_SUCCESS)
					{
						throw XTSystem::Exception(_T("Key already exist"));
					}
		
					XTSystem::String name = GetNameFromRegistryPath(subkey);
					XTSystem::String relpathtoparrent = relativePathToParent + _T('\\') + subkey;
					return new RegistryKey(hSubKey,name,relpathtoparrent, true);
				}
			public:

				void SetValue(const XTSystem::String &name, const XTSystem::Char*value)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));

					LONG RetCode = RegSetValueEx (hKey, name.c_str(), NULL, REG_SZ, (LPBYTE)value, (DWORD)((_tcslen(value)+1)*sizeof(XTSystem::Char)));
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));

				}
				void SetValue(const XTSystem::String &name, const XTSystem::String &value)
				{	
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));

					LONG RetCode = RegSetValueEx (hKey, name.c_str(), NULL, REG_SZ, (LPBYTE)value.c_str(), (DWORD)((value.Length()+1)*sizeof(XTSystem::Char)));
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));

				}

                void SetValue(const XTSystem::String &name, XTSystem::Char value)
                {
                    if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
                    SetValue(name, XTSystem::String()+value);
                }

				void SetValue(const XTSystem::String &name, bool value)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::Bool::ToString(value));
				}
				void SetValue(const XTSystem::String &name, double value)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					double abs_value = XTSystem::Math::Abs(value);
					if (abs_value>1e-4 && abs_value <1e10)
					{
						SetValue(name, XTSystem::String::Format(_T("{0:15.14f}"),value));
					} else
					{
						SetValue(name, XTSystem::String::Format(_T("{0:15.14E}"),value));
					}
				}

				void SetValue(const XTSystem::String &name, short data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::String::Format(_T("{0}"),data));
				}

				void SetValue(const XTSystem::String &name, int value)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					DWORD data = (DWORD)value;
					//LPBYTE bytes = (LPBYTE)&data;
					LONG RetCode = RegSetValueEx(hKey, name.c_str(), NULL, REG_DWORD, (LPBYTE)&data, sizeof(data));
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));
				}

				void SetValue(const XTSystem::String &name, long data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::String::Format(_T("{0}"),data));
				}

				void SetValue(const XTSystem::String &name, long long data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::String::Format(_T("{0}"),data));
				}

				void SetValue(const XTSystem::String &name, unsigned short data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::String::Format(_T("{0}"),data));
				}

				void SetValue(const XTSystem::String &name, unsigned int data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::String::Format(_T("{0}"),data));
				}

				void SetValue(const XTSystem::String &name, unsigned long data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::String::Format(_T("{0}"),data));
				}


				void SetValue(const XTSystem::String &name, const XTSystem::Collections::Generic::Arr<BYTE> &data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					BYTE *arr=new BYTE[data.Length()];
					for (int i=0;i<data.Length();i++) arr[i]=data[i];

					LONG RetCode = RegSetValueEx(hKey, name.c_str(), NULL, REG_BINARY, arr, data.Length());
					XTSystem::safe_delete_array(arr);
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));
				}

				void SetValue(const XTSystem::String &name, const BYTE*data, int length)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					LONG RetCode = RegSetValueEx(hKey, name.c_str(), NULL, REG_BINARY, data, length);
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));
				}

				private: XTSystem::Char *accumulate(const XTSystem::String*data, int length, int *pSize)
						 {
							int size = 1;
							for (int i=0;i<length; ++i) size += data[i].Length()+1;
							XTSystem::Char *arr = new XTSystem::Char[size];
							XTSystem::Char *arr1 = arr;
							for (int i=0;i<length; ++i)
							{
								#pragma warning(disable: 4996)
								_tcsncpy(arr1, data[i].c_str(), data[i].Length());
								#pragma warning(default: 4996)
								arr1 += data[i].Length();
								*arr1 = _T('\0'); ++arr1;
							}
							*arr1 = _T('\0');
							*pSize = size;
							return arr;
						 }

				private: XTSystem::Char *accumulate(const XTSystem::Collections::Generic::Arr<XTSystem::String> &data, int *pSize)
						 {
							int size = 1;
							for (int i=0;i<data.Length(); ++i) size += data[i].Length()+1;
							XTSystem::Char *arr = new XTSystem::Char[size];
							XTSystem::Char *arr1 = arr;
							for (int i=0;i<data.Length(); ++i)
							{
								#pragma warning(disable: 4996)
								_tcsncpy(arr1, data[i].c_str(), data[i].Length());
								#pragma warning(default: 4996)
								arr1 += data[i].Length();
								*arr1 = _T('\0'); ++arr1;
							}
							*arr1 = _T('\0');
							*pSize = size;
							return arr;
						 }
						 
				public: void SetValue(const XTSystem::String &name, const XTSystem::String*data, int length)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					int size = 0;
					XTSystem::Char *arr = accumulate(data, length, &size);
					LONG RetCode = RegSetValueEx(hKey, name.c_str(), NULL, REG_MULTI_SZ, (LPBYTE)arr, (DWORD)((size)*sizeof(XTSystem::Char)));
					XTSystem::safe_delete_array(arr);
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));
				}

				void SetValue(const XTSystem::String &name, const XTSystem::Collections::Generic::Arr<XTSystem::String> &data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					int size = 0;
					XTSystem::Char *arr = accumulate(data, &size);
					LONG RetCode = RegSetValueEx(hKey, name.c_str(), NULL, REG_MULTI_SZ, (LPBYTE)arr, (DWORD)((size)*sizeof(XTSystem::Char)));
					XTSystem::safe_delete_array(arr);
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));
				}

				public: XTSystem::Collections::Generic::Arr<XTSystem::String> GetSubKeyNames()
					{
						int numSubkeys = SubKeyCount(); 
						XTSystem::Collections::Generic::Arr<XTSystem::String> arr(numSubkeys);
						if (0 == numSubkeys) return arr;

						TCHAR bufSubKeyName[MAX_KEY_LENGTH];
						for (int i=0; i<numSubkeys; i++) 
						{ 
							DWORD dwSizeOfSubKeyName = MAX_KEY_LENGTH;
							DWORD retCode1 = RegEnumKeyEx(hKey, i, bufSubKeyName, &dwSizeOfSubKeyName, NULL, NULL, NULL, NULL); 
							if (retCode1 == ERROR_SUCCESS) arr[i] = bufSubKeyName;
						}

						return arr;

					}
				public: XTSystem::Collections::Generic::Arr<XTSystem::String> GetValueNames()
					{
						int numValues = ValueCount(); 
						XTSystem::Collections::Generic::Arr<XTSystem::String> arr(numValues);
						if (0 == numValues) return arr;

						TCHAR bufValueName[MAX_VALUE_NAME]; 
						for (int i=0; i<numValues; i++) 
						{ 
							DWORD dwSizeOfValueName = MAX_VALUE_NAME; 
							DWORD retCode1 = RegEnumValue(hKey, i, bufValueName, &dwSizeOfValueName, NULL, NULL, NULL, NULL);
 							if (retCode1 == ERROR_SUCCESS) arr[i] = bufValueName;
						}

						return arr;

					}
				public: RegistryValueKind GetValueKind(const XTSystem::String &name)
						{
							DWORD type = REG_SZ;
							DWORD len = 0;
							LONG RetCode = RegQueryValueEx(hKey,name.c_str(), NULL, &type, NULL, &len);
							if (RetCode != ERROR_SUCCESS) return RegistryValueKind::None;
							if (type==REG_SZ) return RegistryValueKind::String_reg;
							if (type==REG_DWORD) return RegistryValueKind::DWord;
							if (type==REG_MULTI_SZ) return RegistryValueKind::MultiString;
							if (type==REG_QWORD) return RegistryValueKind::QWord;
							if (type==REG_EXPAND_SZ) return RegistryValueKind::ExpandString;
							if (type==REG_BINARY) return RegistryValueKind::Binary;
							return RegistryValueKind::Unknown;
						}

				public: RegistryObject GetValue(const XTSystem::String &name)
						{
							DWORD value_length = 0;
							DWORD type = REG_NONE;
							LONG RetCode = RegQueryValueEx(hKey,name.c_str(), NULL, &type, NULL, &value_length);
							if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("Can't read registry value"));
							BYTE *data = new BYTE[value_length];
							RetCode = RegQueryValueEx(hKey,name.c_str(), NULL, &type, data, &value_length);
							return RegistryObject::Create(type, data, value_length);
						}

				friend class Registry;
  
				static const int MAX_KEY_LENGTH = 255;
				static const int MAX_VALUE_NAME = 16383;
				static const int MAX_REGISTRY_PATH = 1000;

				private: static XTSystem::String GetNameFromRegistryPath(const XTSystem::String &path)
                    {
                        int k = XTSystem::Math::Max(path.LastIndexOf(_T('\\')), path.LastIndexOf(_T('/')));
                        if (k>=0) return path.Substring(k+1);
                        return path;
                    }
                /*
				void GetInfo()
				{
					TCHAR    bufClassName[MAX_REGISTRY_PATH];	     // don't know what is it
					DWORD    dwSizeOfClassName = MAX_REGISTRY_PATH;  // don't know what is it
					DWORD    dwNumberOfSubKeys = 0;               
					DWORD    dwSizeOfLongestSubKeyName;         
					DWORD    dwSizeOfLongestClassName;    // don't know what is it
					DWORD    dwNumberOfValues;     
					DWORD    dwSizeOfLongestValueName; 
					DWORD    dwSizeOfLongestValueData;   // don't know what is it
					DWORD    dwSizeOfSecurityDescriptor; // don't know what is it
					FILETIME ftLastWriteTime;			 // last write time 
 
					// Get the class name and the value count. 
					DWORD retCode = RegQueryInfoKey(
						hKey,                    
						bufClassName,           
						&dwSizeOfClassName,      
						NULL,                    // reserved 
						&dwNumberOfSubKeys,         
						&dwSizeOfLongestSubKeyName, 
						&dwSizeOfLongestClassName, 
						&dwNumberOfValues,
						&dwSizeOfLongestValueName,
						&dwSizeOfLongestValueData,  
						&dwSizeOfSecurityDescriptor,
						&ftLastWriteTime);       // last write time 
				}
                */
		};

		class Registry
		{
			public: 
				static RegistryKey &ClassesRoot() 
				{ 
						static RegistryKey _ClassesRoot(HKEY_CLASSES_ROOT,_T("HKEY_CLASSES_ROOT"),_T("HKEY_CLASSES_ROOT")); 
						return _ClassesRoot; 
				}
				static RegistryKey &CurrentConfig() 
				{ 
					static RegistryKey _CurrentConfig(HKEY_CURRENT_CONFIG,_T("HKEY_CURRENT_CONFIG"),_T("HKEY_CURRENT_CONFIG")); 
					return _CurrentConfig; 
				}
				static RegistryKey &CurrentUser() 
				{ 
					static RegistryKey _CurrentUser(HKEY_CURRENT_USER,_T("HKEY_CURRENT_USER"),_T("HKEY_CURRENT_USER")); 
					return _CurrentUser; 
				}
				static RegistryKey &DynData() 
				{ 
					static RegistryKey _DynData(HKEY_DYN_DATA,_T("HKEY_DYN_DATA"),_T("HKEY_DYN_DATA")); 
					return _DynData; 
				}
				static RegistryKey &LocalMachine() 
				{ 
					static RegistryKey _LocalMachine(HKEY_LOCAL_MACHINE,_T("HKEY_LOCAL_MACHINE"),_T("HKEY_LOCAL_MACHINE")); 
					return _LocalMachine; 
				}
				static RegistryKey &PerformanceData() 
				{ 
					static RegistryKey _PerformanceData(HKEY_PERFORMANCE_DATA,_T("HKEY_PERFORMANCE_DATA"),_T("HKEY_PERFORMANCE_DATA")); 
					return _PerformanceData; 
				}
				static RegistryKey &Users() 
				{ 
					static RegistryKey _Users(HKEY_USERS,_T("HKEY_USERS"),_T("HKEY_USERS")); 
					return _Users; 
				}
		};

	}
}

