#pragma once

#include "TcpNetwork.h"
#include "IClientSceen.h"

class ClientChat : public IClientSceen
{
public:
	ClientChat() {};
	virtual ~ClientChat() {};

	void CreateUI(form* from);
	virtual void Update() override 
	{
		if (GetCurSceenType() == CLIENT_SCEEN_TYPE::LOBBY ||
			GetCurSceenType() == CLIENT_SCEEN_TYPE::ROOM)
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

};

void ClientChat::CreateUI(form* pf)
{
	m_pForm = pf;
	m_pChatBox = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(152, 522, 530, 140));
	m_pChatBox->editable(false);

	m_pChatInput = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(152, 661, 530, 27));
	m_pChatInput->enabled(false);
	m_pChatInput->events().key_press([&](const nana::arg_keyboard& event)
	{
		auto key = event.key;
		if (key != '\r')
			return;

		std::string msg;
		m_pChatInput->getline(0, msg);
		
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
			m_pChatBox->append("로비에 입장하셨습니다.\n", false);
		}
		
	}break;
	case(short)PACKET_ID::ROOM_ENTER_RES:
	{

	}
	case (short)PACKET_ID::LOBBY_CHAT_RES:
	{
		m_pChatBox->append("LOBBY ", false);
		m_pChatBox->append(pData, false);
		m_pChatBox->append("\n", false);
	}
	break;
	case (short)PACKET_ID::ROOM_CHAT_RES:
	{
		m_pChatBox->append("ROOM ", false);
		m_pChatBox->append(pData, false);
		m_pChatBox->append("\n", false);

	}
	break;
	case (short)PACKET_ID::LOBBY_WHISPER_RES:
	{
		m_pChatBox->append("WHISHPER ", false);
		m_pChatBox->append(pData, false);
		m_pChatBox->append("\n", false);
	}
	break;
	default:
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
		if (GetCurSceenType() == CLIENT_SCEEN_TYPE::LOBBY)
		{
			//로비 채팅
			NCommon::PktLobbyChatReq reqPkt;
			std::wstring wMsg = std::wstring(msg.begin(), msg.end());
			wcscpy(reqPkt.Msg, wMsg.c_str());

			m_pRefNetwork->SendPacket((short)PACKET_ID::LOBBY_CHAT_REQ, sizeof(reqPkt), (char*)&reqPkt);
		}
		else if (GetCurSceenType() == CLIENT_SCEEN_TYPE::ROOM)
		{
			//룸채팅
			NCommon::PktRoomChatReq reqPkt;
			std::wstring wMsg = std::wstring(msg.begin(), msg.end());
			wcscpy(reqPkt.Msg, wMsg.c_str());

			m_pRefNetwork->SendPacket((short)PACKET_ID::ROOM_CHAT_REQ, sizeof(reqPkt), (char*)&reqPkt);
		}
	}
	return true;
}
