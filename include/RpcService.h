#ifndef DODO_RPC_RPCSERVICE_H_
#define DODO_RPC_RPCSERVICE_H_

#include <string>
#include <functional>

#include "RpcCommon.h"

namespace dodo
{
    namespace rpc
    {
        using namespace std;

        template<typename PROTOCOL_TYPE>
        class RpcService
        {
        public:
            RpcService()
            {
            }

            template<typename... Args>
            CALL_RESULT    call(const Args&... args)
            {
                return mCaller.call(args...);
            }

            template<typename F>
            SERVICE_WRAPPER def(F func)
            {
                return PROTOCOL_TYPE::makeWrapper(func);
            }

        private:
            typename PROTOCOL_TYPE::Caller  mCaller;
        };
    }
}

#endif