

/*
* breeze License
* Copyright (C) 2014 YaweiZhang <yawei_zhang@foxmail.com>.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http ://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef _DEFINE_PROTO_H_
#define _DEFINE_PROTO_H_
#include <string>

typedef char i8;
typedef unsigned char ui8;
typedef short i16;
typedef unsigned short ui16;
typedef int i32;
typedef unsigned int ui32;
typedef long long i64;
typedef unsigned long long ui64;


// �������ڲ�������ͨѶЭ������Ϊ[)
const ui16 MIN_SERVER_CONTROL_PROTO_ID = 0;
const ui16 MAX_SERVER_CONTROL_PROTO_ID = 1000;

// �������ڲ��߼���ͨѶЭ������Ϊ[)
const ui16 MIN_SERVER_LOGIC_PROTO_ID = 1000;
const ui16 MAX_SERVER_LOGIC_PROTO_ID = 20000;

//����֤����¿���ͨѶЭ��[)
const ui16 MIN_C_R_AUTH_PROTO_ID = 20000;
const ui16 MAX_C_R_AUTH_PROTO_ID = 20100;

// CLIENT ��agent server��ͨѶЭ������Ϊ[)
const ui16 MIN_C_R_AS_PROTO_ID = 20100;
const ui16 MAX_C_R_AS_PROTO_ID = 21000;

// CLIENT ��logic server��ͨѶЭ������Ϊ[)
const ui16 MIN_C_R_LS_PROTO_ID = 21000;
const ui16 MAX_C_R_LS_PROTO_ID = 40000;

// CLIENT ��dbagent server��ͨѶЭ������Ϊ[)
const ui16 MIN_C_R_DS_PROTO_ID = 42000;
const ui16 MAX_C_R_DS_PROTO_ID = 45000;

// CLIENT ��center server��ͨѶЭ������Ϊ[)
const ui16 MIN_C_R_CS_PROTO_ID = 45000;
const ui16 MAX_C_R_CS_PROTO_ID = 48000;

// CLIENT ��battle server��ͨѶЭ������Ϊ[)
const ui16 MIN_C_R_BS_PROTO_ID = 48000;
const ui16 MAX_C_R_BS_PROTO_ID = 50000;

// CLIENT ��platform��ͨѶЭ������Ϊ[)
const ui16 MIN_C_R_PS_PROTO_ID = 50000;
const ui16 MAX_C_R_PS_PROTO_ID = 51000;

//CLIENT
const ui16 MIN_C_R_RESERVE_PROTO_ID = 51000;
const ui16 MAX_C_R_RESERVE_PROTO_ID = 65535;



//�ͻ��˵�����Э����Ը������º����ж�
inline bool isClientPROTO(ui16 protoID) { return protoID >= MIN_C_R_AUTH_PROTO_ID && protoID < MIN_C_R_RESERVE_PROTO_ID; }
inline bool isNeedAuthClientPROTO(ui16 protoID) { return protoID >= MAX_C_R_AUTH_PROTO_ID && protoID < MIN_C_R_RESERVE_PROTO_ID; }
inline bool isRouteToAgent(ui16 protoID){ return protoID >= MIN_C_R_AS_PROTO_ID && protoID < MAX_C_R_AS_PROTO_ID; }
inline bool isRouteToCenter(ui16 protoID){ return protoID >= MIN_C_R_CS_PROTO_ID && protoID < MAX_C_R_CS_PROTO_ID; }
inline bool isRouteToLogic(ui16 protoID) { return protoID >= MIN_C_R_LS_PROTO_ID && protoID < MAX_C_R_LS_PROTO_ID; }
inline bool isRouteToBattle(ui16 protoID) { return protoID >= MIN_C_R_BS_PROTO_ID && protoID < MAX_C_R_BS_PROTO_ID; }



//����������ڲ�����Э��
//��������
const ui16 PROTO_CONTROL_PULSE = 0;

//�ڲ�����·�ɵ�C
const ui16 PROTO_SERVER_ROUTE_CLIENT = 10;

//�ڲ�����·�ɵ��ƶ�����
const ui16 PROTO_SERVER_ROUTE_AGENT = 20;
const ui16 PROTO_SERVER_ROUTE_CENTER = 21;
const ui16 PROTO_SERVER_ROUTE_GAME = 22;
const ui16 PROTO_SERVER_ROUTE_DBAGENT = 23;
const ui16 PROTO_SERVER_ROUTE_BATTLE = 24;





//������
enum ErrorCode
{
	EC_SUCCESS = 0,
	EC_UNKOWN_ERROR,
	EC_SERVER_ERROR,
	EC_NOT_FOUND_ACCCOUNT_NAME,
	EC_AUTH_ERROR,
	EC_AREADY_AUTH,
};





















#endif
