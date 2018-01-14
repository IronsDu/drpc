#ifndef DODO_RPC_COMMON_H_
#define DODO_RPC_COMMON_H_

#include <string>
#include <functional>
#include <tuple>

namespace dodo
{
    namespace rpc
    {
        // 所有的服务函数的包装函数
        typedef std::function<void(const std::string&)> SERVICE_WRAPPER;
        // call函数返回CALL_RESULT,其中std::string表示参数打包的JSON串,第二个SERVICE_WRAPPER则为回调(可为nullptr)
        typedef std::tuple<std::string, SERVICE_WRAPPER> CALL_RESULT;

        template <typename T>
        class HasCallOperator
        {
            typedef char _One;
            typedef struct{ char a[2]; }_Two;
            template<typename TT>
            static _One hasFunc(decltype(&TT::operator()));
            template<typename TT>
            static _Two hasFunc(...);

        public:
            static const bool value = sizeof(hasFunc<T>(nullptr)) == sizeof(_One);
        };
    }
}

#endif