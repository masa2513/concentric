# concentric

ターミナル上で下のコマンドでコンパイル

g++ -O3 filmfestival01_2_2.cpp -std=c++11 `pkg-config --cflags --libs opencv4` -framework OpenGL -framework GLUT -I/opt/homebrew/include -L/opt/homebrew/lib -lalut -framework OpenAL -Wno-deprecated

コンパイル後下のコマンドで実行

./a.out

## ラズパイ側の実行方法
### Thonnyでやる場合
Runボタンを押す
### ターミナルでやる場合
ターミナルで下のコマンドを実行

python3 game.py

## button_receiver.cppの実行方法
g++ -o button_receiver button_receiver.cpp -I/opt/homebrew/Cellar/jsoncpp/1.9.6/include -L/opt/homebrew/Cellar/jsoncpp/1.9.6/lib -ljsoncpp -pthread
./button_receiver

