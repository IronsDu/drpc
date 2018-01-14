#ifndef DODO_RPC_MSGPACKRPC_UTILS_H_
#define DODO_RPC_MSGPACKRPC_UTILS_H_

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

        // 读写 Msgpack
        class MsgpackUtils
        {
        public:
            template<class Tuple, std::size_t N>
            struct TupleRead
            {
                static void read(const char* buffer, size_t size, size_t& off, Tuple& t)
                {
                    TupleRead<Tuple, N - 1>::read(buffer, size, off, t);
                    ValueRead<std::is_base_of<::google::protobuf::Message, 
                        std::remove_reference<decltype(std::get<N - 1>(t))>::type>::value>::read(
                            buffer, 
                            size, 
                            off, 
                            std::get<N - 1>(t));
                }
            };

            template<class Tuple>
            struct TupleRead < Tuple, 1 >
            {
                static void read(const char* buffer, 
                    size_t size, 
                    size_t& off, 
                    Tuple& t)
                {
                    ValueRead<std::is_base_of<::google::protobuf::Message, 
                        std::remove_reference<decltype(std::get<0>(t))>::type>::value>::read(
                            buffer, 
                            size, 
                            off, 
                            std::get<0>(t));
                }
            };

            template<bool isPB>
            struct ValueRead
            {
                template<class... Args>
                static  void    read(const char* buffer, 
                    size_t size, 
                    size_t& off, 
                    std::tuple<Args...>& value)
                {
                    TupleRead<decltype(value), sizeof...(Args)>::read(buffer, size, off, value);
                }

                template<typename T>
                static  void    read(const char* buffer, size_t size, size_t& off, T& value)
                {
                    auto h = msgpack::unpack(buffer, size, off);
                    const msgpack::object& o = h.get();
                    value = o.as<T>();
                }

                template<typename T>
                static void     read(const char* buffer, size_t size, size_t& off, std::vector<T>& value)
                {
                    int32_t len;
                    read(buffer, size, off, len);
                    while (off != size && len > 0)
                    {
                        T t;
                        read(buffer, size, off, t);
                        value.push_back(std::move(t));
                        len--;
                    }
                }

                template<typename K, typename T>
                static void     read(const char* buffer, size_t size, size_t& off, std::map<K, T>& value)
                {
                    int32_t len;
                    read(buffer, size, off, len);
                    while (off != size && len > 0)
                    {
                        K key;
                        read(buffer, size, off, key);
                        T t;
                        read(buffer, size, off, t);
                        value.insert(std::make_pair(std::move(key), std::move(t)));
                        len--;
                    }
                }

            };

            template<>
            struct ValueRead<true>
            {
                template<typename T>
                static  void    read(const char* buffer, size_t size, size_t& off, T& value)
                {
                    /*TODO::直接读取二进制流*/
                    string str;
                    msgpack::unpacked result;
                    msgpack::unpack(result, buffer, size, off);
                    const msgpack::object& o = result.get();
                    o.convert(&str);
                    value.ParseFromArray(str.c_str(), str.size());
                }

            };

            template<class Tuple, std::size_t N>
            struct TupleWrite
            {
                static void write(msgpack::sbuffer& sbuf, const Tuple& value)
                {
                    TupleWrite<Tuple, N - 1>::write(sbuf, value);
                    ValueWrite<std::is_base_of<::google::protobuf::Message, 
                        decltype(std::get<N - 1>(value))>::value>::write(
                            sbuf, 
                            std::get<N - 1>(value));
                }
            };

            template<class Tuple>
            struct TupleWrite < Tuple, 1 >
            {
                static void write(msgpack::sbuffer& sbuf, const Tuple& value)
                {
                    ValueWrite<std::is_base_of<::google::protobuf::Message, 
                        decltype(std::get<0>(value))>::value>::write(
                            sbuf, 
                            std::get<0>(value));
                }
            };

            template<bool isPB>
            struct ValueWrite
            {
                template<typename T>
                static  void    write(msgpack::sbuffer& sbuf, const T& value)
                {
                    msgpack::pack(&sbuf, value);
                }

                template<class... Args>
                static  void    write(msgpack::sbuffer& sbuf, const std::tuple<Args...>& value)
                {
                    TupleWrite<decltype(value), sizeof...(Args)>::write(sbuf, value);
                }

                template<typename T>
                static  void    write(msgpack::sbuffer& sbuf, const vector<T>& value)
                {
                    write(sbuf, (int32_t)value.size());
                    for (auto& v : value)
                    {
                        write(sbuf, v);
                    }
                }

                template<typename K, typename T>
                static  void    write(msgpack::sbuffer& sbuf, const map<K, T>& value)
                {
                    write(sbuf, (int32_t)value.size());
                    for (auto& v : value)
                    {
                        write(sbuf, v.first);
                        write(sbuf, v.second);
                    }
                }
            };

            template<>
            struct ValueWrite<true>
            {
                template<typename T>
                static  void    write(msgpack::sbuffer& sbuf, const T& value)
                {
                    char stackBuf[1024];
                    int pbByteSize = value.ByteSize();
                    if (pbByteSize <= sizeof(stackBuf))
                    {
                        value.SerializeToArray(stackBuf, pbByteSize);

                        msgpack::packer<msgpack::sbuffer>(sbuf).pack_str(pbByteSize);
                        msgpack::packer<msgpack::sbuffer>(sbuf).pack_str_body(stackBuf, pbByteSize);
                    }
                    else
                    {
                        string str;
                        str.resize(pbByteSize);
                        value.SerializeToArray((void*)str.c_str(), pbByteSize);

                        msgpack::packer<msgpack::sbuffer>(sbuf).pack_str(str.size());
                        msgpack::packer<msgpack::sbuffer>(sbuf).pack_str_body(str.c_str(), str.size());
                    }
                }
            };
        };
    }
}

#endif