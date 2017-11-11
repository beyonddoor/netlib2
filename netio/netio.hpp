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
	unsigned short checksum;
	unsigned short size;
	int			   timestamp;
	unsigned char  endian; // 0 == little endian, 1 == big endian
};
#pragma pack()
#else
struct __attribute__((__packed__)) MessageHeader {
	unsigned short checksum;
	unsigned short size;
	int			   timestamp;
	unsigned char  endian; // 0 == little endian, 1 == big endian
};
#endif

class NetIo;
class TcpSocket;

typedef shard_ptr_t<SocketLib::Buffer> BufferPtr;
typedef shard_ptr_t<TcpSocket>		   TcpSocketPtr;
typedef shard_ptr_t<SocketLib::TcpAcceptor<SocketLib::IoService> > NetIoTcpAcceptorPtr;

typedef function_t<void(TcpSocketPtr, BufferPtr)> MessageReceiver;
typedef function_t<bool(TcpSocketPtr, const MessageHeader&)> MessageHeaderChecker;

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

	void Run();

	void Stop();

	// ��ȡ�����쳣
	inline SocketLib::SocketError GetLastError()const;

	inline SocketLib::IoService& GetIoService();

	/*
	 *����������������Ϊ�麯�����Ա����ʵ��ҵ���ģʽ��������ģʽ����Ϣ���ַ�����
	 *��֤ͬһ��socket���������������ĵ�����ѭOnConnected -> OnReceiveData -> OnDisconnected��˳��
	 *��֤ͬһ��socket�����º����������ĵ��ö���ͬһ���߳���
	 */

	// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnConnected(TcpSocketPtr clisock);

	// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnDisconnected(TcpSocketPtr clisock);

	// ���ݰ�֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnReceiveData(TcpSocketPtr clisock, BufferPtr buffer);

protected:
	void AcceptHandler(SocketLib::SocketError error, TcpSocketPtr clisock, NetIoTcpAcceptorPtr acceptor);

protected:
	NetIo(const NetIo&);
	NetIo& operator=(const NetIo&);

private:
	SocketLib::IoService   _ioservice;
	SocketLib::s_uint32_t  _backlog;

	std::list<TcpSocketPtr> _connected_list;
	std::list<TcpSocketPtr> _disconnected_list; // ����
};

// class tcpsocket
class TcpSocket : public enable_shared_from_this_t<TcpSocket>
{
public:
	struct _readerinfo_ {
		SocketLib::s_byte_t* readbuf;
		SocketLib::Buffer	 msgbuffer;
		SocketLib::MutexLock lock;

		_readerinfo_();
		~_readerinfo_();
	};
	struct _writerinfo_ {
		std::list<SocketLib::Buffer*> buffer_pool;
		SocketLib::MutexLock lock;
		bool writing;

		_writerinfo_();
	};

public:
	TcpSocket(NetIo& netio, MessageReceiver receiver, MessageHeaderChecker checker);

	~TcpSocket();

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket();

	void Init();

	const SocketLib::Tcp::EndPoint& LocalEndpoint()const;

	const SocketLib::Tcp::EndPoint& RemoteEndpoint()const;

	// ���������������ζ�����������Ͽ���������е�δ��������ݴ�����ͻ�Ͽ�
	void PostClose();

	void Send(SocketLib::Buffer*);

protected:
	void WriteHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error);

	void ReadHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error);

	void Close();

	// �ü������ݰ�������false��ζ�����ݰ��д�
	bool CutMsgPack();

	bool TrySendData();

private:
	NetIo& _netio;
	SocketLib::TcpSocket<SocketLib::IoService>* _socket;

	MessageReceiver		 _message_receiver;
	MessageHeaderChecker _header_checker;

	_readerinfo_ _reader;
	_writerinfo_ _writer;

	// endpoint
	SocketLib::Tcp::EndPoint _localep;
	SocketLib::Tcp::EndPoint _remoteep;

	bool _stopped; // stop flag
};

M_NETIO_NAMESPACE_END
#include "netio_impl.hpp"
#include "tsocket_impl.hpp"
#endif