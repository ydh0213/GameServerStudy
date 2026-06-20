using System.Net.Sockets;

namespace ClientAsync
{
    class Program
    {
        private const string SERVER_IP = "127.0.0.1";
        private const int SERVER_PORT = 8888;
        private const int BATCH_NUMBER = 13200;

        static async Task Main(string[] args)
        {
            int totalConnectedCount = 0;

            Console.WriteLine($"비동기 연결 폭격을 시작합니다 ({BATCH_NUMBER}개 동시 타격)...");

            while (true)
            {
                List<Socket> sockets = new List<Socket>();
                List<Task> connectTasks = new List<Task>();

                Console.WriteLine($"\n[배치 시작] 새로운 {BATCH_NUMBER}개 연결을 동시에 시도합니다...");

                for (int i = 0; i < BATCH_NUMBER; i++)
                {
                    Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

                    // 소켓 옵션 중 LingerOption(true, 0)으로 설정하여 Close() 시 즉시 연결 종료 (TIME_WAIT 상태 방지)
                    sock.LingerState = new LingerOption(true, 0);
                    sockets.Add(sock);

                    connectTasks.Add(ConnectSocketAsync(sock, i + 1));
                }

                Console.WriteLine("모든 연결 요청(SYN)을 서버로 던졌습니다. 결과를 기다립니다...");
                await Task.WhenAll(connectTasks);

                int connectedCount = 0;
                foreach (Socket sock in sockets)
                {
                    if (sock.Connected)
                        ++connectedCount;

                    sock.Close();
                }

                totalConnectedCount += connectedCount;
                sockets.Clear();

                Console.WriteLine($"\n[결과] 이번 배치 성공: {connectedCount} / {BATCH_NUMBER}");
                Console.WriteLine($"총 성공 연결 수: {totalConnectedCount}");

                if (connectedCount < BATCH_NUMBER)
                    break;
            }
        }

        static async Task ConnectSocketAsync(Socket sock, int number)
        {
            try
            {
                await sock.ConnectAsync(SERVER_IP, SERVER_PORT);

                if (number % 1000 == 0)
                    Console.WriteLine($"{number}번째 소켓 연결 성공!");
            }
            catch (SocketException ex)
            {
            }
        }
    }
}
