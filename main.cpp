#include "netio/netio.hpp"
#include <iostream>
#include "thread.hpp"

using namespace std;

class TestNetIo : public netiolib::NetIo {
public:
	// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnConnected(netiolib::TcpSocketPtr clisock) {
		cout << "OnConnected one : " << clisock->LocalEndpoint().Address() << endl;
	}

	// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnDisconnected(netiolib::TcpSocketPtr clisock) {
		cout << "OnDisconnected one : " << clisock->LocalEndpoint().Address() << endl;
	}

	// ���ݰ�֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnReceiveData(netiolib::TcpSocketPtr clisock, netiolib::BufferPtr buffer) {
		cout << "OnReceiveData one : " << clisock->LocalEndpoint().Address() << endl;
	}

	void Start(void*) {
		Run();
	}
	
};

int main() {

	TestNetIo test_io;
	thread thr(&TestNetIo::Start,&test_io,0);
	thr.sleep(200);
	if (!test_io.ListenOne("127.0.0.1", 3001)) {
		cout << test_io.GetLastError().What() << endl;
	}

	thr.join();
	return 0;
}