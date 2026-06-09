using System.Net.Sockets;

namespace Client
{
    class Program
    {
        private const string SERVER_IP = "127.0.0.1";
        private const int SERVER_PORT = 8888;
        private const int BATCH_NUMBER = 13200;
        private const int WAIT_MS = 1_000;

        static void Main(string[] args)
        {
            int totalConnectedCount = 0;
            bool isQueueFull = false; // 백로그 큐가 꽉 찼는지

            Console.WriteLine($"연결 폭격을 시작합니다 ({BATCH_NUMBER}개 배치 및 {WAIT_MS / 1000}초 대기 모드)...");

            while (!isQueueFull)
            {
                List<Socket> sockets = new List<Socket>();
                Console.WriteLine($"\n[배치 시작] 새로운 {BATCH_NUMBER}개 연결을 시도합니다...");

                for (int i = 0; i < BATCH_NUMBER; i++)
                {
                    Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

                    // 소켓 옵션 중 LingerOption(true, 0)으로 설정하여 Close() 시 즉시 FIN 패킷 전송 (TIME_WAIT 상태 방지)
                    sock.LingerState = new LingerOption(true, 0);

                    try
                    {
                        sock.Connect(SERVER_IP, SERVER_PORT);
                        sockets.Add(sock);

                        if (++totalConnectedCount % 1000 == 0)
                            Console.WriteLine($"{totalConnectedCount}번째 연결 큐에 진입...");
                    }
                    catch (SocketException ex)
                    {
                        // 큐가 꽉 찼거나(10061/10060) 버퍼가 부족함(10055)
                        Console.WriteLine($"\n[!] 연결 실패! 큐가 꽉 찼습니다. Error: {ex.NativeErrorCode}");
                        sock.Close();
                        isQueueFull = true;
                        break;
                    }
                }

                // 이번 배치에서 연결했던 모든 소켓을 Close() 하여 FIN 패킷 전송 (서버는 CLOSE_WAIT 상태로 전환)
                Console.WriteLine($"\n[정리 작업] 이번 배치에서 연결된 {sockets.Count}개의 소켓을 닫습니다...");
                foreach (Socket sock in sockets)
                    sock.Close();

                sockets.Clear();

                if (isQueueFull)
                    break;

                // Close() 후 TIME_WAIT 상태를 거쳐 포트가 해제되길 기다림
                Console.WriteLine($"[휴식] 포트 고갈(10055) 방지를 위해 {WAIT_MS / 1000}초간 대기합니다...");
                Thread.Sleep(WAIT_MS);
            }

            Console.WriteLine("\n======================================");
            Console.WriteLine($"테스트 종료! 총 성공한 연결 횟수: {totalConnectedCount}");
            Console.WriteLine("======================================");

            // 프로그램 종료 대기
            Thread.Sleep(Timeout.Infinite);
        }
    }
}