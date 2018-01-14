#include <iostream>

#include "RpcService.h"
#include "JsonRpc.h"
#include "MsgpackRpc.h"

using namespace std;
using namespace dodo::rpc;

int main()
{
    RpcService<JsonProtocol> rpc;

    // ����ʵ����Σ�call ���Ƿ��� std::tuple<string, std::function<void(string)>>
    // ǰ�߱�ʾ����pack��json�ַ��������������Ϊnullptr���ʾ�ص�����������string������ִ�У�

    auto fuck1 = rpc.call(1, 2, "hehe", [](const std::string& value, int a) {
        cout << "receive " << value << endl;
    });
    auto fuck2 = rpc.call("haha", 10);

    // def ������ֱ�ӷ���һ����װ���� std::function<void(string)>���� call�������ص�tuple�ڶ���ֵ��ȫͬ��.
    auto service = rpc.def([](const std::string& value, int a) {
        cout << "receive " << value << endl;
    });

    // �� service ����һ��string�����ɵ��ûص�
    service(std::get<0>(fuck2));
    std::get<1>(fuck1)(std::get<0>(fuck2));

    return 0;
}