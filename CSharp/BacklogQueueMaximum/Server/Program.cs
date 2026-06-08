using System.Net;
using System.Net.Sockets;

namespace Server
{
    class Program
    {
        private const int SERVER_PORT = 8888;

        static void Main(string[] args)
        {
            Console.WriteLine($"PID: {Environment.ProcessId}");
            Console.WriteLine("======================================");

            Socket listenSock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            try
            {
                IPEndPoint serverAddr = new IPEndPoint(IPAddress.Any, SERVER_PORT);
                listenSock.Bind(serverAddr);

                // C++의 SOMAXCONN과 동일 (backlog queue 크기 200개 정도)
                // listenSock.Listen((int)SocketOptionName.MaxConnections);

                // C++의 SOMAXCONN_HINT(65535)와 동일하게 확장 함수 만들어 사용
                listenSock.ListenWithHint(65535);
            }
            catch (SocketException ex)
            {
                Console.WriteLine($"listen 실패! Error: {ex.NativeErrorCode}");
                listenSock.Close();
                return;
            }

            Console.WriteLine("서버가 listen 중...");
            Console.Write("클라에서 접속 폭격이 끝나면 '1'을 누르세요: ");

            string input = Console.ReadLine();

            if (input == "1")
            {
                Console.WriteLine("\n큐에서 연결을 수락하기 시작합니다.");
                int acceptCount = 0;
                List<Socket> acceptedSockets = new List<Socket>();

                while (true)
                    try
                    {
                        // C++의 select(0, &readFds, ...) 에 해당
                        // timeout 0: 즉시 반환
                        if (listenSock.Poll(0, SelectMode.SelectRead))
                        {
                            Socket clientSocket = listenSock.Accept();
                            ++acceptCount;
                            acceptedSockets.Add(clientSocket);
                        }
                        else
                        {
                            Console.WriteLine("큐가 비었습니다!");
                            break;
                        }
                    }
                    catch (SocketException ex)
                    {
                        Console.WriteLine($"Accept 또는 Poll 에러 발생! Error: {ex.NativeErrorCode}");
                        break;
                    }

                Console.WriteLine("\n======================================");
                Console.WriteLine($"총 수락 연결수: {acceptCount}");
                Console.Write("연결을 전부 해제하려면 '2'를 누르세요: ");

                input = Console.ReadLine();

                if (input == "2")
                {
                    foreach (Socket sock in acceptedSockets)
                        sock.Close();

                    Console.WriteLine("모든 클라이언트 소켓 연결 해제 완료.");
                }
            }

            listenSock.Close();
        }
    }

    public static class SocketExtensions
    {
        public static void ListenWithHint(this Socket socket, int backlog)
        {
            int hintBacklog = -Math.Abs(backlog);
            socket.Listen(hintBacklog);
        }
    }
}