/*
* breeze License
* Copyright (C) 2014 YaweiZhang <yawei_zhang@foxmail.com>.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


//! ����



#include <zsummerX/FrameHeader.h>
#include <zsummerX/FrameTcpSessionManager.h>
#include <zsummerX/FrameMessageDispatch.h>
#include <unordered_map>
#include <ProtoDefine.h>
#include <ProtoCommon.h>
#include <ProtoAuth.h>
using namespace zsummer::log4z;

//! Ĭ����������
std::string g_remoteIP = "0.0.0.0"; //�����Ϊ�������ʹ�� �����Ǽ�����ַ
unsigned short g_remotePort = 81; //ͬ��
unsigned short g_maxClient = 1; //����Ƿ���� ���������ƿͻ��˵ĸ��� �����Ļᱻ�ߵ�, ����ǿͻ��� �����������Ŀͻ�������.
//!�շ�������ͳ������
unsigned long long g_totalEchoCount = 0;
unsigned long long g_lastEchoCount = 0;
unsigned long long g_totalSendCount = 0;
unsigned long long g_totalRecvCount = 0;
void MonitorFunc()
{
	LOGI("per seconds Echos Count=" << (g_totalEchoCount - g_lastEchoCount) / 5
		<< ", g_totalSendCount[" << g_totalSendCount << "] g_totalRecvCount[" << g_totalRecvCount << "]");
	g_lastEchoCount = g_totalEchoCount;
	CTcpSessionManager::getRef().CreateTimer(5000, MonitorFunc);
};


class CStressHeartBeatManager
{
public:
	CStressHeartBeatManager()
	{
		//! ע���¼�����Ϣ
		CMessageDispatcher::getRef().RegisterOnConnectorEstablished(std::bind(&CStressHeartBeatManager::OnConnecotrConnected, this,
			std::placeholders::_1));
		CMessageDispatcher::getRef().RegisterOnMyConnectorHeartbeatTimer(std::bind(&CStressHeartBeatManager::OnConnecotrHeartbeatTimer, this,
			std::placeholders::_1));
		CMessageDispatcher::getRef().RegisterOnConnectorDisconnect(std::bind(&CStressHeartBeatManager::OnConnecotrDisconnect, this,
			std::placeholders::_1));
	}
	
	void OnConnecotrConnected(ConnectorID cID)
	{
		m_connectorHB[cID] = time(NULL);
		LOGI("connect sucess. cID=" << cID);
	}
	void OnConnecotrHeartbeatTimer(ConnectorID cID)
	{

	}
	void OnConnecotrDisconnect(ConnectorID cID)
	{
		m_connectorHB.erase(cID);
		LOGI("Disconnect. cID=" << cID);
	}

	void OnMsgServerHeartbeat(ConnectorID cID, ProtocolID pID, ReadStreamPack & pack)
	{
		auto iter = m_connectorHB.find(cID);
		if (iter != m_connectorHB.end())
		{
			iter->second = time(NULL);
		}
	}


private:
	std::unordered_map<ConnectorID, time_t> m_connectorHB;
};



class CStressClientHandler
{
public:
	CStressClientHandler()
	{
		CMessageDispatcher::getRef().RegisterOnConnectorEstablished(std::bind(&CStressClientHandler::OnConnected, this, std::placeholders::_1));
		CMessageDispatcher::getRef().RegisterConnectorMessage(ID_AS2C_AuthAck,
			std::bind(&CStressClientHandler::msg_AuthAck_fun, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		CMessageDispatcher::getRef().RegisterOnConnectorDisconnect(std::bind(&CStressClientHandler::OnConnectDisconnect, this, std::placeholders::_1));
	}

	void OnConnected (ConnectorID cID)
	{
		LOGI("OnConnected. ConnectorID=" << cID );
		WriteStreamPack ws;
		ProtoAuthReq req;
		req.info.user = "zhangyawei";
		req.info.pwd = "123";
		ws << ID_C2AS_AuthReq << req;
		CTcpSessionManager::getRef().SendOrgConnectorData(cID, ws.GetStream(), ws.GetStreamLen());
		LOGI("OnConnected. Send AuthReq. cID=" << cID << ", user=" << req.info.user << ", pwd=" << req.info.pwd);
		g_totalSendCount++;
	};
	void OnConnectDisconnect(ConnectorID cID)
	{
		m_sessionStatus[cID] = false;
	}

	void msg_AuthAck_fun(ConnectorID cID, ProtocolID pID, ReadStreamPack & rs)
	{
		
		ProtoAuthAck ack;
		rs >> ack;
		if (ack.retCode == EC_SUCCESS)
		{
			LOGI("Auth Success. cID=" << cID);
		}
		else
		{
			LOGE("Auth Failed. cID=" << cID);
		}
		

		g_totalRecvCount++;
		g_totalEchoCount++;

	};
private:
	std::unordered_map<ConnectorID, bool> m_sessionStatus;
};



int main(int argc, char* argv[])
{

#ifndef _WIN32
	//! linux����Ҫ���ε�һЩ�ź�
	signal( SIGHUP, SIG_IGN );
	signal( SIGALRM, SIG_IGN ); 
	signal( SIGPIPE, SIG_IGN );
	signal( SIGXCPU, SIG_IGN );
	signal( SIGXFSZ, SIG_IGN );
	signal( SIGPROF, SIG_IGN ); 
	signal( SIGVTALRM, SIG_IGN );
	signal( SIGQUIT, SIG_IGN );
	signal( SIGCHLD, SIG_IGN);
#endif
	if (argc == 2 && 
		(strcmp(argv[1], "--help") == 0 
		|| strcmp(argv[1], "/?") == 0))
	{
		cout << "please input like example:" << endl;
		cout << "tcpTest remoteIP remotePort maxClient" << endl;
		cout << "./tcpTest 0.0.0.0 81 1" << endl;
		cout << "maxClient: limit max" << endl;
		return 0;
	}
	if (argc > 1)
		g_remoteIP = argv[1];
	if (argc > 2)
		g_remotePort = atoi(argv[2]);
	if (argc > 3)
		g_maxClient = atoi(argv[3]);

	ILog4zManager::GetInstance()->Config("client.cfg");
	ILog4zManager::GetInstance()->Start();


	LOGI("g_remoteIP=" << g_remoteIP << ", g_remotePort=" << g_remotePort << ", g_maxClient=" << g_maxClient );


//	ILog4zManager::GetInstance()->SetLoggerLevel(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_INFO);



	CTcpSessionManager::getRef().Start();


//	CTcpSessionManager::getRef().CreateTimer(5000, MonitorFunc);

	//������������handler��ʵ�� ֻҪ��������, ���캯���л�ע���Ӧ�¼�
	CStressHeartBeatManager statusManager;

	//���ﴴ������handler�Ϳͻ���handler ��������������ͬ��Ӳ�ͬ��ɫ.
	CStressClientHandler client;
	
	//��Ӷ��connector.
	for (int i = 0; i < g_maxClient; ++i)
	{
		tagConnctorConfigTraits traits;
		traits.cID = i;
		traits.remoteIP = g_remoteIP;
		traits.remotePort = g_remotePort;
		traits.reconnectInterval = 5000;
		traits.reconnectMaxCount = 50;
		CTcpSessionManager::getRef().AddConnector(traits);
	}
	
	//������ѭ��.
	CTcpSessionManager::getRef().Run();

	return 0;
}

