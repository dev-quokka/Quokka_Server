#pragma once

#include "Define.h"
#include "PacketManager.h"
// ��Ƽ ä�� ������, ģ���� ������, ��Ƽ�� ������, �ӼӸ� ������

/*�α��� �ϸ� �������� �ο��޴� ��ȣ
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

	std::cout << "���� ������" << std::endl;

	std::atexit(clean);

	while (1) {
		if (!connect(soc, (SOCKADDR*)&addr, sizeof(addr))) {
			std::cout << "������ ����Ϸ�" << std::endl << std::endl;
			break;
		}
	}

	/* GUID guid = WSAID_CONNECTEX;
	 LPFN_CONNECTEX g_connect;
	 DWORD cbBytes{0};

	 WSAIoctl(soc, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &g_connect, sizeof(g_connect), &cbBytes, NULL, NULL);

	 OVERLAPPED over;

	 std::cout << "���� ���� �����..." << std::endl << std::endl;

	 if (g_connect(soc, (const sockaddr*)&addr, sizeof(addr), NULL, NULL, NULL, (LPOVERLAPPED)&over) == (SOCKET_ERROR && WSA_IO_PENDING) != WSAGetLastError())
	 {

		 std::cout << "������" << std::endl;
	 }
		 else std::cout << "����" << std::endl;*/


	while (1) {
		std::cout << std::endl;
		int select;

		std::cout << "========================" << std::endl;
		std::cout << "===    1.  �α���    ===" << std::endl;
		std::cout << "===    2. ä���ϱ�   ===" << std::endl;
		std::cout << "===    3. ��ƼȮ��   ===" << std::endl;
		std::cout << "===    4. ģ��Ȯ��   ===" << std::endl;
		std::cout << "= 5. ģ�� �߰� �� ���� =" << std::endl;
		std::cout << "===    6. �α׾ƿ�   ===" << std::endl;
		std::cout << "===    7.  ������    ===" << std::endl;
		std::cout << "========================" << std::endl;

		if (friendCnt != 0) {
			std::cout <<"���� �������� ģ�� �� : " << friendCnt << std::endl;
			std::cout << "========================" << std::endl;
		}
		//// ���⿡ ���� ��Ƽ �� �� �ְ�
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
		//		std::cout << "���� ��Ƽ���� �����ϴ�." << std::endl;
		//		std::cout << "========================" << std::endl;
		//	}
		//	else {
		//		std::cout << "���� ��Ƽ�� �� : " << std::string(buffer) << std::endl;
		//		std::cout << "========================" << std::endl;
		//	}

		//}
		//if (login_status) {

		//	// �̰� ��Ƽ ��û�� �������� �������� ����. (������� ������)
		//	std::cout << "���� ��Ƽ �ʴ� ��û" << std::endl;
		//	std::cout << "========================" << std::endl;
		//}

		////���� �������� ģ�� �� Ȯ��
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
		//	std::cout << "���� �������� ģ�� : " << buffer << std::endl;
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
		//	std::cout << "ģ���߰� ��û " << new_friends_num << "���� �ֽ��ϴ�." << std::endl;
		//	std::cout << "========================" << std::endl;
		//	new_friends_req = true;
		//}

		//// 0���̸� ģ�� ��û �Ⱥ����� 
		//else {
		//	new_friends_req = false;
		//}

		std::cout << std::endl;
		std::cin >> select;
		std::cout << std::endl;

		switch (select) {

			//�α���
		case 1: {
			//�α��� ���� �ȵ� ����
			if (UserState == USER_STATE::NONE) {
				UINT32 temp = login(soc);
				if (temp < 31) {
					friendCnt = temp;
					getFriendsFServer(soc, friendCnt);
					UserState = USER_STATE::LOGIN;
					std::cout << "�α��� ����" << std::endl;
				}
				else if (temp == 31) {
					UserState = USER_STATE::NONE;
					std::cout << "������ ���� ������" << std::endl;
				}
				else if (temp == 34) {
					UserState = USER_STATE::NONE;
					std::cout << "���� �̹� ���� ��" << std::endl;
				}
				else if (temp == 35) {
					UserState = USER_STATE::NONE;
					std::cout << "�α��� ����" << std::endl;
				}
			}
			//�α��� �� ����
			else {
				std::cout << "�̹� �α��� �Ǿ� �־�� ! ���� ȭ������ ���ư��ϴ�." << std::endl << std::endl;
			}
		}
			  break;

			  //ä���ϱ�
		case 2: {

			// ��Ƽ���� 2�� ���ڸ��� ������ ����. �Ѹ��̸� ������ �ٽ� ����.

			if (UserState == USER_STATE::NONE) {
				std::cout << "�α����� ���� ���ּ���" << std::endl << std::endl;
			}

			// �������� ģ�� ��� ��û
			else {
				std::cout << "�ӼӸ��� ���ϸ� 10100�� �Է� �� ģ���� ���̵� �Է��� �ּ��� (����ȭ������ ���ư��÷��� 10101�� �Է����ּ���.) : " << std::endl;
				while (1) {

					// ��ü ä�� ������ ����
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

						// ��Ƽ�� ��üä�� ���� �̰� ��Ƽ�� ���� ���� �� ���� �Ȳ���
						// std::thread proc1(thr_recvs_party);

						// ä�� ��� �Է��ϰ� �޴� while��
						while (!WSAGetLastError()) {

							// ���⵵ �ޱ� ���ؼ� cin���� cin.getline ���
							std::cin.getline(buffer, PACKET_SIZE, '\n');
							std::string finish = buffer;

							if (finish == "10101") {
								send(soc, "10101 ", strlen(buffer), 0);
								break;
							}

							// �ӼӸ� ����
							else if (finish == "10100") {

								//�������� ģ�� ��� ��û ����
								std::string sends = "3 2 ";
								sends += (realid);
								char* sendc = new char[sends.length() + 1];
								sendc[sends.length()] = '\n';
								sends.copy(sendc, sends.length());
								char buffer2[PACKET_SIZE] = { 0 };
								memset(&buffer2, 0, sizeof(buffer2));
								send(soc, sendc, strlen(sendc), 0);
								recv(soc, buffer2, PACKET_SIZE, 0);

								//buffer2���� ���� ��� ��� (1, �ѹ��� ,�� �����ؼ� �޾Ƽ� split, 2. �ϴ� ģ�� �� k�� k��ŭ �ݺ��� ���� �׸�ŭ ����ؼ� recv�ޱ�)
								char* nulltext = NULL;
								char* friends = strtok_s(buffer2, " ", &nulltext);
								int cnt = 1;

								std::cout << "���� �������� ģ�����" << std::endl << std::endl;
								while (friends != NULL) {
									std::cout << cnt++ << ". " << friends << std::endl;
									friends = strtok_s(NULL, " ", &nulltext);
								}

								while (1) {

									std::string want_chat;
									std::cout << "ä�� ���ϴ� ģ�� ���̵� �Է��� �ּ���(�ӼӸ� ��Ҵ� 10101)" << std::endl;
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

									// ģ�� ��Ͽ� �� ���̵� �����Ҷ�
									if (std::string(buffer3) == "1") {

										// �ӼӸ� ä�� ��û ����
										sends = "3 2 ";
										sends += (realid + " " + want_chat);

										char* sendc2 = new char[sends.length() + 1];
										sendc2[sends.length()] = '\n';
										sends.copy(sendc2, sends.length());
										send(soc, sendc2, strlen(sendc2), 0);

										char buffer[PACKET_SIZE] = { 0 };
										char msg[PACKET_SIZE] = {};
										std::cout << std::endl;
										std::cout << "�ӼӸ��� �����ϰ� ��ü ä���� �Ͻ÷��� 10101�Է�" << std::endl;

										// ���⵵ �ޱ� ���ؼ� cin���� cin.getline ���
										std::cin.getline(buffer, PACKET_SIZE, '\n');
										std::string finish = buffer;
										if (buffer) {
											std::cout << "[ " << want_chat << " ]�Կ��� : " << buffer << std::endl;
										}
										send(soc, buffer, strlen(buffer), 0);
									}

									//ģ�� ��Ͽ� �� ���̵� �������� ���� ��
									else {
										std::cout << "ģ����Ͽ� �����ϴ�. �ٽ� �Է����ּ���." << std::endl;
										continue;
									}

								} // �ӼӸ� ���� while ��

							} //10100 �ӼӸ� else if ��


					// ���ᳪ �ӼӸ� �Ѵ� �ƴҶ� (�׳� ��ü ä�� ����)
							else {
								std::cout << realid << ": ";
								send(soc, buffer, strlen(buffer), 0);
							}

						} // ��üä�� while��

					} // ä��â �������� ��üä�� �����ϰ� �ϴ� if��
					else continue;

				}// ä��â �������� ��üä�� �����Ҷ� ���� ���� while��

				break;
			}
			break;
		}


			  // ��ƼȮ��
		case 3: {
			if (UserState == USER_STATE::NONE) {
				std::cout << "�α����� ���� ���ּ���" << std::endl << std::endl;
			}
			else {

				while (1) {

					/*sends = ("4 2 " + );*/

					// �̰� ��Ƽ ��û�� �������� �������� ����.
					std::cout << "���� ��Ƽ ��û" << std::endl;

					//��Ƽ�� ������ (������ ���� �߰� �����)
					std::cout << "=======================" << std::endl;
					std::cout << "���� ��Ƽ��" << std::endl;

					std::cout << "=======================" << std::endl << std::endl;

					//��Ƽ���� �θ� �̻��϶� �߰� �ϱ�
					std::cout << "��Ƽ ������" << std::endl;

					//��Ƽ���϶��� �߰� �ϱ�
					std::cout << "��Ƽ�� �߹��ϱ�" << std::endl;
					std::cout << "��Ƽ�� �ѱ��" << std::endl;

					std::cout << "1. ��Ƽ �����ϱ�" << std::endl;
					std::cout << "2. ��Ƽ �ʴ��ϱ�" << std::endl;

					//�������� ģ�� ��� ��û ����
					std::string sends = "3 2 ";
					sends += (realid);
					char* sendc = new char[sends.length() + 1];
					sendc[sends.length()] = '\n';
					sends.copy(sendc, sends.length());
					char buffer2[PACKET_SIZE] = { 0 };
					memset(&buffer2, 0, sizeof(buffer2));
					send(soc, sendc, strlen(sendc), 0);
					recv(soc, buffer2, PACKET_SIZE, 0);

					//buffer2���� ���� ��� ��� (1, �ѹ��� ,�� �����ؼ� �޾Ƽ� split, 2. �ϴ� ģ�� �� k�� k��ŭ �ݺ��� ���� �׸�ŭ ����ؼ� recv�ޱ�)
					char* nulltext = NULL;
					char* friends = strtok_s(buffer2, " ", &nulltext);
					int cnt = 1;

					std::cout << "���� �������� ģ�����" << std::endl << std::endl;
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

			  //ģ����û
		case 4: {

			if (UserState == USER_STATE::NONE) {
				std::cout << "�α����� ���� ���ּ���" << std::endl << std::endl;
			}

			else if (UserState == USER_STATE::LOGIN) {

				for (int i = 0; i < Friendsv.size(); i++) {
					std::cout << "���� �������� ģ��" << i + 1 << " ���̵�� : " << Friendsv[i].id << std::endl;
					std::cout << "���� �������� ģ��" << i + 1 << " ���� : " << Friendsv[i].userLevel << std::endl;
					if(Friendsv[i].partyIdx>0)
					std::cout << Friendsv[i].id << "���� ��Ƽ�� �ֽ��ϴ�." << std::endl;
					else
						std::cout << Friendsv[i].id << "���� ��Ƽ�� �����ϴ�." << std::endl;
					std::cout << std::endl;
				}

				std::cout << "���� �޴��� ���ư����� 1���� �����ּ���" << std::endl;

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

				//		std::cout << "���ο� ģ���߰� ��û" << std::endl << std::endl;
				//		std::cout << "=======================" << std::endl;
				//		//std::string���� ���ڿ� �ڸ���
				//		//buffer3���� ���� ��� ��� (1, �ѹ��� ,�� �����ؼ� �޾Ƽ� split, 2. �ϴ� ģ�� �� k�� k��ŭ �ݺ��� ���� �׸�ŭ ����ؼ� recv�ޱ�)
				//		std::string temp_rcv_friends_s = buffer2;
				//		std::string temp_string;
				//		char temp_rcv_friends_c = ' ';
				//		std::stringstream ss(temp_rcv_friends_s);

				//		while (getline(ss, temp_string, ',')) {
				//			std::cout << cnt++ << ". " << temp_string << "���� ģ���߰� ��û�� �Ͽ����ϴ�." << std::endl;
				//		}

				//		std::cout << "=======================" << std::endl << std::endl;
				//		std::cout << "ģ�� �߰� �Ͻ� ��ȣ�� �Է� �� ���͸� �����ּ���. (ģ�� �߰��� �ѹ��� �ѹ��� ����, �����÷��� 10101�� �����ּ���))" << std::endl;
				//		std::cout << "ģ�� �߰� �� ��ȣ : ";
				//	}

				//	else {
				//		std::cout << "���ο� ģ�� ��û�� �����ϴ�. �ڷ� ���÷��� 10101�� �����ּ���." << std::endl;
				//	}

				//	std::string temp_friend_req;
				//	std::cin >> temp_friend_req;

				//	int temp_friend_req_s = stoi(temp_friend_req);
				//	//�Է��� ���� �� cnt ���� �ȿ� ���� ��
				//	if (temp_friend_req == "10101") break;

				//	if (temp_friend_req_s <= cnt && temp_friend_req_s > 0) {

				//		std::string sends = "94 ";
				//		sends += (realid + " " + temp_friend_req);
				//		char* sendc = new char[sends.length() + 1];
				//		sendc[sends.length()] = '\n';
				//		sends.copy(sendc, sends.length());
				//		send(soc, sendc, strlen(sendc), 0);

				//	}

				//	//�Է��� ���� ���� ���� ������ ���ų� ���� ��
				//	else {
				//		std::cout << "���� ���� ��ȣ �߿��� �Է��� �ּ���" << std::endl << std::endl;
				//	}

				//}
			}

			break;
		}

			  // ģ�� �߰� �� ����
		case 5: {
			if (UserState == USER_STATE::NONE) {
				std::cout << "�α����� ���� ���ּ���" << std::endl << std::endl;
			}

			else {
				while (1) {

					//�� ģ�� ��� ��û ����
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

					//char�� ���ڿ� �ڸ���
					//buffer2���� ���� ��� ��� (1, �ѹ��� ,�� �����ؼ� �޾Ƽ� split, 2. �ϴ� ģ�� �� k�� k��ŭ �ݺ��� ���� �׸�ŭ ����ؼ� recv�ޱ�)
					char* nulltext = NULL;
					char* friends = strtok_s(buffer2, " ", &nulltext);
					int cnt = 1;

					std::cout << "���� �������� ģ�����" << std::endl << std::endl;
					std::cout << "=======================" << std::endl;
					while (friends != NULL) {
						std::string tempchar;

						for (int i = 0; i < strlen(friends) - 1; i++) {
							tempchar += friends[i];
						}
						if (friends[strlen(friends) - 1] == '1') {
							std::cout << cnt++ << ". " << tempchar << "�� �������Դϴ�." << std::endl;
						}
						else {
							std::cout << cnt++ << ". " << tempchar << "�� �������� �ƴմϴ�." << std::endl;
						}
						friends = strtok_s(NULL, " ", &nulltext);
					}
					std::cout << "=======================" << std::endl << std::endl;



					std::string add_friends;
					std::string addordelete;
					std::cout << std::endl;
					std::cout << "ģ���߰� �Ͻ÷��� 1�� �Է��ϰ� ���� �Ͻ÷��� 2�� �Է��ϰ� ���� �� ���̵� �Է� �ּ���.(�ڷ� ���÷��� 10101�� �����ּ���) : ";
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
						//��û ģ�� ����
						if (rcv_temp == "1") {

							//ģ����û
							std::string sends = "96 3 ";
							sends += (realid + " ");
							sends += add_friends;
							char* sendc2 = new char[sends.length() + 1];
							sendc2[sends.length()] = '\n';
							sends.copy(sendc2, sends.length());
							send(soc, sendc2, PACKET_SIZE, 0);

							std::cout << add_friends << "�Կ��� ģ���߰� ��û�� ���½��ϴ�." << std::endl << std::endl;

						}
						//��û ģ�� ����
						else {
							std::cout << add_friends << "���� ���� �����Դϴ�. �ٽ� �Է����ּ���" << std::endl << std::endl;
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
						// ��û ģ�� ����
						if (rcv_temp == "1") {

							// ģ�� ����
							std::string sends = "96 4 ";
							sends += (realid + " ");
							sends += add_friends;
							char* sendc2 = new char[sends.length() + 1];
							sendc2[sends.length()] = '\n';
							sends.copy(sendc2, sends.length());
							send(soc, sendc2, PACKET_SIZE, 0);

							std::cout << add_friends << "���� ģ������ �����Ͽ����ϴ�" << std::endl << std::endl;

						}
						//��û ģ�� ����
						else {
							std::cout << add_friends << "���� ���� �����Դϴ�. �ٽ� �Է����ּ���" << std::endl << std::endl;
						}
					}

					else std::cout << "1,2,10101 ������ ��ȣ �� �Ѱ����� �Է����ּ���" << std::endl << std::endl;

				}
				std::cout << std::endl;

			}
			break;
		}

			  // �α׾ƿ�
		case 6: {

			if (UserState == USER_STATE::NONE) {
				std::cout << "�α��� ���°� �ƴմϴ�." << std::endl << std::endl;
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
			  // ������
		case 7: {
			std::cout << "������ �� ������" << std::endl;

			//�α��� ���¸� �α׾ƿ��޽��� �����ֱ�
			if (UserState == USER_STATE::NONE) {
				std::string sends;
				sends += (101 + " ");
				sends += (realid);
				char* sendc = new char[sends.length() + 1];
				sendc[sends.length()] = '\n';
				sends.copy(sendc, sends.length());

				UserState = USER_STATE::NONE;
				send(soc, sendc, PACKET_SIZE, 0);

				// ��Ƽ�� ������ ���� ���� ����
			}

			closesocket(soc);
			std::cout << "������ ����Ǿ����ϴ�.";
			WSACleanup();
			return 0;

		}

			  //�ٽ� ���� ȭ������ ���ư�
			  continue;
		}
	}
}