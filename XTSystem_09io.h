
namespace XTSystem
{
    namespace IO
    {
        class StreamReader
        {
            public: StreamReader(const String &path)
                {
                    isopened=false;
        #pragma warning(disable: 4996)
                    fin = _tfopen(path.c_str(),_T("r"));
        #pragma warning(default: 4996)
                    if (!fin) throw Exception(_T("Can't open file"));
                    isopened=true;
                }

            public: ~StreamReader()
                {
                    Close();
                }

            private: bool GetLine(String &line, bool &endOfFile)
                {
                    Char buffer[2000];
                    int i=1;
                    Char *ptr(NULL);
                    line = _T("");
                    endOfFile = false;
                    for (;;) //while (true) 
                    {
                        ptr = _fgetts(buffer, 2000, fin);
                        if (NULL==ptr)
                        {
                            if (i==1)
                            {
                                return false;
                            }
                            endOfFile = true;
                            return true;
                        }

                        line += String(buffer);
                        int len = line.Length();
                        if (len>0)
                        {
                            if (line[len-1]==_T('\n'))
                            {
                                line = line.TrimEnd(_T("\r\n"));
                                return true;
                            }
                        }

                        i++;
                    }
                    return false;
                }

            public: String ReadLine()
                {
                    String s;
                    bool endOfFile(false);
                    bool bOk = GetLine(s, endOfFile);
                    if (!bOk) throw Exception(_T("System.IO.IOException"));
                    return s;
                }
            public: String ReadToEnd()
                {
                    return _T("");
                }


            public: void Close()
                {
                    if (isopened) { fclose(fin); fin=NULL; isopened=false; }
                }
            private: FILE *fin;
                bool isopened;
                friend class File;
        };





        class StreamWriter
        {
        public: StreamWriter(const String &path)
                {
                    Open(path);
                }

        public: StreamWriter()
                {
                    autoflush = false;
                    isopened = false;
                }

                void Open(const String &path)
                {
                    if (isopened) throw Exception(_T("File already opened"));
                    autoflush = false;
                    isopened = false;
        #pragma warning(disable: 4996)
                    fout = _tfopen(path.c_str(), _T("w"));
        #pragma warning(default: 4996)
                    if (!fout) throw Exception(_T("Can't open file"));
                    isopened = true;
                }

            public: ~StreamWriter()
                {
                    Close();
                }
            public: void WriteLine(const String &text)
                {
                    if (!isopened)   throw Exception(_T("File not opened"));
                    int numchars = _fputts(text.c_str(), fout);
                    numchars = _fputts(_T("\n"), fout);
                    (numchars);
                    if (autoflush) Flush();
                }
                    void Flush()
                    {
                        fflush(fout);
                    }

                TEMP_ARG_1 void WriteLine(const String &text, TCREF_ARG_1) { WriteLine(String::Format(text,ARG_1)); }
                TEMP_ARG_2 void WriteLine(const String &text, TCREF_ARG_2) { WriteLine(String::Format(text,ARG_2)); }
                TEMP_ARG_3 void WriteLine(const String &text, TCREF_ARG_3) { WriteLine(String::Format(text,ARG_3)); }
                TEMP_ARG_4 void WriteLine(const String &text, TCREF_ARG_4) { WriteLine(String::Format(text,ARG_4)); }
                TEMP_ARG_5 void WriteLine(const String &text, TCREF_ARG_5) { WriteLine(String::Format(text,ARG_5)); }
                TEMP_ARG_6 void WriteLine(const String &text, TCREF_ARG_6) { WriteLine(String::Format(text,ARG_6)); }
                TEMP_ARG_7 void WriteLine(const String &text, TCREF_ARG_7) { WriteLine(String::Format(text,ARG_7)); }
                TEMP_ARG_8 void WriteLine(const String &text, TCREF_ARG_8) { WriteLine(String::Format(text,ARG_8)); }
                TEMP_ARG_9 void WriteLine(const String &text, TCREF_ARG_9) { WriteLine(String::Format(text,ARG_9)); }
                TEMP_ARG_10 void WriteLine(const String &text, TCREF_ARG_10) { WriteLine(String::Format(text,ARG_10)); }
            public: void Write(const String &text)
                {
                    if (!isopened)   throw Exception(_T("File not opened"));

                    int numchars = _fputts(text.c_str(), fout);
                    (numchars);
                    if (autoflush) Flush();
                }
                TEMP_ARG_1 void Write(const String &text, TCREF_ARG_1) { Write(String::Format(text,ARG_1)); }
                TEMP_ARG_2 void Write(const String &text, TCREF_ARG_2) { Write(String::Format(text,ARG_2)); }
                TEMP_ARG_3 void Write(const String &text, TCREF_ARG_3) { Write(String::Format(text,ARG_3)); }
                TEMP_ARG_4 void Write(const String &text, TCREF_ARG_4) { Write(String::Format(text,ARG_4)); }
                TEMP_ARG_5 void Write(const String &text, TCREF_ARG_5) { Write(String::Format(text,ARG_5)); }
                TEMP_ARG_6 void Write(const String &text, TCREF_ARG_6) { Write(String::Format(text,ARG_6)); }
                TEMP_ARG_7 void Write(const String &text, TCREF_ARG_7) { Write(String::Format(text,ARG_7)); }
                TEMP_ARG_8 void Write(const String &text, TCREF_ARG_8) { Write(String::Format(text,ARG_8)); }
                TEMP_ARG_9 void Write(const String &text, TCREF_ARG_9) { Write(String::Format(text,ARG_9)); }
                TEMP_ARG_10 void Write(const String &text, TCREF_ARG_10) { Write(String::Format(text,ARG_10)); }


