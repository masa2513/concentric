from gpiozero import Button
import socket
import time
import json
from datetime import datetime
from signal import pause

# 設定値
BUTTON_PIN = 2,3,4,17  # ボタンが接続されているGPIOピン番号
MACBOOK_IP = "172.16.11.57"  # MacBookのIPアドレス
MACBOOK_PORT = 8888  # MacBookのサーバーポート番号
DEBOUNCE_TIME = 0.2  # ボタンのチャタリング防止時間（秒）

class ButtonSender:
    """
    Raspberry Piのボタン押下をMacBookに送信するクラス
    """
    def __init__(self):
        """
        ButtonSenderクラスの初期化
        """
        self.last_press_time = 0  # 最後にボタンが押された時刻
        self.setup_button()  # ボタンの設定を行う
        
    def setup_button(self):
        """
        ボタンの設定を行う
        """
        # gpiozeroのButtonオブジェクトを作成
        # pull_up=True: 内部プルアップ抵抗を有効化
        # bounce_time: ハードウェアレベルでのチャタリング防止
        self.button = Button(BUTTON_PIN, pull_up=True, bounce_time=DEBOUNCE_TIME)
        
        # ボタンが押されたときに呼び出される関数を設定
        self.button.when_pressed = self.button_pressed
        
    def button_pressed(self):
        """
        ボタンが押されたときに呼び出される関数
        チャタリング防止とイベント送信を行う
        """
        current_time = time.time()
        
        # ソフトウェアレベルでのチャタリング防止
        # 前回のボタン押下から一定時間内の場合は無視
        if current_time - self.last_press_time < DEBOUNCE_TIME:
            return
            
        # 最後の押下時刻を更新
        self.last_press_time = current_time
        
        # MacBookにボタン押下イベントを送信
        self.send_button_event()
        
    def send_button_event(self):
        """
        MacBookにボタン押下イベントを送信する
        TCP通信を使用してJSONデータを送信
        """
        try:
            # TCP通信のソケットを作成して接続
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.settimeout(5)  # 5秒でタイムアウト
                sock.connect((MACBOOK_IP, MACBOOK_PORT))
                
                # 送信するデータをJSON形式で作成
                message = {
                    "event": "button_pressed",  # イベント種類
                    "timestamp": datetime.now().isoformat(),  # 送信時刻（ISO形式）
                    "device": "raspberry_pi_5"  # 送信元デバイス名
                }
                
                # JSONを文字列に変換して改行文字を追加
                message_json = json.dumps(message) + "\n"
                
                # UTF-8エンコードして送信
                sock.send(message_json.encode('utf-8'))
                
                print(f"? Button press sent: {datetime.now().strftime('%H:%M:%S')}")
                
        except socket.timeout:
            print("? Send timeout")
        except ConnectionRefusedError:
            print("? Connection refused - MacBook server is not running")
        except Exception as e:
            print(f"? Send error: {e}")
            
    def run(self):
        """
        メインループを開始する
        プログラムを実行してボタンの監視を開始
        """
        print("Starting button monitoring...")
        print(f"MacBook IP: {MACBOOK_IP}:{MACBOOK_PORT}")
        print("Press Ctrl+C to exit")
        
        try:
            # gpiozeroのpause()を使用して無限ループで待機
            # ボタンイベントは別スレッドで処理される
            pause()
        except KeyboardInterrupt:
            print("\nExiting program...")
        finally:
            # リソースを適切に解放
            self.button.close()
            print("Button resources released")

if __name__ == "__main__":
    # プログラムのエントリーポイント
    sender = ButtonSender()
    sender.run()