#ifndef DODO_RPC_JSONRPC_UTILS_H_
#define DODO_RPC_JSONRPC_UTILS_H_

#include <cassert>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <tuple>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace dodo
{
    namespace rpc
    {
        using namespace std;
        using namespace rapidjson;

        // 读写 RapidJson
        class JsonUtils
        {
        public:
            /*  反序列化    */
            static  void    readJson(const rapidjson::Value& msg, int8_t& ret)
            {
                ret = static_cast<int8_t>(msg.GetInt64());
            }

            static  void    readJson(const rapidjson::Value& msg, int16_t& ret)
            {
                ret = msg.GetInt();
            }

            static  void    readJson(const rapidjson::Value& msg, int64_t& ret)
            {
                ret = msg.GetInt64();
            }

            static  void    readJson(const rapidjson::Value& msg, char& ret)
            {
                ret = msg.GetInt();
            }

            static  void    readJson(const rapidjson::Value& msg, int& ret)
            {
                ret = msg.GetInt();
            }

            static  void    readJson(const rapidjson::Value& msg, string& ret)
            {
                ret = msg.GetString();
            }

            static  void    readJson(const rapidjson::Value& msg, vector<int>& ret)
            {
                for (rapidjson::SizeType i = 0; i < msg.Size(); ++i)
                {
                    ret.push_back(msg[i].GetInt());
                }
            }

            static  void    readJson(const rapidjson::Value& msg, vector<string>& ret)
            {
                for (rapidjson::SizeType i = 0; i < msg.Size(); ++i)
                {
                    ret.push_back(msg[i].GetString());
                }
            }

            template<typename T>
            static  void    readJson(const rapidjson::Value& msg, vector<T>& ret)
            {
                for (rapidjson::SizeType i = 0; i < msg.Size(); ++i)
                {
                    T tmp;
                    readJson(msg[i], tmp);
                    ret.push_back(std::move(tmp));
                }
            }

            static  void    readJson(const rapidjson::Value& msg, map<string, string>& ret)
            {
                for (rapidjson::Value::ConstMemberIterator itr = msg.MemberBegin(); itr != msg.MemberEnd(); ++itr)
                {
                    ret[(*itr).name.GetString()] = (*itr).value.GetString();
                }
            }

            static  void    readJson(const rapidjson::Value& msg, map<int, int>& ret)
            {
                for (rapidjson::Value::ConstMemberIterator itr = msg.MemberBegin(); itr != msg.MemberEnd(); ++itr)
                {
                    ret[atoi((*itr).name.GetString())] = (*itr).value.GetInt();
                }
            }

            static  void    readJson(const rapidjson::Value& msg, map<string, int>& ret)
            {
                for (rapidjson::Value::ConstMemberIterator itr = msg.MemberBegin(); itr != msg.MemberEnd(); ++itr)
                {
                    ret[(*itr).name.GetString()] = (*itr).value.GetInt();
                }
            }

            template<typename T>
            static  void    readJson(const rapidjson::Value& msg, map<string, T>& ret)
            {
                for (Value::ConstMemberIterator itr = msg.MemberBegin(); itr != msg.MemberEnd(); ++itr)
                {
                    T tmp;
                    readJson((*itr).value, tmp);
                    ret[(*itr).name.GetString()] = std::move(tmp);
                }
            }

            template<typename T>
            static  void    readJson(const rapidjson::Value& msg, map<int, T>& ret)
            {
                for (Value::ConstMemberIterator itr = msg.MemberBegin(); itr != msg.MemberEnd(); ++itr)
                {
                    T tmp;
                    readJson((*itr).value, tmp);
                    ret[atoi((*itr).name.GetString())] = std::move(tmp);
                }
            }

            template<class Tuple, std::size_t N>
            struct TupleReader {
                static void read(const rapidjson::Value& msg, Tuple& t)
                {
                    TupleReader<Tuple, N - 1>::read(msg, t);
                    if (msg.IsObject())
                    {
                        rapidjson::Value::ConstMemberIterator itr = msg.FindMember(std::to_string(N - 1).c_str());
                        readJson((*itr).value, std::get<N - 1>(t));
                    }
                }
            };

            template<class Tuple>
            struct TupleReader < Tuple, 1 > {
                static void read(const rapidjson::Value& msg, Tuple& t)
                {
                    if (msg.IsObject())
                    {
                        rapidjson::Value::ConstMemberIterator itr = msg.FindMember("0");
                        readJson((*itr).value, std::get<0>(t));
                    }
                }
            };

            template<class... Args>
            static  void    readJson(const rapidjson::Value& msg, std::tuple<Args...>& value)
            {
                TupleReader<decltype(value), sizeof...(Args)>::read(msg, value);
            }

            /*  序列化-把数据转换为Json对象  */
            static  rapidjson::Value    writeJson(rapidjson::Document& doc, const int8_t& value)
            {
                return rapidjson::Value(value);
            }

            static  rapidjson::Value    writeJson(rapidjson::Document& doc, const int16_t& value)
            {
                return rapidjson::Value(value);
            }

            static  rapidjson::Value    writeJson(rapidjson::Document& doc, const int& value)
            {
                return rapidjson::Value(value);
            }

            static  rapidjson::Value    writeJson(rapidjson::Document& doc, const int64_t& value)
            {
                return rapidjson::Value(value);
            }

            static  rapidjson::Value   writeJson(rapidjson::Document& doc, const char* const& value)
            {
                return rapidjson::Value(value, doc.GetAllocator());
            }

            static  rapidjson::Value   writeJson(rapidjson::Document& doc, const string& value)
            {
                return rapidjson::Value(value.c_str(), doc.GetAllocator());
            }

            static  rapidjson::Value   writeJson(rapidjson::Document& doc, const vector<int>& value)
            {
                rapidjson::Value arrayObject(rapidjson::kArrayType);
                for (size_t i = 0; i < value.size(); ++i)
                {
                    arrayObject.PushBack(rapidjson::Value(value[i]), doc.GetAllocator());
                }
                return arrayObject;
            }

            static  rapidjson::Value   writeJson(rapidjson::Document& doc, const vector<string>& value)
            {
                rapidjson::Value arrayObject(rapidjson::kArrayType);
                for (size_t i = 0; i < value.size(); ++i)
                {
                    arrayObject.PushBack(rapidjson::Value(value[i].c_str(), doc.GetAllocator()), doc.GetAllocator());
                }
                return arrayObject;
            }

            template<typename T>
            static  rapidjson::Value   writeJson(rapidjson::Document& doc, const vector<T>& value)
            {
                rapidjson::Value arrayObject(rapidjson::kArrayType);
                for (size_t i = 0; i < value.size(); ++i)
                {
                    arrayObject.PushBack(writeJson(doc, value[i]), doc.GetAllocator());
                }
                return arrayObject;
            }

            static  rapidjson::Value   writeJson(rapidjson::Document& doc, const map<string, string>& value)
            {
                rapidjson::Value mapObject(rapidjson::kObjectType);
                map<string, string>::const_iterator itend = value.end();
                for (map<string, string>::const_iterator it = value.begin(); it != itend; ++it)
                {
                    mapObject.AddMember(rapidjson::GenericValue<rapidjson::UTF8<>>(it->first.c_str(), doc.GetAllocator()),
                        rapidjson::Value(it->second.c_str(), doc.GetAllocator()), doc.GetAllocator());
                }
                return mapObject;
            }

            static  rapidjson::Value   writeJson(rapidjson::Document& doc, const map<int, string>& value)
            {
                rapidjson::Value mapObject(rapidjson::kObjectType);
                map<int, string>::const_iterator itend = value.end();
                for (map<int, string>::const_iterator it = value.begin(); it != itend; ++it)
                {
                    mapObject.AddMember(rapidjson::GenericValue<rapidjson::UTF8<>>(std::to_string((*it).first).c_str(), doc.GetAllocator()),
                        rapidjson::Value(it->second.c_str(), doc.GetAllocator()), doc.GetAllocator());
                }
                return mapObject;
            }

            static  rapidjson::Value   writeJson(rapidjson::Document& doc, const map<string, int>& value)
            {
                rapidjson::Value mapObject(rapidjson::kObjectType);
                std::map<string, int>::const_iterator itend = value.end();
                for (std::map<string, int>::const_iterator it = value.begin(); it != itend; ++it)
                {
                    mapObject.AddMember(rapidjson::GenericValue<rapidjson::UTF8<>>(it->first.c_str(), doc.GetAllocator()),
                        rapidjson::Value(it->second), doc.GetAllocator());
                }
                return mapObject;
            }

            template< typename V>
            static  rapidjson::Value   writeJson(rapidjson::Document& doc, const map<int, V>& value)
            {
                rapidjson::Value mapObject(rapidjson::kObjectType);
                /*遍历此map*/
                for (map<int, V>::const_iterator it = value.begin(); it != value.end(); ++it)
                {
                    /*把value序列化到map的jsonobject中,key就是它在map结构中的key*/
                    mapObject.AddMember(rapidjson::GenericValue<rapidjson::UTF8<>>(std::to_string(it->first).c_str(), doc.GetAllocator()),
                        writeJson(doc, it->second), doc.GetAllocator());
                }

                /*把此map添加到msg中*/
                return mapObject;
            }

            template<typename V>
            static  rapidjson::Value   writeJson(rapidjson::Document& doc, const map<string, V>& value)
            {
                rapidjson::Value mapObject(rapidjson::kObjectType);
                /*遍历此map*/
                for (map<string, V>::const_iterator it = value.begin(); it != value.end(); ++it)
                {
                    /*把value序列化到map的jsonobject中,key就是它在map结构中的key*/
                    mapObject.AddMember(GenericValue<UTF8<>>(it->first.c_str(), doc.GetAllocator()),
                        writeJson(doc, it->second), doc.GetAllocator());
                }

                /*把此map添加到msg中*/
                return mapObject;
            }

            template<class Tuple, std::size_t N>
            struct TupleWriter {
                static void write(rapidjson::Document& doc, rapidjson::Value& mapObject, const Tuple& t)
                {
                    TupleWriter<Tuple, N - 1>::write(doc, mapObject, t);
                    mapObject.AddMember(rapidjson::GenericValue<rapidjson::UTF8<>>(std::to_string(N - 1).c_str(), doc.GetAllocator()),
                        writeJson(doc, std::get<N - 1>(t)), doc.GetAllocator());
                }
            };

            template<class Tuple>
            struct TupleWriter < Tuple, 1 > {
                static void write(rapidjson::Document& doc,
                    rapidjson::Value& mapObject,
                    const Tuple& t)
                {
                    mapObject.AddMember(rapidjson::GenericValue<rapidjson::UTF8<>>(std::to_string(0).c_str(), doc.GetAllocator()),
                        writeJson(doc, std::get<0>(t)), doc.GetAllocator());
                }
            };


            template<class... Args>
            static  rapidjson::Value   writeJson(rapidjson::Document& doc, const std::tuple<Args...>& value)
            {
                rapidjson::Value mapObject(rapidjson::kObjectType);
                TupleWriter<decltype(value), sizeof...(Args)>::write(doc, mapObject, value);
                return mapObject;
            }

            template<typename T>
            static  void    writeJsonByIndex(rapidjson::Document& doc, rapidjson::Value& msg, const T& t, int index)
            {
                msg.AddMember(rapidjson::GenericValue<rapidjson::UTF8<>>(std::to_string(index).c_str(),
                    doc.GetAllocator()),
                    writeJson(doc, t), doc.GetAllocator());
            }
        };
    }
}

#endif