#pragma once

#include "TcpNetwork.h"
#include "IClientScene.h"

class ClientChat : public IClientScene
{
public:
	ClientChat() { std::setlocale(LC_ALL, "utf-8"); };
	virtual ~ClientChat() {};

	void CreateUI(form* from);
	virtual void Update() override 
	{
		if (GetCurSceneType() == CLIENT_SCENE_TYPE::LOBBY ||
			GetCurSceneType() == CLIENT_SCENE_TYPE::ROOM)
		{
			m_pChatInput->enabled(true);
		}
		else
			m_pChatInput->enabled(false);
	};

	virtual bool ProcessPacket(const short packetId, char* pData) override;

	
private:

	bool ProcessMsg(std::string& msg);
	//void SendLobbyMessage();
	//void SendRoomMessage();
	//void SendWishper();

	form* m_pForm = nullptr;
	std::shared_ptr<textbox> m_pChatBox;
	std::shared_ptr<textbox> m_pChatInput;
	char m_myMsgBuffer[NCommon::MAX_LOBBY_CHAT_MSG_SIZE];
};

void ClientChat::CreateUI(form* pf)
{
	m_pForm = pf;
	m_pChatBox = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(152, 522, 530, 140));
	m_pChatBox->editable(false);
	m_pChatBox->caption(L"즐거운 채팅창.\n");

	m_pChatInput = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(152, 661, 530, 27));
	m_pChatInput->enabled(false);
	m_pChatInput->events().key_press([&](const nana::arg_keyboard& event)
	{
		auto key = event.key;
		if (key != '\r')
			return;

		std::string msg;
		std::string tmp;
		
		int i = 0;
		while (m_pChatInput->getline(i, tmp))
		{
			msg += tmp;
			i++;
		}

		msg.erase(msg.find_last_not_of("\n\r\t") + 1);
		
		this->ProcessMsg(msg);

		m_pChatInput->reset();
		
	});
}

bool ClientChat::ProcessPacket(const short packetId, char * pData)
{
	switch (packetId)
	{
	case (short)PACKET_ID::LOBBY_ENTER_RES:
	{
		NCommon::PktLobbyEnterRes* pktData = (NCommon::PktLobbyEnterRes*)pData;
		if (pktData->ErrorCode == (short)NCommon::ERROR_CODE::NONE)
		{
			m_pChatBox->append("Enter Lobby.\n", true);
		}
	}
		break;
	case (short)PACKET_ID::ROOM_ENTER_RES:
	{
		NCommon::PktRoomEnterRes* pktData = (NCommon::PktRoomEnterRes*)pData;
		if (pktData->ErrorCode == (short)NCommon::ERROR_CODE::NONE)
		{
			m_pChatBox->append("Enter Room.\n", true);
		}
	}
		break;
	case (short)PACKET_ID::LOBBY_CHAT_RES:
	{
		std::cout << "Lobby Chat Res" << std::endl;

		NCommon::PktLobbyChatRes* pktData = (NCommon::PktLobbyChatRes*)pData;

		if ((short)NCommon::ERROR_CODE::NONE == pktData->ErrorCode)
		{
//			m_pChatInput->reset();
			m_pChatBox->append("me: ", true);
			m_pChatBox->append(m_myMsgBuffer, true);	
			m_pChatBox->append("\n", true);
		}
		else
		{
			std::cout << "Lobby Chat Err" << std::endl;
		}
	}
		break;

	case (short)PACKET_ID::LOBBY_CHAT_NTF:
	{
		std::cout << "Lobby Chat Notify" << std::endl;

		NCommon::PktLobbyChatNtf* pktData = (NCommon::PktLobbyChatNtf*)pData;
		
		char id[NCommon::MAX_USER_ID_SIZE + 1] = { '\0', };
		memcpy(id, pktData->UserID, NCommon::MAX_USER_ID_SIZE);

		char sentence[NCommon::MAX_LOBBY_CHAT_MSG_SIZE + 1] = { '\0', };
		memcpy(sentence, pktData->Msg, sizeof(sentence));

//		UnicodeToAnsi(pktData->Msg, sizeof(sentence), sentence);

		m_pChatBox->append(id, true);
		m_pChatBox->append(": ", true);
		m_pChatBox->append(sentence, true);
		m_pChatBox->append("\n", true);
	}
		break;

	case (short)PACKET_ID::ROOM_CHAT_RES:
	{
		std::cout << "Robby Chat Res" << std::endl;

		NCommon::PktRoomChatRes* pktData = (NCommon::PktRoomChatRes*)pData;

		if ((short)NCommon::ERROR_CODE::NONE == pktData->ErrorCode)
		{
			//			m_pChatInput->reset();
			m_pChatBox->append("me: ", true);
			m_pChatBox->append(m_myMsgBuffer, true);
			m_pChatBox->append("\n", true);
		}
		else
		{
			std::cout << "Room Chat Err" << std::endl;
		}
	}
		break;

	case (short)PACKET_ID::ROOM_CHAT_NTF:
	{
		std::cout << "Room Chat Notify" << std::endl;

		NCommon::PktRoomChatNtf* pktData = (NCommon::PktRoomChatNtf*)pData;

		char id[NCommon::MAX_USER_ID_SIZE + 1] = { '\0', };
		memcpy(id, pktData->UserID, NCommon::MAX_USER_ID_SIZE);

		char sentence[NCommon::MAX_ROOM_CHAT_MSG_SIZE + 1] = { '\0', };
		memcpy(sentence, pktData->Msg, sizeof(sentence));

		//		UnicodeToAnsi(pktData->Msg, sizeof(sentence), sentence);

		m_pChatBox->append(id, true);
		m_pChatBox->append(": ", true);
		m_pChatBox->append(sentence, true);
		m_pChatBox->append("\n", true);
	}
	break;

	case (short)PACKET_ID::LOBBY_WHISPER_RES:
	{
		m_pChatBox->append("WHISHPER ", false);
		m_pChatBox->append(pData, false);
		m_pChatBox->append("\n", false);
	}
		break;
	}

	return false;
}

