#pragma once

#include "IClientScene.h"

//PacketProcess에서 Logout 부분 서버에서 아직 구현 안 함

class ClientScene : public IClientScene
{
public:
	ClientScene() {}
	virtual ~ClientScene() {}

	virtual void Update() override
	{
		if (GetCurSceenType() != CLIENT_SCEEN_TYPE::CONNECT) 
		{
			return;
		}
	}

	void CreateUI(form* pform)
	{
		m_pForm = pform;

		//GUI - IP
		m_label_ip = std::make_shared<label>((form&)*m_pForm, nana::rectangle(22, 17, 18, 18));
		m_label_ip->caption("IP:");
		m_text_ip = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(43, 15, 128, 20));
		m_text_ip->caption("127.0.0.1");

		//GUI - Port
		m_label_port = std::make_shared<label>((form&)*m_pForm, nana::rectangle(187, 17, 30, 18));
		m_label_port->caption("Port:");
		m_text_port = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(214, 15, 60, 20));
		m_text_port->caption("23452");

		//GUI - Connect
		m_button_connect = std::make_shared<button>((form&)*m_pForm, nana::rectangle(283, 14, 102, 23));
		m_button_connect->caption("Connent");
		m_button_connect->events().click([&]() { this->ConnectOrDisConnect(); });

		m_text_connect = std::make_unique<textbox>((form&)*m_pForm, nana::rectangle(450, 15, 120, 20));
		m_text_connect->caption("Disconnected");

		//GUI - ID
		m_label_id = std::make_shared<label>((form&)*m_pForm, nana::rectangle(22, 58, 18, 18));
		m_label_id->caption("ID:");
		m_text_id = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(43, 56, 115, 20));
		m_text_id->caption("jacking");

		//GUI - PW
		m_label_pw = std::make_shared<label>((form&)*m_pForm, nana::rectangle(170, 58, 69, 18));
		m_label_pw->caption("PassWord:");
		m_text_pw = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(230, 56, 115, 20));
		m_text_pw->caption("1234");

		//GUI - Login
		m_button_login = std::make_shared<button>((form&)*m_pForm, nana::rectangle(353, 54, 102, 23));
		m_button_login->caption("Login");
		m_button_login->events().click([&]() { this->LogInOut(); });
		m_button_login->enabled(false);
	}

	bool ProcessPacket(const short packetId, char* pData) override
	{
		switch (packetId)
		{
		case (short)PACKET_ID::LOGIN_IN_RES:
		{
			m_button_login->enabled(true);

			auto pktRes = (NCommon::PktLogInRes*)pData;

			if (pktRes->ErrorCode == (short)NCommon::ERROR_CODE::NONE)
			{
				m_button_login->caption("LogOut");
				m_IsLogined = true;
				SetCurSceenType(CLIENT_SCEEN_TYPE::LOGIN);
			}
			else
			{
				nana::msgbox m((form&)*m_pForm, "Fail LOGIN_IN_REQ", nana::msgbox::ok);
				m.icon(m.icon_warning);
				m << "ErrorCode: " << pktRes->ErrorCode;
				m.show();
			}
		}
			break;
		case (short)PACKET_ID::LOGIN_OUT_RES: //서버에서 아직 구현 안 함
		{
			m_button_login->enabled(true);

			auto pktRes = (NCommon::PktLogOutRes*)pData;

			if (pktRes->ErrorCode == (short)NCommon::ERROR_CODE::NONE)
			{
				m_button_login->caption("Login");
				m_IsLogined = false;
				SetCurSceenType(CLIENT_SCEEN_TYPE::CONNECT);
			}
			else
			{
				nana::msgbox m((form&)*m_pForm, "Fail LOGIN_OUT_REQ", nana::msgbox::ok);
				m.icon(m.icon_warning);
				m << "ErrorCode: " << pktRes->ErrorCode;
				m.show();
			}
		}
		break;
		}
		
		return true;
	}

private:
	//connect button callback
	void ConnectOrDisConnect()
	{
		//서버와 연결되어 있지 않으면
		if (m_pRefNetwork->IsConnected() == false)
		{ 
			char IP[64] = { 0, };
			UnicodeToAnsi(m_text_ip->caption_wstring().c_str(), sizeof(IP), IP);

			//연결 요청을 한다
			if (m_pRefNetwork->ConnectTo(IP, (unsigned short)m_text_port->to_int()))
			{//연결 성공
				m_button_connect->caption("DisConnect");
				m_text_connect->caption("state: Connected");
				m_button_login->enabled(true);
			}
			else
			{//연결 실패
				nana::msgbox m((form&)*m_pForm, "Fail Connect", nana::msgbox::ok);
				m.icon(m.icon_warning);
				m.show();
			}
		}
		//서버와 연결되어 있으면
		else 
		{
			//연결을 해제한다
			m_pRefNetwork->DisConnect();
			
			m_button_connect->caption("Connect");
			m_text_connect->caption("state: Disconnected");

			//로그인되어 있으면
			if (m_IsLogined)
			{
				//로그아웃 요청한다.
				NCommon::PktLogOutReq reqPkt;
				m_pRefNetwork->SendPacket((short)PACKET_ID::LOGIN_OUT_REQ, sizeof(reqPkt), (char*)&reqPkt);
			}
		}
	}

	//login button callback
	void LogInOut()
	{
		//로그인되어 있지 않으면
		if (m_IsLogined == false)
		{ //로그인 요청한다
			char szID[64] = { 0, };
			UnicodeToAnsi(m_text_id->caption_wstring().c_str(), 64, szID);

			char szPW[64] = { 0, };
			UnicodeToAnsi(m_text_pw->caption_wstring().c_str(), 64, szPW);

			NCommon::PktLogInReq reqPkt;
			strncpy_s(reqPkt.szID, NCommon::MAX_USER_ID_SIZE + 1, szID, NCommon::MAX_USER_ID_SIZE);
			strncpy_s(reqPkt.szPW, NCommon::MAX_USER_PASSWORD_SIZE + 1, szPW, NCommon::MAX_USER_PASSWORD_SIZE);

			m_pRefNetwork->SendPacket((short)PACKET_ID::LOGIN_IN_REQ, sizeof(reqPkt), (char*)&reqPkt);
		}
		//로그인되어 있으면
		else
		{ //로그아웃 요청한다
			NCommon::PktLogOutReq reqPkt;
			m_pRefNetwork->SendPacket((short)PACKET_ID::LOGIN_OUT_REQ, sizeof(reqPkt), (char*)&reqPkt);
		}
	}

	bool m_IsLogined = false;

	form* m_pForm = nullptr;

	std::shared_ptr<label> m_label_ip;
	std::shared_ptr<textbox> m_text_ip;

	std::shared_ptr<label> m_label_port;
	std::shared_ptr<textbox> m_text_port;

	std::shared_ptr<button> m_button_connect;
	std::unique_ptr<textbox> m_text_connect;
	
	std::shared_ptr<label> m_label_id;
	std::shared_ptr<textbox> m_text_id;

	std::shared_ptr<label> m_label_pw;
	std::shared_ptr<textbox> m_text_pw;

	std::shared_ptr<button> m_button_login;
};