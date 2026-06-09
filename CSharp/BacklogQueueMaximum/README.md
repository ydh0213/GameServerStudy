# backlog queue 최대치 구하기 (C#)

서버, 클라를 C++로 만들어 진행해봤던 프로젝트를 C#으로도 해보았다.



## 백로그 개수

서버에서 `listenSock.Listen((int)SocketOptionName.MaxConnections)`으로 Listen하게 되면 200개에서 끊긴다. `MaxConnections`은 C++의 `SOMAXCONN`에 해당하는 값인데, 0x7fffffff (=2147483647)이다. C++에서는 매크로 값으로 되어있지만 C#의 `MaxConnections`은 enum이라서 int로 캐스팅해서 써야한다.

<img alt="클라 200개 연결" src="https://github.com/user-attachments/assets/94fd37ca-d070-446a-8c60-a1c3aad3b8b7" />

이 값으로 설정하면 윈도우에서 백로그를 적절한 최대값으로 설정한다고 한다. 그 값이 200으로 되어있어 그 이상 받기 위해서는 C++에서는 `SOMAXCONN_HINT(65535)`로 바꿔주면 되었다. 그러면 훨씬 늘어나기는 하는데, 여전히 65535개는 안되고, 최대 16,000대 개수만큼 진행되다가 끊긴다. (테스트하고 있는 PC가 사용 중인 포트 수에 따라 이보다 적을 수 있다)  
`SOMAXCONN_HINT()`는 음수로 바꿔주는 매크로라서 C#에서는 -65535를 전달해주면 된다. 나는 `ListenWithHint()` 확장 함수를 만들어서 썼다.



## 동적 포트 고갈 문제

<img alt="클라 13000개 연결" src="https://github.com/user-attachments/assets/ba2b88f7-f5df-41dd-9300-d9f19e6679d4" />

**WSAENOBUFS** (10055) 에러가 발생하는데, 이 에러 내용은

>사용할 수 있는 버퍼 공간이 없습니다.  
시스템에 충분한 버퍼 공간이 부족하거나 큐가 가득 차서 소켓에서 작업을 수행할 수 없습니다.
>

라고 한다. 이 에러가 발생하는 이유는 다음과 같다.

이 클라와 서버 TCP 연결 폭격에서 서버 IP / Port, 클라 IP / Port 중에 나머지는 전부 동일하지만 클라의 Port 번호는 전부 다르다. Connect 때마다 가용 포트번호들 중 하나를 임의로 선택해서 연결하게 되는데, 이를 동적 포트라 한다. 동적 포트 범위는 49152 ~ 65535 이니까 총 16,384개다. 이 중에 이미 다른데서 쓰고 있는 포트들을 제외하고 남은 포트들을 써서 연결이 되고 있었다. 그러다가 더 이상 가용 포트가 없어 위 에러가 발생하고 있었던 것이다.

그렇다면 PC를 여러대 사용해서 각각 다른 IP에서 16,000여개 연결을 하는 식으로 테스트를 하면 된다. 환경에 따라 이미 사용하고 있는 포트 개수가 많아서 16,000개보다 적게 쓸 수 밖에 없는 PC도 있을 수는 있겠다. 1대당 13,200 ~ 15,000개 정도 연결할 수 있다면 5대 정도만 있으면 된다. 1대는 서버를 돌리고 있는 동일 PC에서 클라도 실행하면 되니까 추가로 4대만 있으면 된다.



### PC 1대만으로 하는 방법

PC를 5대나 구하기 힘들다면 1대로 해결하는 방법도 있다. 클라에서 Connect 직후 바로 Close를 하면 TCP 연결해제 4-handshaking 중에서 2단계까지 진행되고 클라는 FIN_WAIT_2, 서버에서는 CLOSE_WAIT 상태가 된다.

<img alt="image" src="https://github.com/user-attachments/assets/0acca73f-733d-4682-ac2e-9af9d5909a56" />

서버는 아직 Accept조차 하기 전이지만 클라에서 FIN을 보내왔으니 CLOSE_WAIT 상태가 된다.

그리고 클라에서는 120초 (Windows에서의 기본값)가 지나면 연결을 아예 끝낸다. 그렇지만 서버에서는 Accept를 하지 않았으니 아직 백로그 큐에 남아있다!

그래서 클라에서 14,000개 연결을 하고 바로 Close 하고 120초 이상 기다려 연결이 전부 해제된 걸 확인하고 다시 14,000개 연결을 하고 바로 Close 하고 120초 이상 기다리고, … 이를 반복하다보면 연결이 총 65,535개가 됐을 때 서버에 백로그 큐가 꽉차서 에러가 발생한다.

<img width="437" alt="클라 65535개 연결" src="https://github.com/user-attachments/assets/277b0d07-7d64-4616-8aca-ee614c3fb347" />

<img width="437" alt="클라 65535개 연결" src="https://github.com/user-attachments/assets/d4993925-fea6-4e57-944c-3ecca29ae09a" />

<img alt="서버" src="https://github.com/user-attachments/assets/51af288f-bef7-4382-8f07-234e08ea10f9" />



## LingerOption

소켓 옵션 중 LingerOption을 켜고 대기 시간을 0으로 하면 Close 시 즉시 RST를 전송하며 연결을 종료시킨다. 이렇게 되면 기존의 120초간 TIME_WAIT 상태는 없다.




## NP풀 메모리 사용량

백로그 큐는 NP풀 메모리를 사용한다. 처음부터 메모리를 할당해두는 것은 아니고, 연결이 늘어날수록 메모리를 더 많이 쓴다. 그래서 작업 관리자에서 NP풀 메모리 사용량을 확인해보았다.


<img alt="서버를 갓 실행했을 때" src="https://github.com/user-attachments/assets/fed1714f-e5ca-491e-8af4-b33ae1b86ce9" />
<img alt="백로그 큐에 적재만 하고 Accept하기 전" src="https://github.com/user-attachments/assets/618d63bb-e6ea-40a2-a8c8-294bd925f4d8" />
<img alt="백로그 큐에서 꺼내와 전부 Accept 했을 때" src="https://github.com/user-attachments/assets/5b47c3b1-4199-41fe-be98-c69c15637b1a" />