bool ClientChat::ProcessMsg(std::string& msg)
{
	if (msg[0] == '/')
	{
		//TODO : 슬래시 달려있을때 문자열 처리.. (귓속말 등등)
	}
	else
	{
		if (CLIENT_SCENE_TYPE::LOBBY == GetCurSceneType())
		{
			//자기가 쓴 메시지 버퍼에 저장 -> 성공 시 대화창에 출력
			int msgSize = min(msg.size(), sizeof(m_myMsgBuffer));
			
			for (int i = 0; i < msgSize; i++)
				m_myMsgBuffer[i] = msg.at(i);

			m_myMsgBuffer[msgSize] = '\0';

			std::cout << m_myMsgBuffer << std::endl;

			//textbox->append(...) 내부에서 string을 wstring으로 변환한다
			//multibyte -> wide 
			NCommon::PktLobbyChatReq reqPkt;

			memcpy(reqPkt.Msg, m_myMsgBuffer, msgSize);

//			std::wstring wMsg = std::wstring(msg.begin(), msg.end());
//			std::wstring wMsg = to_wstring(msg);
//			std::wcout << wMsg << std::endl;
//			std::cout << msg << std::endl;

//			mbstowcs(reqPkt.Msg, m_myMsgBuffer, sizeof(m_myMsgBuffer));
			
//			memcpy_s(reqPkt.Msg, sizeof(reqPkt.Msg), wMsg.c_str(), sizeof(wMsg));


			m_pRefNetwork->SendPacket((short)PACKET_ID::LOBBY_CHAT_REQ, sizeof(reqPkt), (char*)&reqPkt);
		}
		else if (GetCurSceneType() == CLIENT_SCENE_TYPE::ROOM)
		{
			//룸채팅
			int msgSize = min(msg.size(), sizeof(m_myMsgBuffer));

			for (int i = 0; i < msgSize; i++)
				m_myMsgBuffer[i] = msg.at(i);

			m_myMsgBuffer[msgSize] = '\0';

			std::cout << m_myMsgBuffer << std::endl;

			NCommon::PktRoomChatReq reqPkt;

			memcpy(reqPkt.Msg, m_myMsgBuffer, msgSize);

			m_pRefNetwork->SendPacket((short)PACKET_ID::ROOM_CHAT_REQ, sizeof(reqPkt), (char*)&reqPkt);
		}
	}

	return true;
}
