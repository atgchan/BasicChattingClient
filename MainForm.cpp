#include "TcpNetwork.h"
#include "MainForm.h"
#include "ClientSceneMain.h"
#include "ClientSceneLogin.h"
#include "ClientSceneLobby.h"
#include "ClientSceneRoom.h"
#include "ClientChat.h"

using PACKET_ID = NCommon::PACKET_ID;

MainForm::MainForm() {}

MainForm::~MainForm()
{
	if (m_Network)
	{
		m_Network->DisConnect();
	}
}

void MainForm::Init()
{
	m_Network = std::make_unique<TcpNetwork>();

	m_pClientScene = std::make_shared<ClientSceneMain>();
	m_pClientScene->SetNetwork(m_Network.get());

	m_pClientSceneLogin = std::make_shared<ClientSceneLogin>();
	m_pClientSceneLogin->SetNetwork(m_Network.get());

	m_pClientSceneLobby = std::make_shared<ClientSceneLobby>();
	m_pClientSceneLobby->SetNetwork(m_Network.get());

	m_pClientChat = std::make_shared<ClientChat>();
	m_pClientChat->SetNetwork(m_Network.get());

	m_pClientSceneRoom = std::make_shared<ClientSceneRoom>();
	m_pClientSceneRoom->SetNetwork(m_Network.get());
}

void MainForm::CreateGUI()
{
	// https://moqups.com/   ���⿡�� ������ ����

	m_fm = std::make_unique<form>(API::make_center(900, 700));
	m_fm->caption("Chat Client");

	//connect & login
	m_pClientScene->CreateUI(m_fm.get());

	//get lobby list & enter lobby
	m_pClientSceneLogin->CreateUI(m_fm.get());

	m_pClientChat->CreateUI(m_fm.get());

	m_RoomUserList = std::make_shared<listbox>((form&)*m_fm.get(), nana::rectangle(22, 522, 120, 166));
	m_RoomUserList->append_header("UserID", 90);

	//
	m_pClientSceneLobby->CreateUI(m_fm.get());

	//
	m_pClientSceneRoom->CreateUI(m_fm.get());

	//
	m_timer.elapse([&]() { PacketProcess();});
	m_timer.interval(32);
	m_timer.start();
}

void MainForm::ShowModal()
{
	m_fm->show();

	exec();
}

void MainForm::PacketProcess()
{
	if (!m_Network)
	{
		return;
	}
	auto packet = m_Network->GetPacket();

	if (packet.PacketId != 0)
	{
		m_pClientScene->ProcessPacket(packet.PacketId, packet.pData);
		m_pClientSceneLogin->ProcessPacket(packet.PacketId, packet.pData);
		m_pClientSceneLobby->ProcessPacket(packet.PacketId, packet.pData);
		m_pClientSceneRoom->ProcessPacket(packet.PacketId, packet.pData);
		m_pClientChat->ProcessPacket(packet.PacketId, packet.pData);

		if (packet.pData != nullptr)
		{
			delete[] packet.pData;
		}
	}

	m_pClientScene->Update();
	m_pClientSceneLogin->Update();
	m_pClientSceneLobby->Update();
	m_pClientSceneRoom->Update();
	m_pClientChat->Update();

}
