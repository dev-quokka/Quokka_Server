# 비동기 처리를 이용한 게임 서버 로비 시스템 개발 [C++ IOCP 프로젝트]

<br>  

## [소개]

- 이 프로젝트는 향후 MMO 서버 개발을 위한 사전 단계로, 서버 구조 설계, 멀티스레드 환경 이해, 그리고 IOCP 기반 소켓 프로그래밍에 대한 실전 감각을 키우는 것을 목표로 진행하였습니다.

- 로그인, 친구 초대, 친구 요청 및 수락/거절, 친구 삭제 기능을 직접 설계하고 구현하였으며, 각 기능은 TCP 기반 소켓 통신과 패킷 단위 처리 방식으로 구성되어 있습니다.

- 유저의 세션 및 상태를 관리하는 구조를 통해 안정성과 확장성을 고려한 구현을 목표로 하였고, 멀티스레드 환경에서도 효율적으로 동작할 수 있도록 설계하였습니다.

- 현재는 파티 생성 및 초대 기능을 개발 중이며, 유저 간 파티 상태를 실시간으로 동기화할 수 있는 구조로 확장하고 있습니다.

<br>

#### ㅇ 개발 기간 : 2023.03 ~ 2023.11

#### ㅇ 프로젝트 소개서 - [프로젝트 소개서 [C++ IOCP 프로젝트].pdf](https://github.com/user-attachments/files/18075390/C%2B%2B.IOCP.pdf)


<br>

## [목표]

- IOCP를 사용하여 접속 처리를 해결 해 보며, 비동기 처리와 멀티쓰레드 사용의 숙련도를 높이기 위한 것을 목표로 하였습니다. 

- 대용량 데이터의 안정적인 송수신을 위해 구조체 단위 전송이 가능한 서버 구조를 설계하였습니다.

- 포인터 기반 구조체와 최소한의 문자열, 비트 단위 자료형을 활용해 성능 최적화를 시도하였습니다.

<br>  

## [흐름도]

- #### Server Flow

![Server Flow](https://github.com/user-attachments/assets/fb35e301-e174-4e29-b73c-d35ce0a86582)


<br>

- #### Login
  
<br>

![Login](https://github.com/user-attachments/assets/dcf59376-8d2d-4227-ae70-a10492446b5f)


<br>

- #### Friends

<br>

![Friends](https://github.com/user-attachments/assets/06bcc676-c73b-4853-8106-d93fbc466135)


<br>

- #### Party

<br>

![Party](https://github.com/user-attachments/assets/87690bda-355e-4943-b8c2-63703f98b86a)


<br>


<br>

## [화면단]

- 아이디, 비밀번호 체크를 하며 로그인을 성공 여부를 서버가 전송 해 줍니다.
  
![LoginCheck](https://github.com/user-attachments/assets/f660a983-d6b5-41c7-aafc-d49c9e7bc6b9)

<br>

- 로그인 성공여부와 함께 클라이언트에게 친구 목록을 구조체로 보내 줍니다. 클라이언트의 처음 화면에는 접속중인 친구수를 확인 가능합니다.

![SendFriendsInfo](https://github.com/user-attachments/assets/1165252d-6225-4931-88de-79c05a20b1e8)

<br>

- 클라이언트에서 파티초대, 친구삭제, 귓속말등을 위해 필요한 최소 정보만을 서버로 부터 전송 받습니다.
  

![CheckFriends](https://github.com/user-attachments/assets/4f138842-a470-48fa-909f-28bb0d1b58f2)


<br>


- 접속종료 및 로그아웃이 되면 정보가 초기화됨으로 재접속 및 로그인이 가능합니다.


![Logout](https://github.com/user-attachments/assets/61ee2b6a-2830-4a83-baee-596f5fc53e5e)


