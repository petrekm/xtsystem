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
                data.push_back(line); 
            }
            String ToString() const
            {
                String s = _T("");
				int size = (int)data.size();
                for (int i=0;i<size;i++) s+= data[i];
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

