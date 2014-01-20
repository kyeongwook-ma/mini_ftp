mini_ftp
========
숭실대 네트워크 프로그래밍 프로젝트
========


네트워크 프로그래밍
(프로젝트 : 분산 파일 서비스 프로그램)


개발내용 요약

: 서버-클라이언트 구조의 파일관련 서비스 프로그램을 구현합니다. 복수 개의 클라이언트에서 각각 고유의 계정으로 서버에 접속하여 파일 관련 명령어를 서버로 보내면 해당 명령어를 서버에서 처리 후 결과를 클라이언트로 보내서 출력합니다. 메시지 전달은 작성한 프로토콜을 이용하며 개별 클라이언트에서 half-close 방식으로 연결을 종료합니다.


시스템 구조 설계

서버
클라이언트
․ TCP 기반의 서버로 클라이언트와 통신합니다.
․ 멀티프로세싱 기반으로 클라이언트의 다중 접속을 허용하여 개별적으로 처리합니다.
․ TCP 기반의 클라이언트로 서버와 통신합니다.
․ 싱글프로세스 기반입니다. 
․ 종료 시 active close이며 half-close 방식으로 종료합니다.



스템 시나리오 설계
① 클라이언트 - 서버 연결
② 클라이언트에서 고유의 ID/Password 로 login
③ 클라이언트에서 명령어와 데이터를 입력받음
④ 클라이언트의 create_message 함수로 패킷 생성 후 서버로 전송
⑤ 서버에서 패킷을 받아 process_message함수로 처리
⑥ 결과를 서버의 create_message 함수로 패킷 생성 후 클라이언트로 전송
⑦ 클라이언트 측에서 패킷을 통해 결과를 출력 
⑧ 클라이언트 측에서 logout 명령어를 입력 시 shutdown함수로 연결해제 요청.
⑨ 서버 측에서 Ack 메시지를 클라이언트로 전송
⑩ 메시지 출력 후 연결종료.
(멀티프로세싱 기반이므로 ①~⑩ 과정은 클라이언트 각각으로 진행)


시스템 모듈 설계

서버
클라이언트
message_protocol.h
: 프로토콜의 정보를 저장한 헤더파일입니다.
commands.h
: 각 명령어에 대한 정의를 포함한 헤더파일입니다.
create_message
: stdin 으로 입력받은 명령어와 파일정보를 프로토콜로 만드는 함수입니다.
process_message
: 클라이언트로부터 받은 프로토콜을 처리하는 함수입니다. 
․
create_message
: 처리한 결과를 프로토콜로 만드는 함수입니다.
․
logging
: 작업 내용을 파일에 쓰느 함수입니다.
․

명령어

'''
ls
cat
save
rm
mkdir
rmdir
cd
cp
'''
