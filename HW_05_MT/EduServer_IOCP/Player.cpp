﻿#include "stdafx.h"
#include "Timer.h"
#include "ThreadLocal.h"
#include "ClientSession.h"
#include "Player.h"
#include "PlayerManager.h"
#include "PlayerWideEvent.h"
#include "GrandCentralExecuter.h"

Player::Player(ClientSession* session) : mSession(session)
{
	PlayerReset();
}

Player::~Player()
{
	CRASH_ASSERT(false);
}

void Player::PlayerReset()
{
	/// 플레이어 맵에서 제거
	GPlayerManager->UnregisterPlayer(mPlayerId);

	mPlayerId = -1;
	mHeartBeat = -1;
	mIsAlive = false;
}

void Player::Start(int heartbeat)
{
	mIsAlive = true;
	mHeartBeat = heartbeat;
	
	/// ID 발급 및 플레잉어 맵에 등록
	mPlayerId = GPlayerManager->RegisterPlayer(GetSharedFromThis<Player>());

	/// 생명 불어넣기 ㄱㄱ
	OnTick();
}

void Player::OnTick()
{
	if (!IsAlive())
		return;

	/// 랜덤으로 이벤트를 발생시켜보기 (예: 다른 모든 플레이어에게 버프 주기)
	if (rand() % 100 == 0) ///< 1% 확률
	{
		int buffId = mPlayerId * 100;
		int duration = (rand() % 3 + 2) * 1000;
	
		//GCE 예. (lock-order 귀찮고, 전역적으로 순서 보장 필요할 때)
		//make_shared는 생성자에서 요구하는 파라미터 값 주고 shared_ptr 얻음
		//http://gcstudy.tistory.com/56
		auto playerEvent = std::make_shared<AllPlayerBuffEvent>(buffId, duration);
		GCEDispatch(playerEvent, &AllPlayerBuffEvent::DoBuffToAllPlayers, mPlayerId);
	}


	//TODO: AllPlayerBuffDecay::CheckBuffTimeout를 GrandCentralExecuter를 통해 실행
	//timeout check는 매 틱마다 하니깐 조건 없이도 괜찮아
	auto playerBuffDecayEvent = std::make_shared<AllPlayerBuffDecay>();
	GCEDispatch( playerBuffDecayEvent, &AllPlayerBuffDecay::CheckBuffTimeout );

	
	//요게 1초에 한 번씩 불러지게 되어 있는 녀석이구만!
	if (mHeartBeat > 0)
		DoSyncAfter(mHeartBeat, GetSharedFromThis<Player>(), &Player::OnTick);
}

void Player::AddBuff(int fromPlayerId, int buffId, int duration)
{
	printf_s("I am Buffed [%d]! from Player [%d]\n", buffId, fromPlayerId);

	/// 플레이어의 버프 리스트에 추가
	mBuffList.insert(std::make_pair(buffId, duration));
}

void Player::DecayTickBuff()
{
	/// 버프의 남은 시간을 주기적으로 수정하고, 시간이 종료되면 제거하기
	for (auto it = mBuffList.begin(); it != mBuffList.end();)
	{
		//mHeartBeat는 1000으로 고정되어 있다옹
		//그래서 매번 여기 지날 땐 1초씩 감소하는 것이지...
		it->second -= mHeartBeat;

		if (it->second <= 0)
		{
			printf_s("Player [%d] BUFF [%d] expired\n", mPlayerId, it->first);
			mBuffList.erase(it++);
		}
		else
		{
			++it;
		}
	}
}