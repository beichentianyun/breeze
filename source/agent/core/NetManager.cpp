#include "NetManager.h"
#include <InProtoCommon.h>
#include <ServerConfig.h>

CNetManager::CNetManager()
{
	CMessageDispatcher::getRef().RegisterSessionMessage(ID_C2AS_AuthReq,
		std::bind(&CNetManager::msg_AuthReq, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	CMessageDispatcher::getRef().RegisterSessionDefaultMessage(
		std::bind(&CNetManager::msg_DefaultReq, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	CMessageDispatcher::getRef().RegisterSessionOrgMessage(
		std::bind(&CNetManager::msg_OrgMessageReq, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));


	CMessageDispatcher::getRef().RegisterConnectorMessage(ID_AS2C_AuthAck,
		std::bind(&CNetManager::msg_AuthAck, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));


	//ע���¼�
	CMessageDispatcher::getRef().RegisterOnConnectorEstablished(std::bind(&CNetManager::event_OnConnect, this, std::placeholders::_1));
	CMessageDispatcher::getRef().RegisterOnConnectorDisconnect(std::bind(&CNetManager::event_OnDisconnect, this, std::placeholders::_1));
	CMessageDispatcher::getRef().RegisterOnSessionEstablished(std::bind(&CNetManager::event_OnSessionEstablished, this, std::placeholders::_1, std::placeholders::_1));
	CMessageDispatcher::getRef().RegisterOnSessionDisconnect(std::bind(&CNetManager::event_OnSessionDisconnect, this, std::placeholders::_1, std::placeholders::_1));
}

bool CNetManager::Start()
{

	auto connecters = GlobalFacade::getRef().getServerConfig().getConfigConnect(AgentNode);
	for (auto con : connecters)
	{
		tagConnctorConfigTraits tag;
		tag.cID = m_lastConnectID++;
		tag.remoteIP = con.remoteIP;
		tag.remotePort = con.remotePort;
		tag.reconnectMaxCount = 2;
		tag.reconnectInterval = 5000;
		tag.curReconnectCount = true;
		if (con.dstNode == CenterNode)
		{
			m_configCenter.insert(std::make_pair(tag.cID, tag));
		}
		else if (con.dstNode == AuthNode)
		{
			m_configAuth.insert(std::make_pair(tag.cID, tag));
		}
		else
		{
			continue;
		}
		if (CTcpSessionManager::getRef().AddConnector(tag) == InvalidConnectorID)
		{
			LOGE("AddConnector failed. remoteIP=" << tag.remoteIP << ", remotePort=" << tag.remotePort);
			return false;
		}
	}

	m_configListen.aID = 1;
	m_configListen.listenIP = GlobalFacade::getRef().getServerConfig().getConfigListen(AgentNode).ip;
	m_configListen.listenPort = GlobalFacade::getRef().getServerConfig().getConfigListen(AgentNode).port;
	m_configListen.maxSessions = 5000;

	LOGI("CNetManager Init Success.");
	return true;
}


void CNetManager::event_OnConnect(ConnectorID cID)
{
	auto founder = m_configAuth.find(cID);
	if (founder != m_configAuth.end())
	{
		m_onlineAuth.push_back(cID);
		LOGI("event_OnConnect Auth. cID=" << cID << ", listenIP=" << founder->second.remoteIP << ", listenPort=" << founder->second.remotePort);
	}
	founder = m_configCenter.find(cID);
	if (founder != m_configCenter.end())
	{
		m_onlineCenter.push_back(cID);
		LOGI("event_OnConnect Center. cID=" << cID << ", listenIP=" << founder->second.remoteIP << ", listenPort=" << founder->second.remotePort);
	}
	if (m_configAuth.size() + m_configCenter.size() != m_onlineAuth.size() + m_onlineCenter.size())
	{
		return;
	}

	//init
	WriteStreamPack ws;
	ProtoDirectServerInit init;
	init.srcServer = GlobalFacade::getRef().getServerConfig().getOwnServerNode();
	init.srcIndex = GlobalFacade::getRef().getServerConfig().getOwnNodeIndex();
	ws << ID_DT2OS_DirectServerInit << init;
	CTcpSessionManager::getRef().SendOrgConnectorData(cID, ws.GetStream(), ws.GetStreamLen());

	//����connector�Ѿ��������ӳɹ� �����ǳ�������ʱ�ĵ�һ�� ��ʱ�򿪿ͻ��˼����˿�
	if (CTcpSessionManager::getRef().AddAcceptor(m_configListen) == InvalidAccepterID)
	{
		LOGE("AddAcceptor Failed. listenIP=" << m_configListen.listenIP << ", listenPort=" << m_configListen.listenPort);
		return;
	}
	LOGI("AddAcceptor Success. listenIP=" << m_configListen.listenIP << ", listenPort=" << m_configListen.listenPort);
}
void CNetManager::event_OnDisconnect(ConnectorID cID)
{
	auto founder = m_configAuth.find(cID);
	if (founder != m_configAuth.end())
	{
		auto searchiter = std::find_if(m_onlineAuth.begin(), m_onlineAuth.end(), [&cID](ConnectorID ccID){ return ccID == cID; });
		if (searchiter != m_onlineAuth.end())
		{
			m_onlineAuth.erase(searchiter);
		}
		LOGW("event_OnDisconnect Auth. cID=" << cID << ", listenIP=" << founder->second.remoteIP << ", listenPort=" << founder->second.remotePort);
	}
	founder = m_configCenter.find(cID);
	if (founder != m_configCenter.end())
	{
		auto searchiter = std::find_if(m_onlineCenter.begin(), m_onlineCenter.end(), [&cID](ConnectorID ccID){ return ccID == cID; });
		if (searchiter != m_onlineCenter.end())
		{
			m_onlineCenter.erase(searchiter);
		}
		LOGW("event_OnDisconnect Center. cID=" << cID << ", listenIP=" << founder->second.remoteIP << ", listenPort=" << founder->second.remotePort);
	}
}


void CNetManager::event_OnSessionEstablished(AccepterID, SessionID)
{

}
void CNetManager::event_OnSessionDisconnect(AccepterID aID, SessionID sID)
{
	auto founder = m_mapSession.find(sID);
	if (founder == m_mapSession.end())
	{
		return;
	}

	m_mapSession.erase(founder);
}






void CNetManager::msg_AuthReq(AccepterID aID, SessionID sID, ProtocolID pID, ReadStreamPack & rs)
{
	ProtoAuthReq req;
	rs >> req;
	LOGD("ID_C2AS_AuthReq user=" << req.info.user << ", pwd=" << req.info.pwd);
// 	//debug �����ظ���֤ ����֤ѹ��
	m_mapSession.erase(sID);
// 	//end
	auto finditer = m_mapSession.find(sID);
	if (finditer != m_mapSession.end())
	{
		WriteStreamPack ws;
		ProtoAuthAck ack;
		ack.retCode = EC_AUTH_ING;
		ws << ID_AS2C_AuthAck << ack;
		CTcpSessionManager::getRef().SendOrgSessionData(aID, sID, ws.GetStream(), ws.GetStreamLen());
		return;
	}
	std::shared_ptr<AgentSessionInfo> sinfo(new AgentSessionInfo);
	sinfo->sInfo.agentIndex = GlobalFacade::getRef().getServerConfig().getOwnNodeIndex();
	sinfo->sInfo.aID = aID;
	sinfo->sInfo.sID = sID;
	m_mapSession.insert(std::make_pair(sID, sinfo));

	if (m_onlineAuth.empty())
	{
		WriteStreamPack ws;
		ProtoAuthAck ack;
		ack.retCode = EC_SERVER_ERROR;
		ws << ID_AS2C_AuthAck << ack;
		CTcpSessionManager::getRef().SendOrgSessionData(aID, sID, ws.GetStream(), ws.GetStreamLen());
		return;
	}

	WriteStreamPack ws;
	ws << ID_C2AS_AuthReq << sinfo->sInfo << req ;
	auto authID = m_onlineAuth.at(rand() % m_onlineAuth.size());
	CTcpSessionManager::getRef().SendOrgConnectorData(authID, ws.GetStream(), ws.GetStreamLen());
}

void CNetManager::msg_AuthAck(ConnectorID cID, ProtocolID pID, ReadStreamPack &rs)
{
	SessionInfo info;
	ProtoAuthAck ack;
	rs >> info;
	rs >> ack;

	auto founder = m_mapSession.find(info.sID);
	if (founder == m_mapSession.end())
	{
		LOGE("msg_AuthAck can not found session ID.  sID=" << info.sID);
		return;
	}

	if (ack.retCode == EC_SUCCESS)
	{
		founder->second->sInfo.accID = ack.accountID;
	}

	WriteStreamPack ws;
	ws << ID_AS2C_AuthAck << ack;
	CTcpSessionManager::getRef().SendOrgSessionData(founder->second->sInfo.aID, founder->second->sInfo.sID, ws.GetStream(), ws.GetStreamLen());
}

bool CNetManager::msg_OrgMessageReq(AccepterID aID, SessionID sID, const char * blockBegin,  FrameStreamTraits::Integer blockSize)
{
	ReadStreamPack pack(blockBegin, blockSize);
	ProtocolID protoID = InvalidProtocolID;
	pack >> protoID;
	if (isClientPROTO(protoID) && isNeedAuthClientPROTO(protoID))
	{
		auto finditer = m_mapSession.find(sID);
		if (finditer == m_mapSession.end() || finditer->second->sInfo.accID == InvalidAccountID)
		{
			LOGW("msg_OrgMessageReq check false. sID=" << sID);
			return false;
		}
	}
	return true;
};

void CNetManager::msg_DefaultReq(AccepterID aID, SessionID sID, ProtocolID pID, ReadStreamPack & rs)
{
	auto finditer = m_mapSession.find(sID);
	ProtoAuthAck ack;
	ack.retCode = EC_SUCCESS;
	ack.accountID = InvalidAccountID;
	if (finditer == m_mapSession.end() || finditer->second->sInfo.accID == InvalidAccountID)
	{
		ack.retCode = EC_AUTH_ERROR;
	}
	else
	{
		ProtocolID inProtoID = InvalidProtocolID;
		if (isNeedAuthClientPROTO(pID))
		{
			inProtoID = ID_RT2OS_RouteToOtherServer;
			ProtoRouteToOtherServer route;
			route.srcIndex = GlobalFacade::getRef().getServerConfig().getOwnNodeIndex();
			route.srcServer = GlobalFacade::getRef().getServerConfig().getOwnServerNode();
			route.dstServer = LogicNode;
			route.routerType = 3;
			route.dstIndex = 0;
			WriteStreamPack ws;
			ws << inProtoID << route << pID << finditer->second->sInfo;
			ws.AppendOriginalData(rs.GetStreamUnread(), rs.GetStreamUnreadLen());
			CTcpSessionManager::getRef().SendOrgSessionData(aID, sID, ws.GetStream(), ws.GetStreamLen());
			return;
		}
	}

	if (ack.retCode != EC_SUCCESS)
	{
		WriteStreamPack ws;
		ws << ID_AS2C_AuthAck << ack;
		CTcpSessionManager::getRef().SendOrgSessionData(aID, sID, ws.GetStream(), ws.GetStreamLen());
	}

};