            public: void Close()
                {
                    if (isopened) { fclose(fout); fout = NULL; isopened = false; }
                }
            public: bool AutoFlush() { return autoflush;}
            public: void AutoFlush(bool value) { autoflush = value;}
            private: FILE *fout;
                bool isopened;
                bool autoflush;
        };

		class BinaryWriter
		{
			public: BinaryWriter(const String &path)
			{
				Open(path);
			}

			public: BinaryWriter()
			{
				autoflush = false;
				isopened = false;
			}

				void Open(const String &path)
				{
					autoflush = false;
					isopened = false;
#pragma warning(disable: 4996)
					fout = _tfopen(path.c_str(), _T("wb"));
#pragma warning(default: 4996)
					if (!fout) throw Exception(_T("Can't open file"));
					isopened = true;
				}

			public: ~BinaryWriter()
			{
				Close();
			}

			void Flush()
			{
				fflush(fout);
			}
			
			/*
			public: void Write(const Char *text)
			{
				if (!isopened)   throw Exception(_T("File not opened"));
				int len = (int)_tcslen(text);
				
					if (len < 256)
					{
						byte byte_len = (byte)len;
						fwrite(&byte_len, sizeof(byte), 1, fout);
					}
					else if (len < 65536)
					{
						unsigned short ulen = (unsigned short)(len);
						fwrite(&ulen, sizeof(unsigned short), 1, fout);
					}
					else
					{
						fwrite(&len, sizeof(int), 1, fout);
					}
				fwrite(text, sizeof(Char)*len, 1, fout);
				if (autoflush) Flush();
			}

			public: void Write(const std::vector<byte>&vec)
			{
				if (!isopened)   throw Exception(_T("File not opened"));
				int len = (int)vec.size();
				byte *arr = new byte[len];
				for (int i = 0; i < len; i++)arr[i] = vec[i];
				fwrite(&len, sizeof(int), 1, fout);
				fwrite(arr, sizeof(byte)*len, 1, fout);
				delete[]arr;
				if (autoflush) Flush();
			}
		
			public: void Write(const String &text)
			{
				if (!isopened)   throw Exception(_T("File not opened"));
				fwrite(text.c_str(), sizeof(Char)*text.Length(), 1, fout);
				if (autoflush) Flush();
			}
*/
			public: template<class T> void Write(const T &data)
			{
				if (!isopened)   throw Exception(_T("File not opened"));
				fwrite(&data, sizeof(T), 1, fout);
				if (autoflush) Flush();
			}

			public: template<class T>void Write(T *data, int length)
			{
				if (!isopened)   throw Exception(_T("File not opened"));
				fwrite(data, sizeof(T), length, fout);
				if (autoflush) Flush();
			}
		//	public: void Write(const unsigned char &data)
		//	{
		//		if (!isopened)   throw Exception(_T("File not opened"));
		//		fwrite(&data, sizeof(unsigned char), 1, fout);
		//		if (autoflush) Flush();
		//	}

		//	public: void Write(const long &data)
		//	{
		//		if (!isopened)   throw Exception(_T("File not opened"));
		//		fwrite(&data, sizeof(long), 1, fout);
		//		if (autoflush) Flush();
		//	}
		//public: void Write(const bool &data)
		//{
		//	if (!isopened)   throw Exception(_T("File not opened"));
		//	fwrite(&data, sizeof(bool), 1, fout);
		//	if (autoflush) Flush();
		//}
		//public: void Write(const double &data)
		//	{
		//		if (!isopened)   throw Exception(_T("File not opened"));
		//		fwrite(&data, sizeof(double), 1, fout);
		//		if (autoflush) Flush();
		//	}

			//TEMP_ARG_1 void Write(const String &text, TCREF_ARG_1) { Write(String::Format(text, ARG_1)); }



			public: void Close()
			{
				if (isopened) { fclose(fout); fout = NULL; isopened = false; }
			}
			public: bool AutoFlush() { return autoflush; }
			public: void AutoFlush(bool value) { autoflush = value; }
			private: FILE *fout;
					 bool isopened;
					 bool autoflush;
		};

