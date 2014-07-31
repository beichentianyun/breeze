#include "NetManager.h"

CNetManager::CNetManager()
{

	CMessageDispatcher::getRef().RegisterSessionMessage(ID_DT2OS_DirectServerAuth,
		std::bind(&CNetManager::msg_SessionServerAuth, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

	CMessageDispatcher::getRef().RegisterConnectorMessage(ID_DT2OS_DirectServerAuth,
		std::bind(&CNetManager::msg_ConnectServerAuth, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	CMessageDispatcher::getRef().RegisterSessionDefaultMessage(
		std::bind(&CNetManager::msg_DefaultSessionReq, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	CMessageDispatcher::getRef().RegisterSessionOrgMessage(
		std::bind(&CNetManager::msg_OrgMessageReq, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));


	CMessageDispatcher::getRef().RegisterConnectorDefaultMessage(
		std::bind(&CNetManager::msg_DefaultConnectReq, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	//ע���¼�
	CMessageDispatcher::getRef().RegisterOnConnectorEstablished(std::bind(&CNetManager::event_OnConnect, this, std::placeholders::_1));
	CMessageDispatcher::getRef().RegisterOnConnectorDisconnect(std::bind(&CNetManager::event_OnDisconnect, this, std::placeholders::_1));
	CMessageDispatcher::getRef().RegisterOnSessionEstablished(std::bind(&CNetManager::event_OnSessionEstablished, this, std::placeholders::_1, std::placeholders::_2));
	CMessageDispatcher::getRef().RegisterOnSessionDisconnect(std::bind(&CNetManager::event_OnSessionDisconnect, this, std::placeholders::_1, std::placeholders::_2));
}

bool CNetManager::Start()
{

	auto connecters = GlobalFacade::getRef().getServerConfig().getConfigConnect(CenterNode);
	for (auto con : connecters)
	{
		tagConnctorConfigTraits tag;
		tag.cID = m_lastConnectID++;
		tag.remoteIP = con.remoteIP;
		tag.remotePort = con.remotePort;
		tag.reconnectMaxCount = 2;
		tag.reconnectInterval = 5000;
		tag.curReconnectCount = true;
		if (con.dstNode == LogicNode)
		{
			m_configConnect.insert(std::make_pair(tag.cID, tag));
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
	m_configListen.listenIP = GlobalFacade::getRef().getServerConfig().getConfigListen(CenterNode).ip;
	m_configListen.listenPort = GlobalFacade::getRef().getServerConfig().getConfigListen(CenterNode).port;
	m_configListen.maxSessions = 50;

	LOGI("CNetManager Init Success.");
	return true;
}


void CNetManager::event_OnConnect(ConnectorID cID)
{
	WriteStreamPack ws;
	ProtoDirectServerAuth auth;
	auth.srcNode = GlobalFacade::getRef().getServerConfig().getOwnServerNode();
	auth.srcIndex = GlobalFacade::getRef().getServerConfig().getOwnNodeIndex();
	ws << ID_DT2OS_DirectServerAuth << auth;
	CTcpSessionManager::getRef().SendOrgConnectorData(cID, ws.GetStream(), ws.GetStreamLen());
	LOGI("send ProtoDirectServerAuth cID=" << cID << ", node=" << auth.srcNode << ", index=" << auth.srcIndex)
}

void CNetManager::event_OnDisconnect(ConnectorID cID)
{
	auto founder = m_configConnect.find(cID);
	if (founder != m_configConnect.end())
	{
		LOGW("event_OnDisconnect Center. cID=" << cID << ", listenIP=" << founder->second.remoteIP << ", listenPort=" << founder->second.remotePort);
	}

	{
		auto searchiter = std::find_if(m_onlineConnect.begin(), m_onlineConnect.end(),
			[&cID](const ServerAuthConnect & sac){ return sac.cID == cID; });
		if (searchiter != m_onlineConnect.end())
		{
			m_onlineConnect.erase(searchiter);
		}
	}
}


void CNetManager::event_OnSessionEstablished(AccepterID aID, SessionID sID)
{
	WriteStreamPack ws;
	ProtoDirectServerAuth auth;
	auth.srcNode = GlobalFacade::getRef().getServerConfig().getOwnServerNode();
	auth.srcIndex = GlobalFacade::getRef().getServerConfig().getOwnNodeIndex();
	ws << ID_DT2OS_DirectServerAuth << auth;
	CTcpSessionManager::getRef().SendOrgSessionData(aID, sID, ws.GetStream(), ws.GetStreamLen());
	LOGD("send ProtoDirectServerAuth aID=" << aID << ", sID=" << sID << ", node=" << auth.srcNode << ", index=" << auth.srcIndex);
}

void CNetManager::event_OnSessionDisconnect(AccepterID aID, SessionID sID)
{
	auto founder = std::find_if(m_onlineSession.begin(), m_onlineSession.end(),
		[&sID](const ServerAuthSession & sac){ return sac.sID == sID; });
	if (founder != m_onlineSession.end())
	{
		m_onlineSession.erase(founder);
	}
}



void CNetManager::msg_ConnectServerAuth(ConnectorID cID, ProtocolID pID, ReadStreamPack &rs)
{
	ProtoDirectServerAuth auth;
	rs >> auth;
	LOGI("msg_ConnectServerAuth. cID=" << cID << ", Node=" << auth.srcNode << ", index=" << auth.srcIndex);

	if (auth.srcNode == DBAgentNode)
	{
		auto founder = std::find_if(m_onlineConnect.begin(), m_onlineConnect.end(),
			[auth](const ServerAuthConnect &sac){return sac.index == auth.srcIndex; });
		if (founder != m_onlineConnect.end())
		{
			m_onlineConnect.erase(founder);
		}
		ServerAuthConnect sac;
		sac.cID = cID;
		sac.node = auth.srcNode;
		sac.index = auth.srcIndex;
		m_onlineConnect.push_back(sac);
	}

	if (m_onlineConnect.size() == m_onlineConnect.size())
	{
		if (CTcpSessionManager::getRef().AddAcceptor(m_configListen) == InvalidAccepterID)
		{
			LOGE("AddAcceptor Failed. listenIP=" << m_configListen.listenIP << ", listenPort=" << m_configListen.listenPort);
			return;
		}
	}
}


void CNetManager::msg_SessionServerAuth(AccepterID aID, SessionID sID, ProtocolID pID, ReadStreamPack & rs)
{
	ProtoDirectServerAuth auth;
	rs >> auth;
	LOGI("msg_SessionServerAuth. aID=" << aID << ", sID=" << sID << ", Node=" << auth.srcNode << ", index=" << auth.srcIndex);

	ServerAuthSession sas;
	sas.aID = aID;
	sas.sID = sID;
	sas.node = auth.srcNode;
	sas.index = auth.srcIndex;

	auto founder = std::find_if(m_onlineSession.begin(), m_onlineSession.end(),
		[auth](const ServerAuthSession & sas){ return sas.index == auth.srcIndex; });
	if (founder != m_onlineSession.end())
	{
		m_onlineSession.erase(founder);
	}
	m_onlineSession.push_back(sas);
}


bool CNetManager::msg_OrgMessageReq(AccepterID aID, SessionID sID, const char * blockBegin,  FrameStreamTraits::Integer blockSize)
{
	return true;
};

void CNetManager::msg_DefaultConnectReq(ConnectorID cID, ProtocolID pID, ReadStreamPack & rs)
{
	msg_DefaultSessionReq(InvalidAccepterID, InvalidSeesionID, pID, rs);
}

void CNetManager::msg_DefaultSessionReq(AccepterID aID, SessionID sID, ProtocolID pID, ReadStreamPack & rs)
{
	if (pID == ID_RT2OS_RouteToOtherServer)
	{
		ProtoRouteToOtherServer route;
		rs >> route;

		if (route.routerType == 0 || route.routerType == 1)
		{
			if (route.dstNode == LogicNode)
			{
				auto founder = std::find_if(m_onlineConnect.begin(), m_onlineConnect.end(),
					[route](const ServerAuthConnect & sas){return sas.node == route.dstNode && (route.routerType == 3 || sas.index == route.dstIndex); });
				if (founder != m_onlineConnect.end())
				{
					WriteStreamPack  ws;
					ws.AppendOriginalData(rs.GetStreamUnread(), rs.GetStreamUnreadLen());
					CTcpSessionManager::getRef().SendOrgConnectorData(founder->cID, ws.GetStream(), ws.GetStreamLen());
					return;
				}
			}
			else
			{
				auto founder = std::find_if(m_onlineSession.begin(), m_onlineSession.end(),
					[route](const ServerAuthSession & sas){return sas.node == route.dstNode && (route.routerType == 3 || sas.index == route.dstIndex); });
				if (founder != m_onlineSession.end())
				{
					WriteStreamPack  ws;
					ws.AppendOriginalData(rs.GetStreamUnread(), rs.GetStreamUnreadLen());
					CTcpSessionManager::getRef().SendOrgSessionData(founder->aID, founder->sID, ws.GetStream(), ws.GetStreamLen());
					return;
				}
			}
		}
		else if (route.routerType == 2)
		{
			if (route.dstNode == LogicNode)
			{
				for (const auto & c : m_onlineConnect)
				{
					if (c.node == route.dstNode)
					{
						WriteStreamPack  ws;
						ws.AppendOriginalData(rs.GetStreamUnread(), rs.GetStreamUnreadLen());
						CTcpSessionManager::getRef().SendOrgConnectorData(c.cID, ws.GetStream(), ws.GetStreamLen());
					}
				}
			}
			else
			{
				for (const auto & s : m_onlineSession)
				{
					if (s.node == route.dstNode)
					{
						WriteStreamPack  ws;
						ws.AppendOriginalData(rs.GetStreamUnread(), rs.GetStreamUnreadLen());
						CTcpSessionManager::getRef().SendOrgSessionData(s.aID, s.sID, ws.GetStream(), ws.GetStreamLen());
					}
				}
			}
		}
		
	}


};

