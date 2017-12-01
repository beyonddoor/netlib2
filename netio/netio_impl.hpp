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

NetIo::NetIo() 
	:_backlog(20){
	_endian = SocketLib::detail::Util::LocalEndian();
}

NetIo::NetIo(SocketLib::s_uint32_t backlog) 
	: _backlog(backlog) {
	_endian = SocketLib::detail::Util::LocalEndian();
}

NetIo::~NetIo() {}

bool NetIo::ListenOne(const SocketLib::Tcp::EndPoint& ep) {
	try {
		NetIoTcpAcceptorPtr acceptor(new SocketLib::TcpAcceptor<SocketLib::IoService>(_ioservice, ep, _backlog));
		TcpSocketPtr clisock(new TcpSocket(*this));
		acceptor->AsyncAccept(bind_t(&NetIo::_AcceptHandler, this, placeholder_1, clisock, acceptor), clisock->GetSocket());
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

// ����һ��http����
bool NetIo::ListenOneHttp(const SocketLib::Tcp::EndPoint& ep) {
	try {
		NetIoTcpAcceptorPtr acceptor(new SocketLib::TcpAcceptor<SocketLib::IoService>(_ioservice, ep, _backlog));
		HttpSocketPtr clisock(new HttpSocket(*this));
		acceptor->AsyncAccept(bind_t(&NetIo::_AcceptHttpHandler, this, placeholder_1, clisock, acceptor), clisock->GetSocket());
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
	return true;
}

bool NetIo::ListenOneHttp(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ListenOneHttp(ep);
}

void NetIo::ConnectOne(const SocketLib::Tcp::EndPoint& ep, unsigned int data) {
	try {
		netiolib::TcpConnectorPtr connector(new netiolib::TcpConnector(*this));
		connector->SetData(data);
		connector->AsyncConnect("127.0.0.1", 3001);
		connector.reset();
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
	}
}
void NetIo::ConnectOne(const std::string& addr, SocketLib::s_uint16_t port, unsigned int data) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ConnectOne(ep,data);
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

void NetIo::_AcceptHandler(SocketLib::SocketError error, TcpSocketPtr& clisock, NetIoTcpAcceptorPtr& acceptor) {
	if (error) {
		M_NETIO_LOGGER("accept handler happend error:" << M_NETIO_LOGGER(error));
	}
	else {
		clisock->Init();
	}
	TcpSocketPtr newclisock(new TcpSocket(*this));
	acceptor->AsyncAccept(bind_t(&NetIo::_AcceptHandler, this, placeholder_1, newclisock, acceptor), newclisock->GetSocket(), error);
	if (error)
		lasterror = error;
}

void NetIo::_AcceptHttpHandler(SocketLib::SocketError error, HttpSocketPtr& clisock, NetIoTcpAcceptorPtr& acceptor) {
	if (error) {
		M_NETIO_LOGGER("accept handler happend error:" << M_NETIO_LOGGER(error));
	}
	else {
		clisock->Init();
	}
	HttpSocketPtr newclisock(new HttpSocket(*this));
	acceptor->AsyncAccept(bind_t(&NetIo::_AcceptHttpHandler, this, placeholder_1, newclisock, acceptor), newclisock->GetSocket(), error);
	if (error)
		lasterror = error;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
*����������������Ϊ�麯�����Ա����ʵ��ҵ���ģʽ��������ģʽ����Ϣ���ַ�����
*��֤ͬһ��socket���������������ĵ�����ѭOnConnected -> OnReceiveData -> OnDisconnected��˳��
*��֤ͬһ��socket�����º����������ĵ��ö���ͬһ���߳���
*/

// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
void NetIo::OnConnected(const TcpSocketPtr& clisock) {
}
void NetIo::OnConnected(const TcpConnectorPtr& clisock, SocketLib::SocketError error) {
}
void NetIo::OnConnected(HttpSocketPtr clisock) {
}

// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
void NetIo::OnDisconnected(const TcpSocketPtr& clisock) {
}
void NetIo::OnDisconnected(const TcpConnectorPtr& clisock) {
}
void NetIo::OnDisconnected(HttpSocketPtr clisock) {
}

// ���ݰ�֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
void NetIo::OnReceiveData(const TcpSocketPtr& clisock, SocketLib::Buffer& buffer) {
}
void NetIo::OnReceiveData(const TcpConnectorPtr& clisock, SocketLib::Buffer& buffer) {
}
void NetIo::OnReceiveData(HttpSocketPtr clisock, HttpSvrRecvMsg& httpmsg) {
}

M_NETIO_NAMESPACE_END
#endif