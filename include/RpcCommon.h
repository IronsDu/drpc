#ifndef DODO_RPC_COMMON_H_
#define DODO_RPC_COMMON_H_

#include <string>
#include <functional>
#include <tuple>

namespace dodo
{
    namespace rpc
    {
        // ���еķ������İ�װ����
        typedef std::function<void(const std::string&)> SERVICE_WRAPPER;
        // call��������CALL_RESULT,����std::string��ʾ���������JSON��,�ڶ���SERVICE_WRAPPER��Ϊ�ص�(��Ϊnullptr)
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