#pragma once

#include "Define.h"
#include "PacketManager.h"
// 파티 채팅 쓰레드, 친구쪽 쓰레드, 파티쪽 쓰레드, 귓속말 쓰레드

/*로그인 하면 서버에서 부여받는 번호
	unsigned int login_number;*/

std::string realid;

SOCKET soc;

USER_STATE UserState = USER_STATE::NONE;

int friendCnt = 0;

void clean() {
	shutdown(soc, SD_BOTH);
	closesocket(soc);
	WSACleanup();
}

int main() {

	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8081);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

	std::cout << "서버 연결중" << std::endl;

	std::atexit(clean);

	while (1) {
		if (!connect(soc, (SOCKADDR*)&addr, sizeof(addr))) {
			std::cout << "서버와 연결완료" << std::endl << std::endl;
			break;
		}
	}

	/* GUID guid = WSAID_CONNECTEX;
	 LPFN_CONNECTEX g_connect;
	 DWORD cbBytes{0};

	 WSAIoctl(soc, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &g_connect, sizeof(g_connect), &cbBytes, NULL, NULL);

	 OVERLAPPED over;

	 std::cout << "서버 연결 대기중..." << std::endl << std::endl;

	 if (g_connect(soc, (const sockaddr*)&addr, sizeof(addr), NULL, NULL, NULL, (LPOVERLAPPED)&over) == (SOCKET_ERROR && WSA_IO_PENDING) != WSAGetLastError())
	 {

		 std::cout << "진행중" << std::endl;
	 }
		 else std::cout << "성공" << std::endl;*/


	while (1) {
		std::cout << std::endl;
		int select;

		std::cout << "========================" << std::endl;
		std::cout << "===    1.  로그인    ===" << std::endl;
		std::cout << "===    2. 채팅하기   ===" << std::endl;
		std::cout << "===    3. 파티확인   ===" << std::endl;
		std::cout << "===    4. 친구확인   ===" << std::endl;
		std::cout << "= 5. 친구 추가 및 삭제 =" << std::endl;
		std::cout << "===    6. 로그아웃   ===" << std::endl;
		std::cout << "===    7.  나가기    ===" << std::endl;
		std::cout << "========================" << std::endl;

		if (friendCnt != 0) {
			std::cout <<"현재 접속중인 친구 수 : " << friendCnt << std::endl;
			std::cout << "========================" << std::endl;
		}
		//// 여기에 현재 파티 뜰 수 있게
		//if (login_status) {
		//	char buffer[PACKET_SIZE];
		//	memset(buffer, 0, PACKET_SIZE);
		//	std::string sends;
		//	char* sendc = new char[sends.length() + 1];
		//	sends += ("4 1 " + realid);
		//	sendc[sends.length()] = '\n';
		//	sends.copy(sendc, sends.length());
		//	send(soc, sendc, PACKET_SIZE, 0);
		//	recv(soc, buffer, PACKET_SIZE, 0);

		//	if (std::string(buffer) == "1") {
		//		std::cout << "현재 파티원이 없습니다." << std::endl;
		//		std::cout << "========================" << std::endl;
		//	}
		//	else {
		//		std::cout << "현재 파티원 수 : " << std::string(buffer) << std::endl;
		//		std::cout << "========================" << std::endl;
		//	}

		//}
		//if (login_status) {

		//	// 이건 파티 요청이 있을때만 보여지게 하자. (쓰레드로 만들어보자)
		//	std::cout << "들어온 파티 초대 요청" << std::endl;
		//	std::cout << "========================" << std::endl;
		//}

		////현재 접속중인 친구 수 확인
		//if (login_status) {

		//	char buffer[PACKET_SIZE];
		//	memset(buffer, 0, PACKET_SIZE);
		//	std::string sends;
		//	char* sendc = new char[sends.length() + 1];
		//	sends += ("98 " + realid);
		//	sendc[sends.length()] = '\n';
		//	sends.copy(sendc, sends.length());

		//	send(soc, sendc, PACKET_SIZE, 0);
		//	recv(soc, buffer, PACKET_SIZE, 0);
		//	std::cout << "현재 접속중인 친구 : " << buffer << std::endl;
		//	std::cout << "========================" << std::endl;

		//}

		//char buffer[PACKET_SIZE];
		//memset(buffer, 0, PACKET_SIZE);
		//std::string sends;
		//char* sendc = new char[sends.length() + 1];
		//sends += ("95 1 " + realid);
		//sendc[sends.length()] = '\n';
		//sends.copy(sendc, sends.length());

		//send(soc, sendc, PACKET_SIZE, 0);
		//recv(soc, buffer, PACKET_SIZE, 0);

		//std::string new_friend;
		//new_friend = buffer;
		//int new_friends_num = stoi(new_friend);

		//if (new_friends_num && login_status) {
		//	std::cout << "친구추가 요청 " << new_friends_num << "건이 있습니다." << std::endl;
		//	std::cout << "========================" << std::endl;
		//	new_friends_req = true;
		//}

		//// 0건이면 친추 요청 안보여줌 
		//else {
		//	new_friends_req = false;
		//}

		std::cout << std::endl;
		std::cin >> select;
		std::cout << std::endl;

		switch (select) {

			//로그인
		case 1: {
			//로그인 아직 안된 상태
			if (UserState == USER_STATE::NONE) {
				UINT32 temp = login(soc);
				if (temp < 31) {
					friendCnt = temp;
					getFriendsFServer(soc, friendCnt);
					UserState = USER_STATE::LOGIN;
					std::cout << "로그인 성공" << std::endl;
				}
				else if (temp == 31) {
					UserState = USER_STATE::NONE;
					std::cout << "서버에 유저 가득참" << std::endl;
				}
				else if (temp == 34) {
					UserState = USER_STATE::NONE;
					std::cout << "유저 이미 접속 중" << std::endl;
				}
				else if (temp == 35) {
					UserState = USER_STATE::NONE;
					std::cout << "로그인 실패" << std::endl;
				}
			}
			//로그인 된 상태
			else {
				std::cout << "이미 로그인 되어 있어요 ! 메인 화면으로 돌아갑니다." << std::endl << std::endl;
			}
		}
			  break;

			  //채팅하기
		case 2: {

			// 파티원이 2명 되자마자 쓰레드 실행. 한명이면 쓰레드 다시 종료.

			if (UserState == USER_STATE::NONE) {
				std::cout << "로그인을 먼저 해주세요" << std::endl << std::endl;
			}

			// 접속중인 친구 목록 요청
			else {
				std::cout << "귓속말을 원하면 10100을 입력 후 친구의 아이디를 입력해 주세요 (메인화면으로 돌아가시려면 10101을 입력해주세요.) : " << std::endl;
				while (1) {

					// 전체 채팅 쓰레드 실행
					std::string sends = "3 1 ";
					sends += realid;
					char* sendc = new char[sends.length() + 1];
					sendc[sends.length()] = '\n';
					sends.copy(sendc, sends.length());
					char buffer[PACKET_SIZE] = { 0 };
					memset(buffer, 0, sizeof(buffer));
					send(soc, sendc, PACKET_SIZE, 0);
					recv(soc, buffer, PACKET_SIZE, 0);

					std::string temp_can_chat = buffer;

					if (temp_can_chat == "1") {

						// 파티원 전체채팅 받음 이건 파티원 전부 나갈 때 까지 안꺼짐
						// std::thread proc1(thr_recvs_party);

						// 채팅 계속 입력하고 받는 while문
						while (!WSAGetLastError()) {

							// 띄어쓰기도 받기 위해서 cin말고 cin.getline 사용
							std::cin.getline(buffer, PACKET_SIZE, '\n');
							std::string finish = buffer;

							if (finish == "10101") {
								send(soc, "10101 ", strlen(buffer), 0);
								break;
							}

							// 귓속말 진행
							else if (finish == "10100") {

								//접속중인 친구 목록 요청 보냄
								std::string sends = "3 2 ";
								sends += (realid);
								char* sendc = new char[sends.length() + 1];
								sendc[sends.length()] = '\n';
								sends.copy(sendc, sends.length());
								char buffer2[PACKET_SIZE] = { 0 };
								memset(&buffer2, 0, sizeof(buffer2));
								send(soc, sendc, strlen(sendc), 0);
								recv(soc, buffer2, PACKET_SIZE, 0);

								//buffer2에서 받은 목록 출력 (1, 한번에 ,로 구분해서 받아서 split, 2. 일단 친구 수 k면 k만큼 반복문 만들어서 그만큼 계속해서 recv받기)
								char* nulltext = NULL;
								char* friends = strtok_s(buffer2, " ", &nulltext);
								int cnt = 1;

								std::cout << "현재 접속중인 친구목록" << std::endl << std::endl;
								while (friends != NULL) {
									std::cout << cnt++ << ". " << friends << std::endl;
									friends = strtok_s(NULL, " ", &nulltext);
								}

								while (1) {

									std::string want_chat;
									std::cout << "채팅 원하는 친구 아이디를 입력해 주세요(귓속말 취소는 10101)" << std::endl;
									std::cin >> want_chat;

									if (want_chat == "10101") break;

									sends = "97 ";
									sends += (realid + " " + want_chat);
									char* sendc3 = new char[sends.length() + 1];
									sendc3[sends.length()] = '\n';
									sends.copy(sendc3, sends.length());
									char buffer3[PACKET_SIZE] = { 0 };
									memset(&buffer3, 0, sizeof(buffer3));
									send(soc, sendc3, PACKET_SIZE, 0);
									recv(soc, buffer3, PACKET_SIZE, 0);

									// 친구 목록에 그 아이디가 존재할때
									if (std::string(buffer3) == "1") {

										// 귓속말 채팅 요청 보냄
										sends = "3 2 ";
										sends += (realid + " " + want_chat);

										char* sendc2 = new char[sends.length() + 1];
										sendc2[sends.length()] = '\n';
										sends.copy(sendc2, sends.length());
										send(soc, sendc2, strlen(sendc2), 0);

										char buffer[PACKET_SIZE] = { 0 };
										char msg[PACKET_SIZE] = {};
										std::cout << std::endl;
										std::cout << "귓속말을 종료하고 전체 채팅을 하시려면 10101입력" << std::endl;

										// 띄어쓰기도 받기 위해서 cin말고 cin.getline 사용
										std::cin.getline(buffer, PACKET_SIZE, '\n');
										std::string finish = buffer;
										if (buffer) {
											std::cout << "[ " << want_chat << " ]님에게 : " << buffer << std::endl;
										}
										send(soc, buffer, strlen(buffer), 0);
									}

									//친구 목록에 그 아이디가 존재하지 않을 때
									else {
										std::cout << "친구목록에 없습니다. 다시 입력해주세요." << std::endl;
										continue;
									}

								} // 귓속말 시작 while 문

							} //10100 귓속말 else if 문


					// 종료나 귓속말 둘다 아닐때 (그냥 전체 채팅 진행)
							else {
								std::cout << realid << ": ";
								send(soc, buffer, strlen(buffer), 0);
							}

						} // 전체채팅 while문

					} // 채팅창 들어왔을때 전체채팅 가능하게 하는 if문
					else continue;

				}// 채팅창 들어왔을때 전체채팅 가능할때 까지 도는 while문

				break;
			}
			break;
		}


			  // 파티확인
		case 3: {
			if (UserState == USER_STATE::NONE) {
				std::cout << "로그인을 먼저 해주세요" << std::endl << std::endl;
			}
			else {

				while (1) {

					/*sends = ("4 2 " + );*/

					// 이건 파티 요청이 있을때만 보여지게 하자.
					std::cout << "들어온 파티 요청" << std::endl;

					//파티가 있을때 (없으면 나만 뜨게 만들기)
					std::cout << "=======================" << std::endl;
					std::cout << "현재 파티원" << std::endl;

					std::cout << "=======================" << std::endl << std::endl;

					//파티원이 두명 이상일때 뜨게 하기
					std::cout << "파티 나가기" << std::endl;

					//파티장일때만 뜨게 하기
					std::cout << "파티원 추방하기" << std::endl;
					std::cout << "파티장 넘기기" << std::endl;

					std::cout << "1. 파티 참가하기" << std::endl;
					std::cout << "2. 파티 초대하기" << std::endl;

					//접속중인 친구 목록 요청 보냄
					std::string sends = "3 2 ";
					sends += (realid);
					char* sendc = new char[sends.length() + 1];
					sendc[sends.length()] = '\n';
					sends.copy(sendc, sends.length());
					char buffer2[PACKET_SIZE] = { 0 };
					memset(&buffer2, 0, sizeof(buffer2));
					send(soc, sendc, strlen(sendc), 0);
					recv(soc, buffer2, PACKET_SIZE, 0);

					//buffer2에서 받은 목록 출력 (1, 한번에 ,로 구분해서 받아서 split, 2. 일단 친구 수 k면 k만큼 반복문 만들어서 그만큼 계속해서 recv받기)
					char* nulltext = NULL;
					char* friends = strtok_s(buffer2, " ", &nulltext);
					int cnt = 1;

					std::cout << "현재 접속중인 친구목록" << std::endl << std::endl;
					while (friends != NULL) {
						std::cout << cnt++ << ". " << friends << std::endl;
						friends = strtok_s(NULL, " ", &nulltext);
					}
					std::cout << std::endl;

					int party_select_num;
					std::cin >> party_select_num;


				}

			}

			break;
		}

			  //친구요청
		case 4: {

			if (UserState == USER_STATE::NONE) {
				std::cout << "로그인을 먼저 해주세요" << std::endl << std::endl;
			}

			else if (UserState == USER_STATE::LOGIN) {

				for (int i = 0; i < Friendsv.size(); i++) {
					std::cout << "현재 접속중인 친구" << i + 1 << " 아이디는 : " << Friendsv[i].id << std::endl;
					std::cout << "현재 접속중인 친구" << i + 1 << " 레벨 : " << Friendsv[i].userLevel << std::endl;
					if(Friendsv[i].partyIdx>0)
					std::cout << Friendsv[i].id << "님은 파티가 있습니다." << std::endl;
					else
						std::cout << Friendsv[i].id << "님은 파티가 없습니다." << std::endl;
					std::cout << std::endl;
				}

				std::cout << "메인 메뉴로 돌아가려면 1번을 눌러주세요" << std::endl;

				int k;
				std::cin >> k;
				if (k == 1) break;

				//while (1) {
				//	int cnt = 1;

				//	if (UserState == USER_STATE::NONE) {
				//		std::string sends = "95 2 ";
				//		sends += (realid);
				//		char* sendc = new char[sends.length() + 1];
				//		sendc[sends.length()] = '\n';
				//		sends.copy(sendc, sends.length());
				//		char buffer2[PACKET_SIZE] = { 0 };
				//		memset(&buffer2, 0, sizeof(buffer2));
				//		send(soc, sendc, strlen(sendc), 0);
				//		recv(soc, buffer2, PACKET_SIZE, 0);

				//		std::cout << "새로운 친구추가 요청" << std::endl << std::endl;
				//		std::cout << "=======================" << std::endl;
				//		//std::string으로 문자열 자르기
				//		//buffer3에서 받은 목록 출력 (1, 한번에 ,로 구분해서 받아서 split, 2. 일단 친구 수 k면 k만큼 반복문 만들어서 그만큼 계속해서 recv받기)
				//		std::string temp_rcv_friends_s = buffer2;
				//		std::string temp_string;
				//		char temp_rcv_friends_c = ' ';
				//		std::stringstream ss(temp_rcv_friends_s);

				//		while (getline(ss, temp_string, ',')) {
				//			std::cout << cnt++ << ". " << temp_string << "님이 친구추가 요청을 하였습니다." << std::endl;
				//		}

				//		std::cout << "=======================" << std::endl << std::endl;
				//		std::cout << "친구 추가 하실 번호를 입력 후 엔터를 눌러주세요. (친구 추가는 한번에 한번씩 가능, 나가시려면 10101을 눌러주세요))" << std::endl;
				//		std::cout << "친구 추가 할 번호 : ";
				//	}

				//	else {
				//		std::cout << "새로운 친구 요청이 없습니다. 뒤로 가시려면 10101을 눌러주세요." << std::endl;
				//	}

				//	std::string temp_friend_req;
				//	std::cin >> temp_friend_req;

				//	int temp_friend_req_s = stoi(temp_friend_req);
				//	//입력한 수가 위 cnt 범위 안에 있을 때
				//	if (temp_friend_req == "10101") break;

				//	if (temp_friend_req_s <= cnt && temp_friend_req_s > 0) {

				//		std::string sends = "94 ";
				//		sends += (realid + " " + temp_friend_req);
				//		char* sendc = new char[sends.length() + 1];
				//		sendc[sends.length()] = '\n';
				//		sends.copy(sendc, sends.length());
				//		send(soc, sendc, strlen(sendc), 0);

				//	}

				//	//입력한 수가 위에 적힌 수보다 많거나 적을 때
				//	else {
				//		std::cout << "위에 적힌 번호 중에서 입력해 주세요" << std::endl << std::endl;
				//	}

				//}
			}

			break;
		}

			  // 친구 추가 및 삭제
		case 5: {
			if (UserState == USER_STATE::NONE) {
				std::cout << "로그인을 먼저 해주세요" << std::endl << std::endl;
			}

			else {
				while (1) {

					//내 친구 목록 요청 보냄
					std::string sends = "100 ";
					sends += (realid);
					char* sendc = new char[sends.length() + 1];
					sendc[sends.length()] = '\n';
					sends.copy(sendc, sends.length());
					char buffer2[PACKET_SIZE] = { 0 };
					memset(&buffer2, 0, sizeof(buffer2));
					send(soc, sendc, strlen(sendc), 0);
					recv(soc, buffer2, PACKET_SIZE, 0);

					std::string want_chat;

					//char로 문자열 자르기
					//buffer2에서 받은 목록 출력 (1, 한번에 ,로 구분해서 받아서 split, 2. 일단 친구 수 k면 k만큼 반복문 만들어서 그만큼 계속해서 recv받기)
					char* nulltext = NULL;
					char* friends = strtok_s(buffer2, " ", &nulltext);
					int cnt = 1;

					std::cout << "현재 접속중인 친구목록" << std::endl << std::endl;
					std::cout << "=======================" << std::endl;
					while (friends != NULL) {
						std::string tempchar;

						for (int i = 0; i < strlen(friends) - 1; i++) {
							tempchar += friends[i];
						}
						if (friends[strlen(friends) - 1] == '1') {
							std::cout << cnt++ << ". " << tempchar << "님 접속중입니다." << std::endl;
						}
						else {
							std::cout << cnt++ << ". " << tempchar << "님 접속중이 아닙니다." << std::endl;
						}
						friends = strtok_s(NULL, " ", &nulltext);
					}
					std::cout << "=======================" << std::endl << std::endl;



					std::string add_friends;
					std::string addordelete;
					std::cout << std::endl;
					std::cout << "친구추가 하시려면 1을 입력하고 삭제 하시려면 2를 입력하고 엔터 후 아이디를 입력 주세요.(뒤로 가시려면 10101을 눌러주세요) : ";
					std::cin >> addordelete;
					std::cout << std::endl;

					if (addordelete == "10101") break;

					else if (addordelete == "1") {
						std::cin >> add_friends;
						std::string sends = "96 1 ";
						sends += (realid + " ");
						sends += add_friends;
						memset(&buffer2, 0, sizeof(buffer2));
						char* sendc2 = new char[sends.length() + 1];
						sendc2[sends.length()] = '\n';
						sends.copy(sendc2, sends.length());
						send(soc, sendc2, PACKET_SIZE, 0);
						recv(soc, buffer2, PACKET_SIZE, 0);
						std::string rcv_temp = buffer2;
						//요청 친구 있음
						if (rcv_temp == "1") {

							//친구신청
							std::string sends = "96 3 ";
							sends += (realid + " ");
							sends += add_friends;
							char* sendc2 = new char[sends.length() + 1];
							sendc2[sends.length()] = '\n';
							sends.copy(sendc2, sends.length());
							send(soc, sendc2, PACKET_SIZE, 0);

							std::cout << add_friends << "님에게 친구추가 요청을 보냈습니다." << std::endl << std::endl;

						}
						//요청 친구 없음
						else {
							std::cout << add_friends << "님은 없는 유저입니다. 다시 입력해주세요" << std::endl << std::endl;
						}
					}

					else if (addordelete == "2") {
						std::cin >> add_friends;
						std::string sends = "96 2 ";
						sends += (realid + " ");
						sends += add_friends;
						memset(&buffer2, 0, sizeof(buffer2));
						char* sendc2 = new char[sends.length() + 1];
						sendc2[sends.length()] = '\n';
						sends.copy(sendc2, sends.length());
						send(soc, sendc2, PACKET_SIZE, 0);
						recv(soc, buffer2, PACKET_SIZE, 0);
						std::string rcv_temp = buffer2;
						// 요청 친구 있음
						if (rcv_temp == "1") {

							// 친구 삭제
							std::string sends = "96 4 ";
							sends += (realid + " ");
							sends += add_friends;
							char* sendc2 = new char[sends.length() + 1];
							sendc2[sends.length()] = '\n';
							sends.copy(sendc2, sends.length());
							send(soc, sendc2, PACKET_SIZE, 0);

							std::cout << add_friends << "님을 친구에서 삭제하였습니다" << std::endl << std::endl;

						}
						//요청 친구 없음
						else {
							std::cout << add_friends << "님은 없는 유저입니다. 다시 입력해주세요" << std::endl << std::endl;
						}
					}

					else std::cout << "1,2,10101 세개의 번호 중 한가지를 입력해주세요" << std::endl << std::endl;

				}
				std::cout << std::endl;

			}
			break;
		}

			  // 로그아웃
		case 6: {

			if (UserState == USER_STATE::NONE) {
				std::cout << "로그인 상태가 아닙니다." << std::endl << std::endl;
			}
			else {
				std::string sends = "101 ";
				sends += realid;
				char* sendc = new char[sends.length() + 1];
				sendc[sends.length()] = '\n';
				sends.copy(sendc, sends.length());

				UserState = USER_STATE::NONE;
				send(soc, sendc, PACKET_SIZE, 0);
			}
			break;
		}
			  // 나가기
		case 7: {
			std::cout << "다음에 또 만나요" << std::endl;

			//로그인 상태면 로그아웃메시지 보내주기
			if (UserState == USER_STATE::NONE) {
				std::string sends;
				sends += (101 + " ");
				sends += (realid);
				char* sendc = new char[sends.length() + 1];
				sendc[sends.length()] = '\n';
				sends.copy(sendc, sends.length());

				UserState = USER_STATE::NONE;
				send(soc, sendc, PACKET_SIZE, 0);

				// 파티가 있으면 접속 안한 상태
			}

			closesocket(soc);
			std::cout << "연결이 종료되었습니다.";
			WSACleanup();
			return 0;

		}

			  //다시 메인 화면으로 돌아감
			  continue;
		}
	}
}