        class Path
                {

                    //public: static bool HasExtension(const String &path);
                    public: static bool IsPathRooted(const String &path)
                            {
                                if (path.StartsWith(Path::DirectorySeparatorChar()) || path.StartsWith(Path::AltDirectorySeparatorChar())) return true;
                                if (path.Length()>=2 && path[1]==Path::VolumeSeparatorChar() ) return true;
                                //TODO:  && path[0] should be some letter a-z A-Z

                                return false;
                            }
                    private: static bool IsFullPathWithDrive(const String &path)
                            {
                                if (path.Length()>=2 && path[1]==Path::VolumeSeparatorChar() ) return true;
                                //TODO:  && path[0] should be some letter a-z A-Z

                                return false;
                            }
                    //public: static char[] GetInvalidFileNameChars();
                    //public: static char[] GetInvalidPathChars();
                    public: static Char AltDirectorySeparatorChar() { return _T('/'); }
                    public: static Char DirectorySeparatorChar() { return _T('\\'); }
                    public: static Char PathSeparator() { return _T(';'); }
                    public: static Char VolumeSeparatorChar() { return _T(':'); }
                    public: static std::vector<Char> InvalidPathChars() 
                            { 
                                static int call=0;
                                static std::vector<Char> v; 
                                if (call==0)
                                {
                                Char chars[] = { 34, 60, 62, 124, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                                                17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };
                                    v = std::vector<Char>(chars,chars + sizeof(chars)/ sizeof(chars[0]));
                                }
                                ++call;
                                return v;
                            };

                    //public: static String ChangeExtension(const String &path, String extension);
                    //public: static String Combine(params String[] paths);
                    public: static String Combine(const String &path1, const String &path2)
                            {
                                bool bPath1EndsWithSlash = path1.EndsWith(Path::DirectorySeparatorChar()) || path1.EndsWith(Path::AltDirectorySeparatorChar());
                                bool bPath2StartsWithSlash = path2.StartsWith(Path::DirectorySeparatorChar()) || path2.StartsWith(Path::AltDirectorySeparatorChar());
                                bool bPath2IsFullPathWithDrive = Path::IsFullPathWithDrive(path2);

                                if (path1.Length()==0) return path2;
                                if (path2.Length()==0) return path1;

                                if (bPath2IsFullPathWithDrive) return path2;
                                if (!bPath1EndsWithSlash && !bPath2StartsWithSlash) return path1 + Path::DirectorySeparatorChar() + path2;
                                if (bPath1EndsWithSlash && !bPath2StartsWithSlash) return path1 + path2;
                                if (!bPath1EndsWithSlash && bPath2StartsWithSlash) return path1 + path2;
                                if (bPath1EndsWithSlash && bPath2StartsWithSlash) 
                                {
                                        return path1.Substring(0,path1.Length()-1) + Path::DirectorySeparatorChar() + path2.Substring(1);
                                }
                                return path2;
                            }
                    //public: static String Combine(const String &path1, const String &path2, const String &path3);
                    //public: static String Combine(const String &path1, const String &path2, const String &path3, const String &path4);
                    public: static String GetDirectoryName(const String &path)
                    {
                        String ret;
                        int k1 = Math::Max(path.LastIndexOf(DirectorySeparatorChar()),path.LastIndexOf(AltDirectorySeparatorChar()));
                        if (k1 >= 0)
                        {
                            ret = path.Substring(0, k1);
                            if (ret.Length() <= 2)  // 'C:\'
                            {
                                ret.Clear();
                            }
                        }
                        return ret;
                    }

                    //public: static String GetExtension(const String &path);
                    public: static String GetFileName(const String &path)
                    {
                        int k = Math::Max(path.LastIndexOf(DirectorySeparatorChar()), path.LastIndexOf(AltDirectorySeparatorChar()));
                        if (k>=0) return path.Substring(k+1);
                        return path;
                    }

                    

                    //public: static String GetFileNameWithoutExtension(const String &path);
                    //public: static String GetFullPath(const String &path);
                    //public: static String GetPathRoot(const String &path);
                    //public: static String GetRandomFileName();
                    //public: static String GetTempFileName();
                    //public: static String GetTempPath();
                };


        class File
        {

            public: static std::vector<String> ReadAllLines(const String &path)
                {
                    std::vector<String> v;
                    StreamReader sr(path);
                    for (;;) //while (true) 
                    {
                        String line;
                        bool endOfFile(false);
                        bool bOk = sr.GetLine(line, endOfFile);

                        if (!bOk)
                        {
                            if (v.size()==0) throw Exception(_T("System.IO.IOException"));

                            return v;
                        }
                        v.push_back(line);
                        if (endOfFile) return v;
                    }
                    return v;
                }
        };
    }
}
