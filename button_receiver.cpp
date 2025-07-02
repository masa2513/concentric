#include <json/json.h>  // libjsoncpp-dev パッケージが必要
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

class ButtonReceiver {
 private:
  int server_socket;
  int client_socket;
  std::atomic<bool> running;
  std::thread server_thread;
  const int PORT = 8080;

 public:
  ButtonReceiver() : server_socket(-1), client_socket(-1), running(false) {}

  ~ButtonReceiver() { stop(); }

  bool start() {
    // ソケット作成
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
      std::cerr << "ソケット作成エラー" << std::endl;
      return false;
    }

    // アドレス再利用設定
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // サーバーアドレス設定
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // バインド
    if (bind(server_socket, (struct sockaddr*)&server_addr,
             sizeof(server_addr)) < 0) {
      std::cerr << "バインドエラー" << std::endl;
      close(server_socket);
      return false;
    }

    // リスン
    if (listen(server_socket, 1) < 0) {
      std::cerr << "リスンエラー" << std::endl;
      close(server_socket);
      return false;
    }

    std::cout << "ポート " << PORT << " で待機中..." << std::endl;

    running = true;
    server_thread = std::thread(&ButtonReceiver::serverLoop, this);

    return true;
  }

  void stop() {
    running = false;
    if (server_socket >= 0) {
      close(server_socket);
    }
    if (client_socket >= 0) {
      close(client_socket);
    }
    if (server_thread.joinable()) {
      server_thread.join();
    }
  }

 private:
  void serverLoop() {
    while (running) {
      // クライアント接続待機
      struct sockaddr_in client_addr;
      socklen_t client_len = sizeof(client_addr);

      client_socket =
          accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
      if (client_socket < 0) {
        if (running) {
          std::cerr << "接続受付エラー" << std::endl;
        }
        continue;
      }

      std::cout << "Raspberry Piから接続されました" << std::endl;

      // データ受信ループ
      handleClient();

      close(client_socket);
      client_socket = -1;
      std::cout << "接続が切断されました" << std::endl;
    }
  }

  void handleClient() {
    char buffer[1024];
    std::string accumulated_data;

    while (running) {
      memset(buffer, 0, sizeof(buffer));
      ssize_t bytes_received =
          recv(client_socket, buffer, sizeof(buffer) - 1, 0);

      if (bytes_received <= 0) {
        break;  // 接続終了またはエラー
      }

      accumulated_data += std::string(buffer, bytes_received);

      // 改行文字で区切られたJSONデータを処理
      size_t pos;
      while ((pos = accumulated_data.find('\n')) != std::string::npos) {
        std::string json_line = accumulated_data.substr(0, pos);
        accumulated_data.erase(0, pos + 1);

        if (!json_line.empty()) {
          processJsonData(json_line);
        }
      }
    }
  }

  void processJsonData(const std::string& json_str) {
    try {
      Json::Value root;
      Json::Reader reader;

      if (reader.parse(json_str, root)) {
        std::string event = root["event"].asString();

        if (event == "button_pressed") {
          int count = root["count"].asInt();
          std::string timestamp = root["timestamp"].asString();
          int gpio_pin = root["gpio_pin"].asInt();

          std::cout << "🔘 === ボタン押下イベント ===" << std::endl;
          std::cout << "📊 回数: " << count << std::endl;
          std::cout << "📌 GPIO: " << gpio_pin << std::endl;
          std::cout << "⏰ 時刻: " << timestamp << std::endl;
          std::cout << "================================" << std::endl;

          // ログファイルに記録
          logEvent(count, timestamp, gpio_pin);

          // 特定の回数でアクションを実行
          if (count % 10 == 0) {
            std::cout << "🎉 " << count << "回到達！特別なアクションを実行"
                      << std::endl;
          }

        } else if (event == "connection_established") {
          std::string message = root["message"].asString();
          std::string timestamp = root["timestamp"].asString();
          std::cout << "✅ " << message << " [" << timestamp << "]"
                    << std::endl;

        } else if (event == "shutdown") {
          std::string message = root["message"].asString();
          std::cout << "終了通知: " << message << std::endl;
          running = false;
          return;
        }
      } else {
        std::cerr << "JSON解析エラー: " << json_str << std::endl;
      }
    } catch (const std::exception& e) {
      std::cerr << "データ処理エラー: " << e.what() << std::endl;
    }
  }

  void logEvent(int count, const std::string& timestamp, int gpio_pin) {
    std::ofstream logfile("button_log.txt", std::ios::app);
    if (logfile.is_open()) {
      logfile << timestamp << " - Button pressed (Count: " << count
              << ", GPIO: " << gpio_pin << ")" << std::endl;
      logfile.close();
    }
  }
};

int main() {
  std::cout << "C++ ボタンイベント受信プログラム" << std::endl;
  std::cout << "================================" << std::endl;

  ButtonReceiver receiver;

  if (!receiver.start()) {
    std::cerr << "サーバー開始に失敗しました" << std::endl;
    return 1;
  }

  std::cout << "Enterキーを押すと終了します..." << std::endl;
  std::cin.get();

  std::cout << "プログラムを終了しています..." << std::endl;
  receiver.stop();

  return 0;
}