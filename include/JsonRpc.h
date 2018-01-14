#ifndef DODO_RPC_JSONRPC_H_
#define DODO_RPC_JSONRPC_H_

#include <cassert>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <tuple>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "RpcCommon.h"
#include "RpcJsonUtils.h"

namespace dodo
{
    namespace rpc
    {
        using namespace std;
        using namespace rapidjson;

        struct JsonProtocol
        {
            struct Decode
            {
                template<typename ...Args>
                struct Invoke
                {
                public:
                    static void invoke(const std::function<void(Args...)>& f, 
                        const rapidjson::Value& msg)
                    {
                        int parmIndex = 0;
                        Eval<sizeof...(Args), Args...>::eval<Args...>(f, msg, parmIndex);
                    }
                };
            };

            template<int SIZE, typename ...Args>
            struct Eval
            {
                template<typename T>
                struct HelpEval
                {
                    template<typename ...LeftArgs, typename ...NowArgs>
                    static  void    eval(const std::function<void(Args...)>& f,
                        const rapidjson::Value& msg, 
                        int& parmIndex, 
                        NowArgs&&... args)
                    {
                        typedef std::tuple_element<sizeof...(Args)-sizeof...(LeftArgs)-1, 
                            std::tuple<Args...>>::type TMP1;

                        std::remove_const<std::remove_reference<TMP1>::type>::type value;
                        auto it = msg.FindMember(std::to_string(parmIndex++).c_str());
                        const rapidjson::Value& element = (*it).value;
                        JsonUtils::readJson(element, value);

                        Eval<sizeof...(LeftArgs), Args...>::eval<LeftArgs...>(
                            f, 
                            msg, 
                            parmIndex,
                            std::forward<NowArgs>(args)..., 
                            value);
                    }
                };

                template<typename T, typename ...LeftArgs, typename ...NowArgs>
                static  void    eval(const std::function<void(Args...)>& f,
                    const rapidjson::Value& msg, 
                    int& parmIndex, 
                    NowArgs&&... args)
                {
                    HelpEval<T>::eval<LeftArgs...>(
                        f, 
                        msg, 
                        parmIndex, 
                        std::forward<NowArgs>(args)...);
                }
            };

            template<typename ...Args>
            struct Eval < 0, Args... >
            {
                template<typename ...NowArgs>
                static  void    eval(const std::function<void(Args...)>& f,
                    const rapidjson::Value& msg, 
                    int& parmIndex, 
                    NowArgs&&... args)
                {
                    f(std::forward<NowArgs>(args)...);
                }
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
                rapidjson::Document doc;
                doc.Parse(msg.c_str());
                Decode::Invoke<Args...>::invoke(lambdaObj, doc);
            }

            template<typename LAMBDA>
            static SERVICE_WRAPPER makeWrapper(const LAMBDA& lambdaObj)
            {
                return [lambdaObj](const std::string& str) {
                    lowExecute(str, lambdaObj);
                };
            }

            class Caller
            {
            public:
                Caller() : mWriter(mBuffer){}

                template<typename... Args>
                CALL_RESULT call(const Args&... args)
                {
                    int index = 0;
                    rapidjson::Value parms(rapidjson::kObjectType);
                    auto f = writeCallArg(mDoc, parms, index, args...);

                    mBuffer.Clear();
                    mWriter.Reset(mBuffer);
                    parms.Accept(mWriter);

                    return std::make_tuple(mBuffer.GetString(), f);
                }
            private:
                SERVICE_WRAPPER writeCallArg(
                    rapidjson::Document& doc, 
                    rapidjson::Value&, 
                    int& index)
                {
                    return nullptr;
                }

                template<typename Arg>
                SERVICE_WRAPPER writeCallArg(
                    rapidjson::Document& doc, 
                    rapidjson::Value& msg, 
                    int& index, 
                    const Arg& arg)
                {
                    /*只(剩)有一个参数,肯定也为最后一个参数，允许为lambda*/
                    return SelectWriteArgJson<std::is_function<Arg>::value || 
                        HasCallOperator<Arg>::value>::Write(
                            doc, 
                            msg, 
                            arg, 
                            index++);
                }

                template<typename Arg1, typename... Args>
                SERVICE_WRAPPER writeCallArg(
                    rapidjson::Document& doc, 
                    rapidjson::Value& msg, 
                    int& index, 
                    const Arg1& arg1, 
                    const Args&... args)
                {
                    JsonUtils::writeJsonByIndex(doc, msg, arg1, index++);
                    return writeCallArg(doc, msg, index, args...);
                }

                template<bool>
                struct SelectWriteArgJson;

                template<>
                struct SelectWriteArgJson < true >
                {
                    template<typename LAMBDA>
                    static  SERVICE_WRAPPER Write(
                        rapidjson::Document& doc, 
                        rapidjson::Value& parms, 
                        const LAMBDA& lambdaObj,
                        int index)
                    {
                        return makeWrapper(lambdaObj);
                    }
                };

                template<>
                struct SelectWriteArgJson < false >
                {
                    template<typename ARGTYPE>
                    static  SERVICE_WRAPPER Write(
                        rapidjson::Document& doc, 
                        rapidjson::Value& parms, 
                        const ARGTYPE& arg, 
                        int index)
                    {
                        JsonUtils::writeJsonByIndex(doc, parms, arg, index);
                        return nullptr;
                    }
                };

            private:
                rapidjson::Document                         mDoc;
                rapidjson::StringBuffer                     mBuffer;
                rapidjson::Writer<rapidjson::StringBuffer>  mWriter;
            };
        };
    }
}

#endif