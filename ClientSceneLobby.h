#pragma once

#include "IClientScene.h"

#include <list>
#include <string>
#include <vector>
#include <algorithm>

#include <clocale>
//������ �κ񿡼��� �ϵ��� ó���Ѵ�

class ClientSceneLobby : public IClientScene
{
public:
	ClientSceneLobby() { std::setlocale(LC_ALL, ""); }
	virtual ~ClientSceneLobby() {}

	virtual void Update() override 
	{
	}

	bool ProcessPacket(const short packetId, char* pData) override 
	{ 
		switch (packetId)
		{
		//ClientSceneLogin�� EnterLobby���� ��û�� ��Ŷ ó��
		case (short)PACKET_ID::LOBBY_ENTER_RES:
		{
			auto pktRes = (NCommon::PktLobbyEnterRes*)pData;

			if (pktRes->ErrorCode == (short)NCommon::ERROR_CODE::NONE)
			{
				//�κ�&���� ����Ʈ�� �ʱ�ȭ�Ѵ�.
				Init(pktRes->MaxUserCount);

				RequestRoomList(0);

				SetCurSceneType(CLIENT_SCENE_TYPE::LOBBY);
			}
			else
			{
				std::cout << "[LOBBY_ENTER_RES] ErrorCode: " << pktRes->ErrorCode << std::endl;
			}
		}
			break;
		//�κ����� �� ��û�� �渮��Ʈ ��Ŷ ó��
		case (short)PACKET_ID::LOBBY_ENTER_ROOM_LIST_RES:
		{
			auto pktRes = (NCommon::PktLobbyRoomListRes*)pData;

			//�������� �����Ѵ�.
			for (int i = 0; i < pktRes->Count; ++i)
			{
				UpdateRoomInfo(&pktRes->RoomInfo[i]);
			}

			//��� ���ŵ��� �ʾ����� �ٽ� ��û�Ѵ�
			if (pktRes->IsEnd == false)
			{
				RequestRoomList(pktRes->RoomInfo[pktRes->Count - 1].RoomIndex + 1);
			}
			//��� ���ŵǾ�����
			else
			{
				//GUI�� �ݿ��ϰ�
				SetRoomListGUI();
				//���������� ��û�Ѵ�
				RequestUserList(0);
			}
		}
			break;
		//�渮��Ʈ ���� �� ��û�� ��������Ʈ ��Ŷ ó��
		case (short)PACKET_ID::LOBBY_ENTER_USER_LIST_RES:
		{
			auto pktRes = (NCommon::PktLobbyUserListRes*)pData;

			//���������� �����Ѵ�
			for (int i = 0; i < pktRes->Count; ++i)
			{
				UpdateUserInfo(false, pktRes->UserInfo[i].UserID);
			}

			//��� ���ŵ��� �ʾ����� �ٽ� ��û�Ѵ�
			if (pktRes->IsEnd == false)
			{
				RequestUserList(pktRes->UserInfo[pktRes->Count - 1].LobbyUserIndex + 1);
			}
			//��� ���ŵǾ�����
			else
			{
				//GUI�� �ݿ��Ѵ�
				SetUserListGui();
			}
		}
			break;
		case (short)PACKET_ID::ROOM_CHANGED_INFO_NTF:
		{
			auto pktRes = (NCommon::PktChangedRoomInfoNtf*)pData;
			UpdateRoomInfo(&pktRes->RoomInfo);
			SetCurSceneType(CLIENT_SCENE_TYPE::ROOM);
		}
			break;
		case (short)PACKET_ID::ROOM_ENTER_RES:
		{
			auto pktRes = (NCommon::PktRoomEnterRes*)pData;
			UpdateRoomInfo(&pktRes->RoomInfo);
			SetCurSceneType(CLIENT_SCENE_TYPE::ROOM);
		}
			break;
		case (short)PACKET_ID::LOBBY_ENTER_USER_NTF:
		{
			auto pktRes = (NCommon::PktLobbyNewUserInfoNtf*)pData;
			UpdateUserInfo(false, pktRes->UserID);
		}
			break;
		case (short)PACKET_ID::LOBBY_LEAVE_USER_NTF:
		{
			auto pktRes = (NCommon::PktLobbyLeaveUserInfoNtf*)pData;
			UpdateUserInfo(true, pktRes->UserID);
		}
		case (short)PACKET_ID::LOBBY_CHAT_RES:
		{
			auto pktRes = (NCommon::PktLobbyChatRes*)pData;
			if (pktRes->ErrorCode == (short)NCommon::ERROR_CODE::NONE)
			{
				/*char mbChatBuffer[NCommon::MAX_LOBBY_CHAT_MSG_SIZE] = { 0, };
				wcstombs(mbChatBuffer, m_chatBuffer, NCommon::MAX_LOBBY_CHAT_MSG_SIZE);
				std::string mbSzChatBuffer;
				mbSzChatBuffer.append(mbChatBuffer);
				m_tbChatContent->append(mbSzChatBuffer, true);

				m_tbChatInput->reset();*/
			}
			else
			{
				std::cout << "Error Code: " << pktRes->ErrorCode << std::endl;
			}
		}
			break;
		case (short)PACKET_ID::LOBBY_CHAT_NTF:
		{
			auto pktRes = (NCommon::PktLobbyChatNtf*)pData;
			UpdateChatContents(pktRes->UserID, pktRes->Msg);
		}
			break;
		}

		return true;
	}

