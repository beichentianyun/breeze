/*
* zsummerX License
* -----------
* 
* zsummerX is licensed under the terms of the MIT license reproduced below.
* This means that zsummerX is free software and can be used for both academic
* and commercial purposes at absolutely no cost.
* 
* 
* ===============================================================================
* 
* Copyright (C) 2013 YaweiZhang <yawei_zhang@foxmail.com>.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* ===============================================================================
* 
* (end of COPYRIGHT)
*/

#ifndef _ZSUMMER_EPOLL_IMPL_H_
#define _ZSUMMER_EPOLL_IMPL_H_
#include "../common/common.h"
#include "../timer/timer.h"

namespace zsummer
{
	namespace network
	{
		class CZSummerImpl
		{
		public:
			typedef std::vector<void*> MessageStack;
			CZSummerImpl();
			~CZSummerImpl();
			bool Initialize();
			void RunOnce();

			template <typename handle>
			inline void Post(const handle &h)
			{
				PostMessage(h);
			}
			inline unsigned long long CreateTimer(unsigned int delayms, const _OnTimerHandler &handle)
			{
				return m_timer.CreateTimer(delayms, handle);
			}
			inline bool CancelTimer(unsigned long long timerID)
			{
				return m_timer.CancelTimer(timerID);
			}

		public:
			bool RegisterEvent(int op, tagRegister &reg);
			void PostMessage(const _OnPostHandler &handle);
		private:
			std::string GetZSummerImplStatus();
		private:
			int	m_epoll = InvalideFD;
			//! ������Ϣ
			epoll_event m_events[5000] = {};
			//�߳���Ϣ
			int		m_sockpair[2] = {};
			tagRegister m_register;
			MessageStack m_stackMessages;
			std::mutex	 m_stackMessagesLock;

			//! timmer
			CTimer m_timer;
		};
	}

}





















#endif











