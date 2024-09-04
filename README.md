![존댓말 일반전](https://github.com/user-attachments/assets/e7f396c4-cdc7-4ad2-9c1c-486de3d58643)![존댓말 일반전](https://github.com/user-attachments/assets/e9bff138-5b26-4466-8bb5-05198291dbbd)# 비동기 처리를 이용한 게임 서버 로비 시스템 개발 (C++ 개인 프로젝트)

<br>  

## [소개]

<br>  

최근에 플레이 한 게임 중 만들기 괜찮다고 느꼈던 배틀그라운드의 로비 시스템을 본따서 로그인, 친구, 파티, 매칭시스템을 구현 해 보았습니다

개발기간 : 2024.08.01 ~ 2024.09.01

<br>

## [목표]

<br>  

- IOCP를 사용하여 대규모 접속 처리를 해결 해 보며, 비동기 처리와 멀티쓰레드 사용의 숙련도를 높이기 위한 것을 목표로 하였습니다. 

- 많은 내용을 한번에 전송하기 위한 구조체 전송도 가능하게 하는것을 목표로 하였습니다.

- 배틀그라운드의 로비 시스템에서 친구, 파티, 매칭 시스템 구현을 목표로 하였습니다.


<br>  

## [설계]

<br>  

- #### Server Flow

![비동기 포폴 2차 drawio](https://github.com/user-attachments/assets/b88f64ab-942f-430d-8713-1d96ed722734)

<br>

<br>

- #### Login

![로그인인 drawio](https://github.com/user-attachments/assets/c00dc820-349c-42fd-a169-215bd8e3d30b)

<br>

<br>

- #### Friends

<br>

<br>

- #### Party

<br>

<br>

- #### Matching

#### 일반전

![존댓말 일반전](https://github.com/user-attachments/assets/c6ddd0a6-e278-434e-8412-ee3f75db48f7)


<br>

#### 경쟁전

![경쟁전](https://github.com/user-attachments/assets/2fcfe184-51e2-435c-b054-3b96e8d86740)


<br>

## [화면단]

<br>  

- 아이디, 비밀번호 체크를 하며 로그인을 성공 여부를 서버가 전송 해 줍니다.
  
![로그인 체크](https://github.com/user-attachments/assets/ced5caf0-e7a9-4904-aad2-4e49df787ad2)


<br>

- 로그인 성공여부와 함께 클라이언트에게 친구 목록을 구조체로 보내 줍니다.

![친구불러옴](https://github.com/user-attachments/assets/d6764090-49eb-4b86-8a57-c053aae505ff)

<br>


- 접속종료가 되면 정보가 초기화됨으로 재접속이 가능합니다.

![재접속](https://github.com/user-attachments/assets/3ac3630b-68c3-42ec-9a90-9662c83cf2f1)