	void CreateUI(form* pform)
	{
		m_pForm = pform;

		//GUI - Lobby room list
		m_RoomList = std::make_shared<listbox>((form&)*m_pForm, nana::rectangle(204, 106, 345, 383));
		m_RoomList->append_header(L"RoomId", 50);
		m_RoomList->append_header(L"Title", 165);
		m_RoomList->append_header(L"Cur", 30);
		m_RoomList->append_header(L"Max", 30);

		//GUI - Lobby user list
		m_UserList = std::make_shared<listbox>((form&)*m_pForm, nana::rectangle(550, 106, 120, 383));
		m_UserList->append_header("UserID", 90);

		//GUI - Enter the room button
		m_btnEnterRoom = std::make_unique<button>((form&)*m_pForm, nana::rectangle(204, 490, 102, 23));
		m_btnEnterRoom->caption("Enter Room");
		m_btnEnterRoom->events().click([&]() { this->RequestEnterRoom(); });

		//GUI - Room name text box
		m_txtRoomName = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(320, 490, 200, 23));

		//GUI - Create the room button
		m_btnCreateRoom = std::make_unique<button>((form&)*m_pForm, nana::rectangle(525, 490, 102, 23));
		m_btnCreateRoom->caption("Create Room");
		m_btnCreateRoom->events().click([&]() { this->RequestCreateRoom(); });

		////GUI - Chat enter button
		//m_btnChatSend = std::make_unique<button>((form&)*m_pForm, nana::rectangle(525, 520, 102, 23));
		//m_btnChatSend->caption("Send");
		//m_btnChatSend->events().click([&]() { this->RequestChat(); });
		//
		////GUI - Chat text box
		//m_txtChatInput = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(320, 520, 200, 23));

		////GUI - Chat Contents
		//m_labelChatContent = std::make_shared<label>((form&)*m_pForm, nana::rectangle(200, 550, 430, 130));
		//m_labelChatContent->caption("Have a good chattings");
	
