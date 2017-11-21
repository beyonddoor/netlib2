/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺2017/11/10
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺
// �޸�˵����
//
// �汾��V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_NETIO_INCLUDE
#define M_NETIO_NETIO_INCLUDE

#include "config.hpp"
M_NETIO_NAMESPACE_BEGIN

#ifdef M_PLATFORM_WIN
#pragma pack(1)
struct MessageHeader {
	int			   timestamp;
	unsigned short size;
	unsigned char  endian; // 0 == little endian, 1 == big endian
};
#pragma pack()
#else
struct __attribute__((__packed__)) MessageHeader {
	int			   timestamp;
	unsigned short size;
	unsigned char  endian; // 0 == little endian, 1 == big endian
};
#endif

class NetIo;
class TcpSocket;
class TcpConnector;

typedef SocketLib::Buffer Buffer;
typedef shard_ptr_t<SocketLib::Buffer> BufferPtr;
typedef shard_ptr_t<TcpSocket>		   TcpSocketPtr;
typedef shard_ptr_t<TcpConnector>	   TcpConnectorPtr;
typedef shard_ptr_t<SocketLib::TcpAcceptor<SocketLib::IoService> > NetIoTcpAcceptorPtr;

typedef function_t<bool(TcpSocketPtr, MessageHeader&, Buffer&)> MessageChecker;
typedef function_t<bool(TcpConnectorPtr, MessageHeader&, Buffer&)> MessageChecker2;

#define lasterror tlsdata<SocketLib::SocketError,0>::data()

// class netio
class NetIo 
{
public:
	NetIo();

	NetIo(SocketLib::s_uint32_t backlog);

	virtual ~NetIo();

	// ����һ������
	bool ListenOne(const SocketLib::Tcp::EndPoint& ep);
	bool ListenOne(const std::string& addr, SocketLib::s_uint16_t port);

	// �첽����
	void ConnectOne(const SocketLib::Tcp::EndPoint& ep, unsigned int data=0);
	void ConnectOne(const std::string& addr, SocketLib::s_uint16_t port, unsigned int data=0);

	virtual void Run();
	virtual void Stop();

	// ��ȡ�����쳣
	inline SocketLib::SocketError GetLastError()const;
	inline SocketLib::IoService& GetIoService();
	inline SocketLib::s_uint32_t LocalEndian()const;

	/*
	 *����������������Ϊ�麯�����Ա����ʵ��ҵ���ģʽ��������ģʽ����Ϣ���ַ�����
	 *��֤ͬһ��socket���������������ĵ�����ѭOnConnected -> OnReceiveData -> OnDisconnected��˳��
	 *��֤ͬһ��socket�����º����������ĵ��ö���ͬһ���߳���
	 */

	// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnConnected(TcpSocketPtr& clisock);
	virtual void OnConnected(TcpConnectorPtr& clisock,SocketLib::SocketError error);

	// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnDisconnected(TcpSocketPtr& clisock);
	virtual void OnDisconnected(TcpConnectorPtr& clisock);

	// ���ݰ�֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnReceiveData(TcpSocketPtr& clisock, SocketLib::Buffer& buffer);
	virtual void OnReceiveData(TcpConnectorPtr& clisock, SocketLib::Buffer& buffer);

protected:
	void AcceptHandler(SocketLib::SocketError error, TcpSocketPtr& clisock, NetIoTcpAcceptorPtr& acceptor);

protected:
	NetIo(const NetIo&);
	NetIo& operator=(const NetIo&);

private:
	SocketLib::IoService   _ioservice;
	SocketLib::s_uint32_t  _backlog;
	SocketLib::s_uint32_t  _endian;
};

enum {
	E_STATE_STOP =  0,
	E_STATE_START = 1,
	E_STATE_WRITE = 3,
};

template<typename T, typename SocketType,typename CheckerType>
class TcpBaseSocket : public enable_shared_from_this_t<T>
{
protected:
	struct _readerinfo_ {
		SocketLib::s_byte_t*  readbuf;
		SocketLib::Buffer	  msgbuffer;
		SocketLib::Buffer	  msgbuffer2;
		MessageHeader		  curheader;
		//SocketLib::MutexLock  lock;

		_readerinfo_();
		~_readerinfo_();
	};
	struct _writerinfo_ {
		SocketLib::slist<SocketLib::Buffer*> buffer_pool;
		SocketLib::slist<SocketLib::Buffer*> buffer_pool2;
		SocketLib::Buffer*   msgbuffer;
		SocketLib::MutexLock lock;
		bool writing;

		_writerinfo_();
		~_writerinfo_();
	};

public:
	TcpBaseSocket(NetIo& netio, CheckerType checker);

	virtual ~TcpBaseSocket();

	const SocketLib::Tcp::EndPoint& LocalEndpoint()const;

	const SocketLib::Tcp::EndPoint& RemoteEndpoint()const;

	// ���������������ζ�����������Ͽ���������е�δ��������ݴ�����ͻ�Ͽ�
	void Close();

	void Send(SocketLib::Buffer*);

	void Send(const SocketLib::s_byte_t* data, SocketLib::s_uint32_t len);

protected:
	void _WriteHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

	void _ReadHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

	inline void _CloseHandler();

	void _PostClose(unsigned int state);

	void _Close(unsigned int state);

	// �ü������ݰ�������false��ζ�����ݰ��д�
	bool _CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t tran_byte);

	bool _TrySendData(bool ignore = false);

	void _TryRecvData();

protected:
	NetIo& _netio;
	SocketType* _socket;
	CheckerType _msgchecker;

	_readerinfo_ _reader;
	_writerinfo_ _writer;

	// endpoint
	SocketLib::Tcp::EndPoint _localep;
	SocketLib::Tcp::EndPoint _remoteep;

	// ״̬��־
	unsigned short _flag;
};

// class tcpsocket
class TcpSocket : public TcpBaseSocket<TcpSocket, SocketLib::TcpSocket<SocketLib::IoService>,
	MessageChecker>
{
	friend class NetIo;
public:
	typedef TcpBaseSocket<TcpSocket, SocketLib::TcpSocket<SocketLib::IoService>,
		MessageChecker> BaseSelf;
public:
	TcpSocket(NetIo& netio, MessageChecker checker);

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket();

protected:
	void Init();
};

class TcpConnector : public TcpBaseSocket<TcpConnector, SocketLib::TcpConnector<SocketLib::IoService>,
	MessageChecker2>
{
	typedef TcpBaseSocket<TcpConnector, SocketLib::TcpConnector<SocketLib::IoService>,
		MessageChecker2> BaseSelf;
public:
	TcpConnector(NetIo& netio, MessageChecker2 checker);

	SocketLib::TcpConnector<SocketLib::IoService>& GetSocket();

	bool Connect(const SocketLib::Tcp::EndPoint& ep);

	bool Connect(const std::string& addr, SocketLib::s_uint16_t port);

	void AsyncConnect(const SocketLib::Tcp::EndPoint& ep);

	void AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port);

	inline void SetData(unsigned int data);

	inline unsigned int GetData()const;

protected:
	void _ConnectHandler(const SocketLib::SocketError& error, TcpConnectorPtr conector);

protected:
	unsigned int _data;
};

M_NETIO_NAMESPACE_END
#include "netio_impl.hpp"
#include "tsocket_impl.hpp"
#include "tconnector_impl.hpp"
#endif