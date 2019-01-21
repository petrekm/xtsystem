#include <map>
#include <list>

template <typename T, std::size_t N>char (&sizeof_array( T(&)[N] ))[N];    // declared, undefined
#define ARR_SIZE(x) sizeof(sizeof_array(x))
#define Array_Init(TYPE, A, ...) TYPE A##_init[] = __VA_ARGS__ ;  Arr<TYPE> A(A##_init,ARR_SIZE(A##_init));

namespace XTSystem
{
	namespace Collections
	{
			namespace Generic
			{
				template<class TValue>
				class List
				{
						typedef std::list<TValue> T_List;
						T_List m_vecValues;

						public: List(int capacity)
								{
									m_vecValues.reserve(capacity);
								}
						public: List()
						{

						}
						public: void Add(const TValue &val)
								{
									m_vecValues.push_back(val);
								}

						public: int BinarySearch(const TValue &item)
						{
							return -1;
						}
						public: int Count() const
							{
								return (int)m_vecValues.size();
							}
						public: int Capacity() const
							{
								return (int)m_vecValues.capacity();
							}
						public: TValue &operator[](const int in_index)
								{
									if (in_index<0 || in_index>=(int)m_vecValues.size())
										throw Exception(_T("Out of bounds"));
									return m_vecValues.at(in_index); 
								}
	

						
						public: typedef typename T_List::iterator iterator;
						public: typedef typename T_List::const_iterator const_iterator;
						public: iterator begin() { return m_vecValues.begin(); }
						public: iterator end() { return m_vecValues.end(); }
				};


				template<class TValue>
				class Arr
				{
						typedef std::vector<TValue> T_Arr;
						T_Arr m_vecValues;

						public: Arr(int length)
								{
									m_vecValues.reserve(length);
									for (int i=0;i<length;++i) m_vecValues.push_back(TValue());
								}

						public: Arr(const TValue*arr, int len)
						{
							m_vecValues = T_Arr(arr,arr+len);
						}

						public: int Length() const
							{
								return (int)m_vecValues.size();
							}

						public: TValue &operator[](const int in_index)
								{
									if (in_index<0 || in_index>=(int)m_vecValues.size())
										throw Exception(_T("Out of bounds"));
									return m_vecValues.at(in_index); 
								}
						public: const TValue &operator[](const int in_index) const
								{
									if (in_index<0 || in_index>=(int)m_vecValues.size())
										throw Exception(_T("Out of bounds"));
									return m_vecValues.at(in_index); 
								}
	

						
						public: typedef typename T_Arr::iterator iterator;
						public: typedef typename T_Arr::const_iterator const_iterator;
						public: iterator begin() { return m_vecValues.begin(); }
						public: iterator end() { return m_vecValues.end(); }
						public: const_iterator cbegin() const { return m_vecValues.cbegin(); }
						public: const_iterator cend() const { return m_vecValues.cend(); }
				};


				template<class TKey, class TValue>
				class Dictionary
				{
                    private:
                            typedef std::map<TKey, TValue> T_mapType;
                            T_mapType m_map;
                            std::list<TKey*> m_lstKeys;
                            std::list<TValue*> m_lstValues;

					public: int Count()
							{
								
							}
					public: void Clear()
							{
							}
					public: void Add(const TKey &key, const TValue &value)
							{
								m_map.insert(std::make_pair(key,value));
							}

					public: std::list<TKey> Keys()
						    {
                                typename T_mapType::const_iterator b = m_map.begin();
                                typename T_mapType::const_iterator e = m_map.end();
                                std::list<TKey> lst;
                                for (typename T_mapType::const_iterator it = b; it != e; it++) lst.push_back(it->first);
							    return lst;
						    }

                    public: std::list<TValue> Values()
                            {
                                typename T_mapType::const_iterator b = m_map.begin();
                                typename T_mapType::const_iterator e = m_map.end();
                                std::list<TValue> lst;
                                for (typename T_mapType::const_iterator it = b; it != e; it++) lst.push_back(it->second);
                                return lst;
                            }

                            public: TValue &operator[](const TKey &key)
                            {
                                typename T_mapType::iterator it = m_map.find(key);
                                if (it == m_map.end()) throw Exception(_T("Item not in dictionary"));
                                return it->second;
                            }
                            public: const TValue &operator[](const TKey &key) const
                            {
                                typename T_mapType::const_iterator it = m_map.find(key);
                                if (it == m_map.end()) throw Exception(_T("Item not in dictionary"));
                                return it->second;
                            }

				    };
			}
	}
}