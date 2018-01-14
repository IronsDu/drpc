#include <iostream>

#include "RpcService.h"
#include "JsonRpc.h"
#include "MsgpackRpc.h"

using namespace std;
using namespace dodo::rpc;

int main()
{
    RpcService<JsonProtocol> rpc;

    // 无论实参如何，call 总是返回 std::tuple<string, std::function<void(string)>>
    // 前者表示参数pack的json字符串，后者如果不为nullptr则表示回调函数（接收string，即可执行）

    auto fuck1 = rpc.call(1, 2, "hehe", [](const std::string& value, int a) {
        cout << "receive " << value << endl;
    });
    auto fuck2 = rpc.call("haha", 10);

    // def 函数则直接返回一个包装函数 std::function<void(string)>，跟 call函数返回的tuple第二个值完全同类.
    auto service = rpc.def([](const std::string& value, int a) {
        cout << "receive " << value << endl;
    });

    // 向 service 传递一个string，即可调用回调
    service(std::get<0>(fuck2));
    std::get<1>(fuck1)(std::get<0>(fuck2));

    return 0;
}