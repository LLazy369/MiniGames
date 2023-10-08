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
float playerA = 0.0f;                          //һ��ʼ���������ǽ

int mapHeight = 16;
int mapWidth = 16;
wstring map;

float FOV = 3.14159 / 4.0;                      //�ӳ���
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

	//������Ļ
	screen = new wchar_t[screenWidth * screenHeight];

	for (int i = 0; i < screenWidth * screenHeight; i++) {
		screen[i] = L' ';
	}

	hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);

	dwBytesWritten = 0;

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	//�����ͼ
	CreateMap();

	while (1) {

		//����ʱ���������ں��棬����ת��ʱ���Ӿ�Ч������ƽ��
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		//��ת
		if (GetAsyncKeyState((unsigned char)'A') & 0x8000) {
			playerA -= (0.8f) * fElapsedTime;
		}
		//��ת
		if (GetAsyncKeyState((unsigned char)'D') & 0x8000) {
			playerA += (0.8f) * fElapsedTime;
		}
		//ǰ��
		if (GetAsyncKeyState((unsigned char)'W') & 0x8000) {
			playerX += sinf(playerA) * 5.0f * fElapsedTime;
			playerY += cosf(playerA) * 5.0f * fElapsedTime;

			//��ײ���
			if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
				playerX -= sinf(playerA) * 5.0f * fElapsedTime;
				playerY -= cosf(playerA) * 5.0f * fElapsedTime;
			}
		}
		//����
		if (GetAsyncKeyState((unsigned char)'S') & 0x8000) {
			playerX -= sinf(playerA) * 5.0f * fElapsedTime;
			playerY -= cosf(playerA) * 5.0f * fElapsedTime;

			//��ײ���
			if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
				playerX += sinf(playerA) * 5.0f * fElapsedTime;
				playerY += cosf(playerA) * 5.0f * fElapsedTime;
			}
		}

		//����ǹ���Ͷ�䣿
		for (int x = 0; x < screenWidth; x++) {
			                  //�ӳ�����ʼ�Ƕȣ���ת���й�  ȫ��Ļ����Ұ,��������ȥ
			float rayAngle = (playerA - FOV / 2.0f) + ((float)x / (float)screenWidth) * FOV;

			float distanceToWall = 0;
			bool hitWall = false;
			bool boundary = false;

			//swprintf_s(screen, 40, L"X=%3.2f,Y=%3.2f,A=%3.2f,FPS=%3.2f��", rayAngle, distanceToWall, playerA, 1.0f / fElapsedTime);

			//���ע�ӵķ���
			float eyeX = sinf(rayAngle);
			float eyeY = cosf(rayAngle);

			while (!hitWall && distanceToWall < depth) {
				distanceToWall += 0.1f;

				//rayAngle��������playerAΪ��ʼ�ߵĽǣ����ǣ����y�ᣨ���ҵ�ͼ�Ϊ���Ľ�
				//�����������û����
				int testX = (int)(playerX + eyeX * distanceToWall);
				int testY = (int)(playerY + eyeY * distanceToWall);

				//�������ߣ����߽�ÿһ�����ض�Ӧ�����ߣ����Ƿ��ڱ߽�֮��
				if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight){
					hitWall = true;
					distanceToWall = depth;
				}
				else {
					//�ڳ����ڣ����������Ƿ�ǽ��ס
					if (map[testY * mapWidth + testX] == '#') {
						hitWall = true;

						//��ǽ��һ�½���

						//          ����   ���
						vector<pair<float, float>> p;

						//��x=1,y=1Ҳ����ʾ��ǰ���ǰ������p.at(1)ע�͵�
						//���Ǳ߱�խ��һ��
						for (int x = 0; x < 2; x++) {
							for (int y = 0; y < 2; y++) {
								float vy = (float)testY + y - playerY;
								float vx = (float)testX + x - playerX;
								float d = sqrt(vx * vx + vy * vy);
								//Ϊɶ�����ô�㰡
								float dot = (eyeX * vx / d) + (eyeY * vy / d);

								//p������ȥ���ĸ���
								p.push_back(make_pair(d, dot));
							}
						}

						//�ӽ���ԶΪ������
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) { return left.first < right.first; });

						//�ýǶ�С��arccos(0.01)�ĵ����ʾ
						float bound = 0.01f;
						if (acos(p.at(0).second) < bound) { boundary = true; }
						if (acos(p.at(1).second) < bound) { boundary = true; }
						//if (acos(p.at(2).second) < bound) { boundary = true; }
					}
				}
			}

			//���㵽�컨��͵ذ�ľ���
			int ceiling = (float)(screenHeight / 2.0) - screenHeight / ((float)distanceToWall);
			int floor = screenHeight - ceiling;

			//ʹ����Ӱ
			short shade = ' ';
			short shade1 = ' ';                   

			//����ӽ���Զ��ʹ�õ���Ӱ��ͬ���ý̳����Unicode����ʾ�������
			if (distanceToWall <= depth / 4.0f) { shade = 'H'/*0x2588*/; }
			else if (distanceToWall < depth / 3.0f) { shade = '#'/*0x2593*/; }
			else if (distanceToWall < depth / 2.0f) { shade = '2'/*0x2592*/; }
			else if (distanceToWall < depth) { shade = '1'/*0x2591*/; }
			else { shade = ' '; }

			//��ǽ�ߵ���Ӱ��ͬ
			if (boundary) { shade = '/'; }

			for (int y = 0; y < screenHeight; y++) {
				if (y < ceiling) {
					screen[y * screenWidth + x] = ' ';
				}
				else if(y > ceiling && y <= floor) {
					screen[y * screenWidth + x] = shade;
				}
				else {
					//�ذ����Ӱ������ԭ����ôд��ʾ�쳣
					//ͬʱΪ�˷�ֹǽ�͵ذ�ʹ����ͬ���ַ����Ӿ���ģ�����壬�����˵ذ����ʾ�ַ�
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

		//����Ļ������ʾ���λ�������ת�ǵȵ���Ϣ
		swprintf_s(screen, 40, L"X=%3.2f,Y=%3.2f,A=%3.2f,FPS=%3.2f��", playerX, playerY, playerA, 1.0f / fElapsedTime);

		//��ʾ��ͼ
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