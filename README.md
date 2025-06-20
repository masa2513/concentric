# concentric

ターミナル上で下のコマンドでコンパイル

g++ -O3 filmfestival01_2_2.cpp -std=c++11 `pkg-config --cflags --libs opencv4` -framework OpenGL -framework GLUT -I/opt/homebrew/include -L/opt/homebrew/lib -lalut -framework OpenAL -Wno-deprecated

コンパイル後下のコマンドで実行

./a.out
