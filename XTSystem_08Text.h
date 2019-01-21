namespace XTSystem
{
    namespace Text
    {
        class StringBuilder 
        {
        public: 
            StringBuilder() {}
            void AppendLine(const String &line) 
            { 
                data.push_back(line + Console::EndLine()); 
            }
            void Append(const String &line) 
            { 
                data.push_back(line); 
            }
            String ToString() const
            {
				int size = (int)data.size();
				int totalsize = 0;
				for (int i = 0; i < size; i++) totalsize += data[i].Length();

				String s;
				s._reserve(totalsize + 1);

				for (int i = 0; i<size; i++) s.data += data[i].data;
				return s;

            }
            void Clear()
            {
                data.clear();
            }
        private:
            std::vector<String> data;
        };
    }
}

