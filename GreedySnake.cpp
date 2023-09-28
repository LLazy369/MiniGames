#include<Windows.h>
#include<list>
#include<thread>

using namespace std;

void GreedySnake() {

	int screenWidth = 120;
	int screenHeight = 40;

	//大概就是蛇身体的位置？或者段
	struct snakeSegment {
		int x;
		int y;
	};

	//添加屏幕缓冲区
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

	list<snakeSegment> snake = { {60,15},{61,15},{62,15},{63,15} ,{64,15},{65,15} ,{66,15},{67,15} ,{68,15},{69,15} };
	int foodX = 30;
	int foodY = 15;
	int score = 0;
	int snakeDirection = 3;       //3应该是西
	bool dead = false;
	bool leftKey = false, rightKey = false, leftOldKey = false, rightOldKey = false;

	while (1) {

		while (!dead) {

			//时间
			//this_thread::sleep_for(200ms);

			auto t1 = chrono::system_clock::now();
			                                              //为了让蛇在纵横两个方向上移动速度观感差不多
			while ((chrono::system_clock::now() - t1) < ((snakeDirection % 2 == 1) ? 120ms : 200ms)) {

				//获取输入
				rightKey = (0x8000 & GetAsyncKeyState((unsigned char)('D'))) != 0;
				leftKey = (0x8000 & GetAsyncKeyState((unsigned char)('A'))) != 0;

				if (rightKey && !rightOldKey) {
					snakeDirection++;

					if (snakeDirection == 4) {
						snakeDirection = 0;
					}
				}

				if (leftKey && !leftOldKey) {
					snakeDirection--;

					if (snakeDirection == -1) {
						snakeDirection = 3;
					}
				}

				rightOldKey = rightKey;
				leftOldKey = leftKey;

			}

			//游戏逻辑
			switch (snakeDirection) {
			case 0://up
				snake.push_front({ snake.front().x,snake.front().y - 1 });
				break;
			case 1://r
				snake.push_front({ snake.front().x + 1,snake.front().y });
				break;
			case 2://d
				snake.push_front({ snake.front().x,snake.front().y + 1 });
				break;
			case 3://left
				snake.push_front({ snake.front().x - 1,snake.front().y });
				break;
			}

			//碰撞检测

			//碰撞边界
			if (snake.front().x<0 || snake.front().x >screenWidth) {
				dead = true;
			}
			if (snake.front().y<3 || snake.front().y >screenHeight) {
				dead = true;
			}

			//碰撞食物
			if (snake.front().x == foodX && snake.front().y == foodY) {
				score++;

				while (screen[foodY * screenWidth + foodX] != L' ') {
					foodX = rand() % screenWidth;
					foodY = (rand() % (screenHeight - 3)) + 3;
				}

				for (int i = 0; i < 5; i++) {
					snake.push_back({ snake.back().x,snake.back().y });
				}
			}

			//撞到蛇
			for (list<snakeSegment>::iterator i = snake.begin(); i != snake.end(); i++) {
				if (i != snake.begin() && i->x == snake.front().x && i->y == snake.front().y) {
					//不小心打上了两个等号，难怪不执行
					dead = true;
				}
			}

			//去掉蛇的尾巴
			snake.pop_back();

			//清屏
			for (int i = 0; i < screenWidth * screenHeight; i++) {
				screen[i] = L' ';
			}

			//显示边界和分数
			for (int i = 0; i < screenWidth; i++) {
				screen[i] = L'=';
				screen[2 * screenWidth + i] = L'=';
			}

		    wsprintf(&screen[screenWidth + 5], L"GreedySnake score: %d", score);
			//wsprintf(&screen[screenWidth + 5], L"GreedySnake score: %d\n", dead);

			//绘制蛇的身体
			for (auto s : snake) {
				screen[s.y * screenWidth + s.x] = dead ? L'+' : L'O';
			}

			//绘制蛇头
			screen[snake.front().y * screenWidth + snake.front().x] = dead ? L'X' : L'@';

			//绘制食物
			screen[foodY * screenWidth + foodX] = L'*';

			if (dead) {
				wsprintf(&screen[15 * screenWidth + 40], L"Press 'SPACE' to play again. ");
			}

			//绘制屏幕
			WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
		}

		//等待玩家按下space
		if ((0x8000 & GetAsyncKeyState((unsigned char)('\x20'))) != 0) {
			snake = { {60,15},{61,15},{62,15},{63,15} ,{64,15},{65,15} ,{66,15},{67,15} ,{68,15},{69,15} };
			foodX = 30;
			foodY = 15;
			score = 0;
			snakeDirection = 3;       //3应该是西
			dead = false;
			leftKey = false, rightKey = false, leftOldKey = false, rightOldKey = false;
		}
	}

	return;
}