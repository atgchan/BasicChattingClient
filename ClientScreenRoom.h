#pragma once

#include <list>
#include <string>
#include <vector>
#include <algorithm>

#include "IClientSceen.h"



class ClientSceenRoom : public IClientSceen
{
public:
	ClientSceenRoom() {}
	virtual ~ClientSceenRoom() {}

	virtual void Update() override
	{
	}

	bool ProcessPacket(const short packetId, char* pData) override
	{
		switch (packetId)
		{
		case (short)PACKET_ID::LOBBY_ENTER_RES:
		{
		}
		break;

		default:
			return false;
		}

		return true;
	}

	void CreateUI(form* pform)
	{
		m_pForm = pform;
	}

	void Init(const int maxUserCount)
	{
		m_MaxUserCount = maxUserCount;
	}


private:
	form* m_pForm = nullptr;

	int m_MaxUserCount = 0;

	bool m_IsUserListWorking = false;
	std::list<std::string> m_UserList;
};
