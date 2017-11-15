/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺2017/11/11
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺
// �޸�˵����
//
// �汾��V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_NETIO_IMPL_INCLUDE
#define M_NETIO_NETIO_IMPL_INCLUDE

M_NETIO_NAMESPACE_BEGIN

NetIo::NetIo() :_backlog(20) {
	_endian = SocketLib::detail::Util::LocalEndian();
}

NetIo::NetIo(SocketLib::s_uint32_t backlog) : _backlog(backlog) {
	_endian = SocketLib::detail::Util::LocalEndian();
}

NetIo::~NetIo() {}

bool NetIo::ListenOne(const SocketLib::Tcp::EndPoint& ep) {
	try {
		NetIoTcpAcceptorPtr acceptor(new SocketLib::TcpAcceptor<SocketLib::IoService>(_ioservice, ep, _backlog));
		TcpSocketPtr clisock(new TcpSocket(*this, 0));
		function_t<void(SocketLib::SocketError)> handler = bind_t(&NetIo::AcceptHandler, this, placeholder_1, clisock, acceptor);
		acceptor->AsyncAccept(handler, clisock->GetSocket());
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
	return true;
}

bool NetIo::ListenOne(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ListenOne(ep);
}

void NetIo::AsyncConnect(const SocketLib::Tcp::EndPoint& ep) {
	netiolib::TcpConnectorPtr connector(new netiolib::TcpConnector(*this, 0));
	connector->AsyncConnect("127.0.0.1", 3001);
	connector.reset();
}
void NetIo::AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return AsyncConnect(ep);
}

void NetIo::Run() {
	try {
		_ioservice.Run();
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		M_NETIO_LOGGER("run happend error:"M_ERROR_DESC_STR(error));
	}
}

void NetIo::Stop() {
	try{
		_ioservice.Stop();
	}
	catch (SocketLib::SocketError& error){
		lasterror = error;
		M_NETIO_LOGGER("stop happend error:"M_ERROR_DESC_STR(error));
	}
}

inline SocketLib::SocketError NetIo::GetLastError()const {
	return lasterror;
}

inline SocketLib::IoService& NetIo::GetIoService() {
	return _ioservice;
}

inline SocketLib::s_uint32_t NetIo::LocalEndian()const {
	return _endian;
}

void NetIo::AcceptHandler(SocketLib::SocketError error, TcpSocketPtr clisock, NetIoTcpAcceptorPtr acceptor) {
	if (error) {
		M_NETIO_LOGGER("accept handler happend error:" << M_NETIO_LOGGER(error));
	}
	else {
		clisock->Init();
	}
	if (!_ioservice.Stopped()) {
		TcpSocketPtr clisock(new TcpSocket(*this, 0));
		function_t<void(SocketLib::SocketError)> handler = bind_t(&NetIo::AcceptHandler, this, placeholder_1, clisock, acceptor);
		SocketLib::SocketError error2;
		acceptor->AsyncAccept(handler, clisock->GetSocket(),error2);
		if (error2)
			lasterror = error2;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
*����������������Ϊ�麯�����Ա����ʵ��ҵ���ģʽ��������ģʽ����Ϣ���ַ�����
*��֤ͬһ��socket���������������ĵ�����ѭOnConnected -> OnReceiveData -> OnDisconnected��˳��
*��֤ͬһ��socket�����º����������ĵ��ö���ͬһ���߳���
*/

// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
void NetIo::OnConnected(TcpSocketPtr clisock) {

}
void NetIo::OnConnected(TcpConnectorPtr clisock, SocketLib::SocketError error) {

}

// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
void NetIo::OnDisconnected(TcpSocketPtr clisock) {

}
void NetIo::OnDisconnected(TcpConnectorPtr clisock) {

}

// ���ݰ�֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
void NetIo::OnReceiveData(TcpSocketPtr clisock, BufferPtr buffer) {

}
void NetIo::OnReceiveData(TcpConnectorPtr clisock, BufferPtr buffer) {

}

M_NETIO_NAMESPACE_END
#endif