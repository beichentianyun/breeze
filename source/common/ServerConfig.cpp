﻿

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

#include "tinyxml2.h"
#include "ServerConfig.h"





const ListenConfig ServerConfig::getConfigListen(ServerNode node, NodeIndex index)
{
	if (index == InvalidNodeIndex)
	{
		index = m_ownNodeIndex;
	}
	auto founder = std::find_if(m_configListen.begin(), m_configListen.end(),
		[node, index](const ListenConfig & lc){return lc.node == node && lc.index == index; });
	if (founder == m_configListen.end())
	{
		static ListenConfig lc;
		return lc;
	}
	return *founder;
}


std::vector<ConnectorConfig > ServerConfig::getConfigConnect(ServerNode node)
{
	std::vector<ConnectorConfig > ret;
	for (const auto & cc : m_configConnect)
	{
		if (cc.srcNode != node)
		{
			continue;
		}
		ret.push_back(cc);
	}
	
	return ret;
}






static ServerNode toServerNode(std::string strNode)
{
	if (strNode == "agent")
	{
		return AgentNode;
	}
	else if (strNode == "center")
	{
		return CenterNode;
	}
	else if (strNode == "auth")
	{
		return AuthNode;
	}
	else if (strNode == "logic")
	{
		return LogicNode;
	}
	return InvalideServerNode;
}

bool ServerConfig::Parse(std::string filename, ServerNode ownNode, NodeIndex ownIndex)
{
	m_ownServerNode = ownNode;
	m_ownNodeIndex = ownIndex;
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS)
	{
		LOGE(filename << " Parse ServerConfig Error. ");
		doc.PrintError();
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	auto elmTraits = doc.FirstChildElement("traits");
	if (!elmTraits || ! elmTraits->Attribute("platid") || ! elmTraits->Attribute("areaid"))
	{
		LOGE(filename << " Parse ServerConfig Error. not have traits");
		return false;
	}
	m_platid = elmTraits->IntAttribute("platid");
	m_areaid = elmTraits->IntAttribute("areaid");

	//////////////////////////////////////////////////////////////////////////
	{
		auto elmListen = doc.FirstChildElement("listen");
		if (!elmListen || !elmListen->FirstChildElement())
		{
			LOGE(filename << " Parse ServerConfig Error. not have listen");
			return false;
		}
		auto elmListenChild = elmListen->FirstChildElement();
		do
		{
			std::string strNode = elmListenChild->Name();
			if (!elmListenChild->Attribute("ip") || !elmListenChild->Attribute("port") || !elmListenChild->Attribute("index"))
			{
				LOGE(filename << " Parse ServerConfig Error. listen have invalide config.");
				return false;
			}

			ListenConfig lconfig;
			lconfig.ip = elmListenChild->Attribute("ip");
			lconfig.port = elmListenChild->IntAttribute("port");
			lconfig.index = elmListenChild->IntAttribute("index");
			lconfig.node = toServerNode(strNode);
			m_configListen.push_back(lconfig);
			LOGD("strNode=" << strNode << ", ip=" << lconfig.ip << ", port=" << lconfig.port << ", lconfig.index=" << lconfig.index);
			elmListenChild = elmListenChild->NextSiblingElement();
		} while (elmListenChild);
	}


	//////////////////////////////////////////////////////////////////////////
	{
		auto elmConnect = doc.FirstChildElement("connect");
		if (!elmConnect || !elmConnect->FirstChildElement())
		{
			LOGE(filename << " Parse ServerConfig Error. not have connect");
			return false;
		}
		auto elmConnectChild = elmConnect->FirstChildElement();
		do
		{
			std::string srcStrNode = elmConnectChild->Name();
			if (!elmConnectChild->Attribute("ip")
				|| !elmConnectChild->Attribute("port")
				|| !elmConnectChild->Attribute("dstNode"))
			{
				LOGE(filename << " Parse ServerConfig Error. connect have invalide config.");
				return false;
			}


			ConnectorConfig lconfig;
			std::string dstStrNode = elmConnectChild->Attribute("dstNode");
			lconfig.remoteIP = elmConnectChild->Attribute("ip");
			lconfig.remotePort = elmConnectChild->IntAttribute("port");
			lconfig.srcNode = toServerNode(srcStrNode);
			lconfig.dstNode = toServerNode(dstStrNode);
			m_configConnect.push_back(lconfig);
			LOGD("srcStrNode=" << srcStrNode << ", remoteIP=" << lconfig.remoteIP << ", remotePort=" << lconfig.remotePort << ", dstStrNode=" << dstStrNode);
			elmConnectChild = elmConnectChild->NextSiblingElement();
		} while (elmConnectChild);
	}

	//////////////////////////////////////////////////////////////////////////
	{
		auto elmMongo = doc.FirstChildElement("mongo");
		if (!elmMongo || !elmMongo->FirstChildElement())
		{
			LOGE(filename << " Parse ServerConfig Error. not have mongo");
			return false;
		}
		auto elmMongoChild = elmMongo->FirstChildElement();
		do
		{
			std::string strNode = elmMongoChild->Name();
			if (!elmMongoChild->Attribute("ip")
				|| !elmMongoChild->Attribute("port")
				|| !elmMongoChild->Attribute("db")
				|| !elmMongoChild->Attribute("user")
				|| !elmMongoChild->Attribute("pwd")
				|| !elmMongoChild->Attribute("needAuth"))
			{
				LOGE(filename << " Parse ServerConfig Error. mongo have invalide config.");
				return false;
			}

			MongoConfig lconfig;
			lconfig.ip = elmMongoChild->Attribute("ip");
			lconfig.port = elmMongoChild->IntAttribute("port");
			lconfig.db = elmMongoChild->Attribute("db");
			lconfig.user = elmMongoChild->Attribute("user");
			lconfig.pwd = elmMongoChild->Attribute("pwd");
			lconfig.needAuth = elmMongoChild->BoolAttribute("needAuth");
			if (strNode == AuthMongoDB)
			{
				m_authMongo = lconfig;
			}
			else if (strNode == InfoMongoDB)
			{
				m_infoMongo = lconfig;
			}
			else if (strNode == LogMongoDB)
			{
				m_logMongo = lconfig;
			}
			elmMongoChild = elmMongoChild->NextSiblingElement();
		} while (elmMongoChild);
	}
	return true;
}


























