#pragma once

#include "IClientScene.h"

#include <iostream>
#include <chrono>

class ClientSceneLogin : public IClientScene
{
public:
	ClientSceneLogin() {}
	virtual ~ClientSceneLogin() {}

	virtual void Update() override
	{
		if (GetCurSceneType() != CLIENT_SCENE_TYPE::LOGIN) 
		{
			return;
		}

		auto curTime = std::chrono::system_clock::now();

		auto diffTimeSec = std::chrono::duration_cast<std::chrono::seconds>(curTime - m_TimeLastedReqLobbyList);
		
		//3초에 한 번 로비리스트를 요청한다
		if (diffTimeSec.count() > 3)
		{
			m_TimeLastedReqLobbyList = curTime;

			RequestLobbyList();
		}
	}

	void CreateUI(form* pform)
	{
		m_pForm = pform;

		//GUI - Lobby list
		m_list_lobby = std::make_unique<listbox>((form&)*m_pForm, nana::rectangle(22, 106, 165, 383));
		m_list_lobby->append_header("LobbyId", 60);
		m_list_lobby->append_header("Cur", 40);
		m_list_lobby->append_header("Max", 40);

		//GUI - Enther the lobby button
		m_button_enterlobby = std::make_unique<button>((form&)*m_pForm, nana::rectangle(22, 490, 102, 23));
		m_button_enterlobby->caption("Enter Channel");
		m_button_enterlobby->events().click([&]() { this->RequestEnterLobby(); });
	}

	bool ProcessPacket(const short packetId, char* pData) override
	{
		switch (packetId)
		{
		case (short)PACKET_ID::LOBBY_LIST_RES:
		{
			auto pktRes = (NCommon::PktLobbyListRes*)pData;

			if (pktRes->ErrorCode == (short)NCommon::ERROR_CODE::NONE)
			{ //로비리스트를 갱신한다
				m_list_lobby->clear();

				for (int i = 0; i < pktRes->LobbyCount; ++i)
				{
					auto& lobbyListInfo = pktRes->LobbyList[i];

					m_list_lobby->at(0).append({ 
						std::to_string(lobbyListInfo.LobbyId),
						std::to_string(lobbyListInfo.LobbyUserCount), 
						std::to_string(50) 
					});
				}
			}
			else
			{
				std::cout << "[LOBBY_LIST_RES] ErrorCode: " << pktRes->ErrorCode << std::endl;
			}
		}
		break;
		case (short)PACKET_ID::LOGIN_OUT_RES:
		{
			auto pktRes = (NCommon::PktLogOutRes*)pData;

			if (pktRes->ErrorCode == (short)NCommon::ERROR_CODE::NONE)
			{
				m_list_lobby->clear();
			}
			else
			{
				std::cout << "[LOGIN_OUT_RES] ErrorCode: " << pktRes->ErrorCode << std::endl;
			}
		}
		break;
		}

		return true;
	}

private:
	void RequestLobbyList()
	{
		m_pRefNetwork->SendPacket((short)PACKET_ID::LOBBY_LIST_REQ, 0, nullptr);
	}

	void RequestEnterLobby()
	{
		if (GetCurSceneType() != CLIENT_SCENE_TYPE::LOGIN) 
		{
			nana::msgbox m((form&)*m_pForm, "Require LogIn", nana::msgbox::ok);
			m.icon(m.icon_warning).show();
			return;
		}

		auto selectedLobby = m_list_lobby->selected();
		if (selectedLobby.empty()) 
		{
			nana::msgbox m((form&)*m_pForm, "Fail Don't Select Lobby", nana::msgbox::ok);
			m.icon(m.icon_warning).show();
			return;
		}

		auto lobbyIdx = selectedLobby[0].item;
		auto lobbyId = std::atoi(m_list_lobby->at(0).at(lobbyIdx).text(0).c_str());

		NCommon::PktLobbyEnterReq reqPkt;
		reqPkt.LobbyId = lobbyId;
		m_pRefNetwork->SendPacket((short)PACKET_ID::LOBBY_ENTER_REQ, sizeof(reqPkt), (char*)&reqPkt);
	}

private:
	form* m_pForm = nullptr;

	std::unique_ptr<button> m_button_enterlobby;
	std::unique_ptr<listbox> m_list_lobby;

	std::chrono::system_clock::time_point m_TimeLastedReqLobbyList = std::chrono::system_clock::now();
};