		//color labelColor;
		//labelColor.from_rgb(180, 200, 190);
		//m_labelChatContent->bgcolor(labelColor);

	}

	void Init(const int maxUserCount)
	{
		m_MaxUserCount = maxUserCount;

		m_isRoomListInitialized = false;
		m_isUserListInitialized = false;

		m_RoomInfos.clear();
		m_UserInfos.clear();
	}
	
	void UpdateChatContents(char* userId, wchar_t* msg)
	{

	}

	void RequestChat()
	{
		std::string buffer;
		
		/*if (m_tbChatInput->getline(0, buffer) == false)
		{
			return;
		}
		*/
		NCommon::PktLobbyChatReq reqPkt;
		mbstowcs(reqPkt.Msg, buffer.c_str(), NCommon::MAX_LOBBY_CHAT_MSG_SIZE);

		if (reqPkt.Msg == nullptr)
		{
			return;
		}

		//memcpy(m_chatBuffer, reqPkt.Msg, NCommon::MAX_LOBBY_CHAT_MSG_SIZE);

		m_pRefNetwork->SendPacket((short)PACKET_ID::LOBBY_CHAT_REQ, sizeof(reqPkt), (char*)&reqPkt);
	}

	//���� �κ� �ִ� �渮��Ʈ�� ��û�Ѵ�
	void RequestRoomList(const short startIndex)
	{
		NCommon::PktLobbyRoomListReq reqPkt;
		reqPkt.StartRoomIndex = startIndex;
		m_pRefNetwork->SendPacket((short)PACKET_ID::LOBBY_ENTER_ROOM_LIST_REQ, sizeof(reqPkt), (char*)&reqPkt);
	}
	
	//���� �κ� �ִ� ��������Ʈ�� ��û�Ѵ�
	void RequestUserList(const short startIndex)
	{
		NCommon::PktLobbyUserListReq reqPkt;
		reqPkt.StartUserIndex = startIndex;
		m_pRefNetwork->SendPacket((short)PACKET_ID::LOBBY_ENTER_USER_LIST_REQ, sizeof(reqPkt), (char*)&reqPkt);
	}
	
	void RequestEnterRoom()
	{
		auto roomSelected = m_RoomList->selected();
		if (roomSelected.empty())
		{
			nana::msgbox m((form&)*m_pForm, "Please Select Room", nana::msgbox::ok);
			m.icon(m.icon_warning).show();
			return;
		}

		auto index = roomSelected[0].item;
		auto roomIndex = std::atoi(m_RoomList->at(0).at(index).text(0).c_str());

		NCommon::PktRoomEnterReq reqPkt;
		reqPkt.IsCreate = false;
		reqPkt.RoomIndex = roomIndex;
		m_pRefNetwork->SendPacket((short)PACKET_ID::ROOM_ENTER_REQ, sizeof(reqPkt), (char*)&reqPkt);
	}

	void RequestCreateRoom()
	{
		char szRoomName[NCommon::MAX_ROOM_TITLE_SIZE] = { 0, };
		UnicodeToAnsi(m_txtRoomName->caption_wstring().c_str(), NCommon::MAX_ROOM_TITLE_SIZE, szRoomName);

		if (strlen(szRoomName) == 0)
		{
			nana::msgbox m((form&)*m_pForm, "Please Enter Room Name", nana::msgbox::ok);
			m.icon(m.icon_warning).show();
			return;
		}
		
		NCommon::PktRoomEnterReq reqPkt;
		reqPkt.IsCreate = true;
		mbstowcs(reqPkt.RoomTitle, szRoomName, NCommon::MAX_ROOM_TITLE_SIZE);
		m_pRefNetwork->SendPacket((short)PACKET_ID::ROOM_ENTER_REQ, sizeof(reqPkt), (char*)&reqPkt);
	}

	//RoomInfos�� RoomList GUI�� �����Ѵ�.
	void SetRoomListGUI()
	{
		m_isRoomListInitialized = true;

		m_RoomList->clear();

		for (auto & room : m_RoomInfos)
		{
			m_RoomList->at(0).append({ 
				std::to_wstring(room.RoomIndex),
				room.RoomTitle,
				std::to_wstring(room.RoomUserCount),
				std::to_wstring(m_MaxUserCount) 
			});
		}

		m_RoomInfos.clear();
	}

	//UserInfos�� LobbyUserList GUI�� �����Ѵ�.
	void SetUserListGui()
	{
		m_isUserListInitialized = true;

		m_UserList->clear();

		for (auto & userId : m_UserInfos)
		{
			m_UserList->at(0).append({ userId });
		}

		m_UserInfos.clear();
	}

	void UpdateRoomInfo(NCommon::RoomSmallInfo* pRoomInfo)
	{
		NCommon::RoomSmallInfo newRoomInfo;
		memcpy(&newRoomInfo, pRoomInfo, sizeof(NCommon::RoomSmallInfo));
		
		//������ ������ ���� �����Ѵ�
		bool isToRemove = newRoomInfo.RoomUserCount == 0 ? true : false;

		if (m_isRoomListInitialized == false)
		{
			if (isToRemove == false)
			{
				//������ �ִ� ������ �˻��Ѵ�
				auto findIter = std::find_if(std::begin(m_RoomInfos), std::end(m_RoomInfos), 
					[&newRoomInfo](auto& room) { return room.RoomIndex == newRoomInfo.RoomIndex; });

				//������ �ִ� ���̸�
				if (findIter != std::end(m_RoomInfos))
				{
					wcsncpy_s(findIter->RoomTitle, NCommon::MAX_ROOM_TITLE_SIZE + 1, 
						newRoomInfo.RoomTitle, NCommon::MAX_ROOM_TITLE_SIZE);
					
					//�ش� �������� �����Ѵ�
					findIter->RoomUserCount = newRoomInfo.RoomUserCount;
				}
				//������ ���� ���̸�
				else
				{
					//�������� �߰��Ѵ�
					m_RoomInfos.push_back(newRoomInfo);
				}
			}
			else //isToRemove == true
			{
				m_RoomInfos.remove_if([&newRoomInfo](auto& room) 
				{ return room.RoomIndex == newRoomInfo.RoomIndex; });
			}
		}
		//�������� ó������ �ʱ�ȭ�ϴ� ���
		else
		{
			std::string roomIndex(std::to_string(newRoomInfo.RoomIndex));

			if (isToRemove == false)
			{
				for (auto& room : m_RoomList->at(0))
				{
					if (room.text(0) == roomIndex) 
					{
						room.text(1, newRoomInfo.RoomTitle);
						room.text(2, std::to_wstring(newRoomInfo.RoomUserCount));
						return;
					}
				}

				m_RoomList->at(0).append({ 
					std::to_wstring(newRoomInfo.RoomIndex),
					newRoomInfo.RoomTitle,
					std::to_wstring(newRoomInfo.RoomUserCount),
					std::to_wstring(m_MaxUserCount) });
			}
			else //isToRemove == true
			{
				for (auto& room : m_RoomList->at(0))
				{
					if (room.text(0) == roomIndex)
					{
						m_RoomList->erase(room);
						return;
					}
				}
			}
		}
	}

	void UpdateUserInfo(bool isToRemove, std::string userID)
	{		
		if (m_isUserListInitialized == false)
		{
			if (isToRemove == false)
			{
				auto findIter = std::find_if(std::begin(m_UserInfos), std::end(m_UserInfos), [&userID](auto& ID) { return ID == userID; });

				if (findIter == std::end(m_UserInfos))
				{
					m_UserInfos.push_back(userID);
				}
			}
			else
			{
				m_UserInfos.remove_if([&userID](auto& ID) { return ID == userID; });
			}
		}
		else
		{
			if (isToRemove == false)
			{
				for (auto& user : m_UserList->at(0))
				{
					if (user.text(0) == userID) {
						return;
					}
				}

				m_UserList->at(0).append(userID);
			}
			else
			{
				auto i = 0;
				for (auto& user : m_UserList->at(0))
				{
					if (user.text(0) == userID)
					{
						m_UserList->erase(user);
						return;
					}
				}
			}
		}
	}

private:
	//GUI handler
	form* m_pForm = nullptr;

	//list
	std::shared_ptr<listbox> m_RoomList;
	std::shared_ptr<listbox> m_UserList;
	
	//room
	std::unique_ptr<button> m_btnEnterRoom;
	std::unique_ptr<button> m_btnCreateRoom;
	std::shared_ptr<textbox> m_txtRoomName;
	
	//lobby chat
	/*std::shared_ptr<textbox> m_txtChatInput;
	std::unique_ptr<button> m_btnChatSend;
	std::shared_ptr<label> m_labelChatContent;
	wchar_t m_chatBuffer[NCommon::MAX_LOBBY_CHAT_MSG_SIZE] = { 0, };
	*/

	//�� ó�� �κ� �������� �� 
	//������, ��������, GUI�� ����
	//�ʱ�ȭ�� �� �� true�� �ٲ۴�
	bool m_isRoomListInitialized = false;
	bool m_isUserListInitialized = false;
	
	std::list<NCommon::RoomSmallInfo> m_RoomInfos;
	std::list<std::string> m_UserInfos;

	int m_MaxUserCount = 0;
};