
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

#ifndef _NET_MANAGER_H_
#define _NET_MANAGER_H_

#include <ProtoDefine.h>
#include <ProtoCommon.h>
#include <ProtoInCommon.h>
#include <ProtoAuth.h>
#include "../core/GlobalFacade.h"
#include <ServerConfig.h>
#include <zsummerX/FrameMessageDispatch.h>
#include <zsummerX/FrameTcpSessionManager.h>

/*
* NetManager
*/


class CNetManager
{
public:
	CNetManager();
	//����������֤������������
	bool Start();
	void event_OnConnect(ConnectorID cID);
	void event_OnDisconnect(ConnectorID cID);

	void event_OnSessionEstablished(AccepterID, SessionID);
	void event_OnSessionDisconnect(AccepterID, SessionID);

	void msg_serverInit(AccepterID aID, SessionID sID, ProtocolID pID, ReadStreamPack & rs);


	void msg_DefaultReq(AccepterID aID, SessionID sID, ProtocolID pID, ReadStreamPack & rs);
	bool msg_OrgMessageReq(AccepterID aID, SessionID sID, const char * blockBegin, FrameStreamTraits::Integer blockSize);

	typedef std::map<SessionID, std::pair<AccepterID, unsigned int /*index*/> > OnlineServerType;
	OnlineServerType m_onlineAgent;
	OnlineServerType m_onlineLogic;
	tagAcceptorConfigTraits m_configListen; //�����������
	ConnectorID m_lastConnectID = 0; //�Զ�������connectorID.
	std::map<ConnectorID, tagConnctorConfigTraits> m_configDBAgent; //cID ��Ӧ����������
	std::vector<ConnectorID> m_onlineDBAgent; //���ߵ���֤����, ������ϵ ������
};




































#endif
