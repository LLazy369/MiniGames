#include<iostream>
#include<Windows.h>
#include<chrono>
#include<vector>
#include<algorithm>

using namespace std;

int screenWidth = 120;
int screenHeight = 40;

float playerX = 8.0f;
float playerY = 8.0f;
float playerA = 0.0f;                          //一开始正对下面的墙

int mapHeight = 16;
int mapWidth = 16;
wstring map;

float FOV = 3.14159 / 4.0;                      //视场角
float depth = 16.0f;

void CreateMap() {
	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";

	map += L"#..........#...#";
	map += L"#..........#...#";
	map += L"#..............#";
	map += L"#..............#";
	
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";

	map += L"#.......########";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";

	return;
}

void FPS() {

	wchar_t* screen = nullptr;

	HANDLE hConsole;
	DWORD dwBytesWritten;

	//创建屏幕
	screen = new wchar_t[screenWidth * screenHeight];

	for (int i = 0; i < screenWidth * screenHeight; i++) {
		screen[i] = L' ';
	}

	hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);

	dwBytesWritten = 0;

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	//创造地图
	CreateMap();

	while (1) {

		//计算时间间隔，用在后面，让旋转的时候视觉效果更加平滑
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		//左转
		if (GetAsyncKeyState((unsigned char)'A') & 0x8000) {
			playerA -= (0.8f) * fElapsedTime;
		}
		//右转
		if (GetAsyncKeyState((unsigned char)'D') & 0x8000) {
			playerA += (0.8f) * fElapsedTime;
		}
		//前进
		if (GetAsyncKeyState((unsigned char)'W') & 0x8000) {
			playerX += sinf(playerA) * 5.0f * fElapsedTime;
			playerY += cosf(playerA) * 5.0f * fElapsedTime;

			//碰撞检测
			if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
				playerX -= sinf(playerA) * 5.0f * fElapsedTime;
				playerY -= cosf(playerA) * 5.0f * fElapsedTime;
			}
		}
		//后退
		if (GetAsyncKeyState((unsigned char)'S') & 0x8000) {
			playerX -= sinf(playerA) * 5.0f * fElapsedTime;
			playerY -= cosf(playerA) * 5.0f * fElapsedTime;

			//碰撞检测
			if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
				playerX += sinf(playerA) * 5.0f * fElapsedTime;
				playerY += cosf(playerA) * 5.0f * fElapsedTime;
			}
		}

		//大概是光线投射？
		for (int x = 0; x < screenWidth; x++) {
			                  //视场的起始角度，和转向有关  全屏幕的视野,慢慢加上去
			float rayAngle = (playerA - FOV / 2.0f) + ((float)x / (float)screenWidth) * FOV;

			float distanceToWall = 0;
			bool hitWall = false;
			bool boundary = false;

			//swprintf_s(screen, 40, L"X=%3.2f,Y=%3.2f,A=%3.2f,FPS=%3.2f，", rayAngle, distanceToWall, playerA, 1.0f / fElapsedTime);

			//玩家注视的方向
			float eyeX = sinf(rayAngle);
			float eyeY = cosf(rayAngle);

			while (!hitWall && distanceToWall < depth) {
				distanceToWall += 0.1f;

				//rayAngle，不是以playerA为起始边的角，而是，额，以y轴（在我的图里）为起点的角
				//所以这里相加没问题
				int testX = (int)(playerX + eyeX * distanceToWall);
				int testY = (int)(playerY + eyeY * distanceToWall);

				//测试射线（或者叫每一个像素对应的视线？）是否在边界之外
				if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight){
					hitWall = true;
					distanceToWall = depth;
				}
				else {
					//在场景内，看看视线是否被墙挡住
					if (map[testY * mapWidth + testX] == '#') {
						hitWall = true;

						//给墙划一下界线

						//          距离   点积
						vector<pair<float, float>> p;

						//设x=1,y=1也能显示，前提是把下面的p.at(1)注释掉
						//就是边边窄了一点
						for (int x = 0; x < 2; x++) {
							for (int y = 0; y < 2; y++) {
								float vy = (float)testY + y - playerY;
								float vx = (float)testX + x - playerX;
								float d = sqrt(vx * vx + vy * vy);
								//为啥点积这么算啊
								float dot = (eyeX * vx / d) + (eyeY * vy / d);

								//p里面存进去了四个点
								p.push_back(make_pair(d, dot));
							}
						}

						//从近到远为点排序
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) { return left.first < right.first; });

						//让角度小于arccos(0.01)的点点显示
						float bound = 0.01f;
						if (acos(p.at(0).second) < bound) { boundary = true; }
						if (acos(p.at(1).second) < bound) { boundary = true; }
						//if (acos(p.at(2).second) < bound) { boundary = true; }
					}
				}
			}

			//计算到天花板和地板的距离
			int ceiling = (float)(screenHeight / 2.0) - screenHeight / ((float)distanceToWall);
			int floor = screenHeight - ceiling;

			//使用阴影
			short shade = ' ';
			short shade1 = ' ';                   

			//距离从近到远，使用的阴影不同；用教程里的Unicode码显示会出问题
			if (distanceToWall <= depth / 4.0f) { shade = 'H'/*0x2588*/; }
			else if (distanceToWall < depth / 3.0f) { shade = '#'/*0x2593*/; }
			else if (distanceToWall < depth / 2.0f) { shade = '2'/*0x2592*/; }
			else if (distanceToWall < depth) { shade = '1'/*0x2591*/; }
			else { shade = ' '; }

			//让墙边的阴影不同
			if (boundary) { shade = '/'; }

			for (int y = 0; y < screenHeight; y++) {
				if (y < ceiling) {
					screen[y * screenWidth + x] = ' ';
				}
				else if(y > ceiling && y <= floor) {
					screen[y * screenWidth + x] = shade;
				}
				else {
					//地板的阴影，按照原来那么写显示异常
					//同时为了防止墙和地板使用相同的字符，视觉上模糊不清，减少了地板的显示字符
					float b = 1.0f - (((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f));

					//if (b < 0.25) { shade1 = '#'; }
					//else if (b < 0.5) { shade1 = 'X'; }
					//else if (b < 0.75) { shade1 = '.'; }
					//else if (b < 0.9) { shade1 = '-'; }
					//else { shade1 = ' '; }
					if (b < 0.25) { shade1 = '.'; }
					else if (b < 0.75) { shade1 = '-'; }
					else { shade1 = ' '; }

					screen[y * screenWidth + x] = shade1;
				}
			}
		}

		//在屏幕上面显示玩家位置坐标和转角等等信息
		swprintf_s(screen, 40, L"X=%3.2f,Y=%3.2f,A=%3.2f,FPS=%3.2f，", playerX, playerY, playerA, 1.0f / fElapsedTime);

		//显示地图
		for (int x = 0; x < mapWidth; x++) {
			for (int y = 0; y < mapHeight; y++) {
				screen[(y + 1) * screenWidth + x] = map[y * mapWidth + x];
			}
		}

		screen[((int)playerY + 1) * screenWidth + (int)playerX] = 'P';

		screen[screenWidth * screenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
	}
	return;
}