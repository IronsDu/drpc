#ifndef DODO_RPC_MSGPACKRPC_H_
#define DODO_RPC_MSGPACKRPC_H_

#include <cassert>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <functional>
#include <tuple>

#include "msgpack.hpp"
#include "RpcCommon.h"
#include "RpcMsgpackUtils.h"

namespace google
{
    namespace protobuf
    {
        class Message;
    }
}

namespace dodo
{
    namespace rpc
    {
        using namespace std;
        using namespace msgpack;

        struct MsgpackProtocol
        {
            template<int SIZE, typename ...Args>
            struct Eval;

            template<typename T>
            struct HelpEval
            {
                template<typename ...Args>
                struct Fuck
                {
                    template<typename T, typename ...LeftArgs, typename ...NowArgs>
                    static  void    eval(const std::function<void(Args...)>& f,
                        const char* buffer, 
                        size_t size, 
                        size_t& off, 
                        NowArgs&&... args)
                    {
                        typedef std::tuple_element<sizeof...(Args)-sizeof...(LeftArgs)-1,
                            std::tuple<Args...>>::type TMP1;

                        std::remove_const<std::remove_reference<TMP1>::type>::type value;

                        MsgpackUtils::ValueRead<std::is_base_of<::google::protobuf::Message,
                            std::remove_reference<decltype(value)>::type>::value>::read(
                                buffer, 
                                size, 
                                off,
                                value);

                        Eval<sizeof...(LeftArgs), Args...>::template eval<LeftArgs...>(
                            f, 
                            buffer, 
                            size, 
                            off, 
                            std::forward<NowArgs>(args)...,
                            value);
                    }
                };
            };

            template<int SIZE, typename ...Args>
            struct Eval
            {
                template<typename ...LeftArgs, typename ...NowArgs>
                static  void    eval(const std::function<void(Args...)>& f,
                    const char* buffer, 
                    size_t size, 
                    size_t& off, 
                    NowArgs&&... args)
                {
                    typedef std::tuple<Args...> TUPLE_TYPE;
                    typedef std::tuple_element<sizeof...(Args)-sizeof...(LeftArgs), TUPLE_TYPE>::type TMP1;
                    typedef HelpEval<TMP1> TMP2;
                    typedef typename TMP2::template Fuck<Args...> TMP;
                    TMP::template eval<LeftArgs...>(f, buffer, size, off, std::forward<NowArgs>(args)...);
                }
            };

            template<typename ...Args>
            struct Eval < 0, Args... >
            {
                template<typename ...NowArgs>
                static  void    eval(const std::function<void(Args...)>& f, 
                    const char* buffer, 
                    size_t size, 
                    size_t& off, 
                    NowArgs&&... args)
                {
                    f(std::forward<NowArgs>(args)...);
                }
            };

            struct Decode
            {
                template<typename ...Args>
                struct Invoke
                {
                public:
                    static void invoke(const std::function<void(Args...)>& f, const char* buffer, size_t len, size_t& off)
                    {
                        Eval<sizeof...(Args), Args...>::template eval<Args...>(f, buffer, len, off);
                    }
                };
            };

            template<typename T>
            static void lowExecute(const std::string& msg, const T& lambdaObj)
            {
                _lowExecute(msg, lambdaObj, &T::operator());
            }

            template<typename LAMBDA_OBJ_TYPE, typename ...Args>
            static void _lowExecute(const std::string& msg,
                const LAMBDA_OBJ_TYPE& lambdaObj,
                void(LAMBDA_OBJ_TYPE::*func)(Args...) const)
            {
                size_t size = msg.size();
                std::size_t off = 0;
                Decode::Invoke<Args...>::invoke(lambdaObj, msg.c_str(), size, off);
            }

            template<typename LAMBDA>
            static SERVICE_WRAPPER makeWrapper(const LAMBDA& lambdaObj)
            {
                return [lambdaObj](const std::string& str) {
                    lowExecute(str, lambdaObj);
                };
            }

            template<bool>
            struct SelectWriteArgMsgpack
            {
                template<typename LAMBDA>
                static  SERVICE_WRAPPER    Write(
                    msgpack::sbuffer& sbuf,
                    const LAMBDA& lambdaObj)
                {
                    return makeWrapper(lambdaObj);
                }
            };

            template<>
            struct SelectWriteArgMsgpack < false >
            {
                template<typename ARGTYPE>
                static  SERVICE_WRAPPER Write(
                    msgpack::sbuffer& sbuf, 
                    const ARGTYPE& arg)
                {
                    MsgpackUtils::ValueWrite<std::is_base_of<::google::protobuf::Message, ARGTYPE>::value>::write(sbuf, arg);
                    return nullptr;
                }
            };

            class Caller
            {
            public:
                template<typename... Args>
                CALL_RESULT call(const Args&... args)
                {
                    msgpack::sbuffer sbuf;

                    auto f = writeCallArg(sbuf, args...);

                    return std::make_tuple(string(sbuf.data(), sbuf.size()), f);
                }
            private:
                template<typename Arg>
                SERVICE_WRAPPER writeCallArg(
                    msgpack::sbuffer& sbuf,
                    const Arg& arg)
                {
                    /*只(剩)有一个参数,肯定也为最后一个参数，允许为lambda*/
                    return SelectWriteArgMsgpack<std::is_function<Arg>::value || 
                        HasCallOperator<Arg>::value>::Write(
                            sbuf, 
                            arg);
                }

                template<typename Arg1, typename... Args>
                SERVICE_WRAPPER writeCallArg(
                    msgpack::sbuffer& sbuf, 
                    const Arg1& arg1, 
                    const Args&... args)
                {
                    MsgpackUtils::ValueWrite<std::is_base_of<::google::protobuf::Message, Arg1>::value>::write(sbuf, arg1);
                    return writeCallArg(sbuf,  args...);
                }

                SERVICE_WRAPPER writeCallArg(msgpack::sbuffer& sbuf)
                {
                    return nullptr;
                }
            };
        };
    }
}

#endif