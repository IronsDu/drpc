#include <iostream>

#include "RpcService.h"
#include "JsonRpc.h"
#include "MsgpackRpc.h"

using namespace std;
using namespace dodo::rpc;

int main()
{
    RpcService<MsgpackProtocol> rpcServer;  // or RpcService<JsonProtocol> rpcServer
    RpcService<MsgpackProtocol> rpcClient;

    rpcServer.def("test", [](const std::string& value) {
        cout << "receive " << value << endl;
    });

    int reqID = 0;
    rpcServer.def("ping", [&reqID](RpcRequestInfo info, std::string& value) {
        cout << "receive " << value << endl;
        reqID = info.getRequestID();
    });

    static_assert(std::is_same<decltype(rpcServer), decltype(rpcClient)>::value, "");

    {
        auto requestBinary = rpcClient.call("test", "hello world?");
        rpcServer.handleRpc(requestBinary);
    }

    {
        auto requestBinary = rpcClient.call("ping", "hello world?", [](RpcRequestInfo info, std::string& value) {
            cout << "reply of " << value << endl;

        });
        rpcServer.handleRpc(requestBinary);


        auto replyBinary = rpcServer.reply(reqID, "hello world!");
        rpcClient.handleRpc(replyBinary);
    }

    {
        auto requestBinary = rpcClient.call("ping", "hello?", [](std::string& value) {
            cout << "reply of " << value << endl;

        });
        rpcServer.handleRpc(requestBinary);


        auto replyBinary = rpcServer.reply(reqID, "hello!");
        rpcClient.handleRpc(replyBinary);
    }

    return 0;
}