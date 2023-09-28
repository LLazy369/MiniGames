#include<iostream>
#include<windows.h>
#include<thread>
#include<vector>

using namespace std;

wstring tetromino[7];                   //存放七种不同形状的方块

int fieldWidth = 12;
int fieldHeight = 18;
unsigned char* pField = nullptr;

//创建不同形状的方块
void CreateTetromino() {

	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"....");

	tetromino[4].append(L"..X.");
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"....");
	tetromino[5].append(L".XX.");
	tetromino[5].append(L"..X.");
	tetromino[5].append(L"..X.");

	tetromino[6].append(L"....");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L".X..");
	tetromino[6].append(L".X..");

	return;
}

//旋转
int Rotato(int x, int y, int r) {

	switch (r % 4) {
	case 0:return y * 4 + x;                         //旋转90
	case 1:return 12 + y - 4 * x;                    //旋转180
	case 2:return 15 - 4 * y - x;                    //旋转270
	case 3:return 3 + 4 * x - y;                     //旋转360
	}

	return 0;
}

//方块是否适合放在指定位置
bool DoesPieceFit(int teromino, int rotation, int posX, int posY) {

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			//方块内部位置
			int i = Rotato(x, y, rotation);
			//方块在游戏区域的位置
			int fi = (posY + y) * fieldWidth + (posX + x);

			if (posX + x >= 0 && posX + x < fieldWidth) {
				if (posY + y >= 0 && posY + y < fieldHeight) {
					//如果发生碰撞，返回false
					if (tetromino[teromino][i] == L'X' && pField[fi] != 0) {
						return false;
					}
				}
			}
		}
	}

	return true;
}

void Teris() {

	int screenWidth = 80;
	int screenHeight = 48;                 //如果按照教程的30，在Windows10上会有显示问题
	wchar_t* screen = nullptr;

	HANDLE hConsole;
	DWORD dwBytesWritten;

	bool gameOver = false;

	int currentPiece = 0;
	int currentRotation = 0;
	int currentX = fieldWidth / 2;
	int currentY = 0;

	bool key[4];
	bool rotateHold = false;

	int speed = 20;
	int speedCount = 0;
	bool forceDown = false;
	int pieceCount = 0;

	vector<int> vLines;

	int score = 0;

	CreateTetromino();

	//创建游戏界面
	pField = new unsigned char[fieldWidth * fieldHeight];

	//用9表示边框，其余位置为0
	for (int x = 0; x < fieldWidth; x++) {
		for (int y = 0; y < fieldHeight; y++) {
			pField[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) ? 9 : 0;
		}
	}

	//创建屏幕
	screen = new wchar_t[screenWidth * screenHeight];

	for (int i = 0; i < screenWidth * screenHeight; i++) {
		screen[i] = L' ';
	}

	hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);

	dwBytesWritten = 0;

	while (!gameOver) {

		//计时
		this_thread::sleep_for(50ms);

		speedCount++;
		forceDown = (speedCount == speed);

		//玩家输入
		for (int k = 0; k < 4; k++) {
			//0x8000是16位，最高位为1，其余为0，用于检测按键状态中的最高位是否被置为（unsigned short也为16位）
			//(unsigned char)("\x27\x25\x28Z"[k]))大概是一个字符数组吧，没搞懂为啥能这么写。
			//整个就是判断键是否被按下，被按下则结果为真，否则为假，该值存储到对应的key[k]里面去。
																 //R   L    D Z
			key[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}

		//移动方块
		currentX += (key[0] && DoesPieceFit(currentPiece, currentRotation, currentX + 1, currentY)) ? 1 : 0;
		currentX -= (key[1] && DoesPieceFit(currentPiece, currentRotation, currentX - 1, currentY)) ? 1 : 0;
		currentY += (key[2] && DoesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) ? 1 : 0;

		//控制旋转
		if (key[3]) {
			currentRotation += (!rotateHold && DoesPieceFit(currentPiece, currentRotation + 1, currentX, currentY)) ? 1 : 0;
			rotateHold = true;
		}
		else {
			rotateHold = false;
		}

		if (forceDown) {
			//下落
			if (DoesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
				currentY++;
			}
			//无法下落
			else {

				//锁定方块在原位
				for (int x = 0; x < 4; x++) {
					for (int y = 0; y < 4; y++) {
						if (tetromino[currentPiece][Rotato(x, y, currentRotation)] == L'X') {
							pField[(currentY + y) * fieldWidth + (currentX + x)] = currentPiece + 1;
						}
					}
				}

				//控制速度？
				pieceCount++;
				if (pieceCount % 10 == 0) {
					if (speed >= 10) {
						speed--;
					}
				}

				//删除行
				for (int y = 0; y < 4; y++) {
					if (currentY + y < fieldHeight - 1) {
						bool line = true;

						for (int x = 1; x < fieldWidth - 1; x++) {
							line &= (pField[(currentY + y) * fieldWidth + x]) != 0;
						}

						if (line) {

							for (int x = 1; x < fieldWidth - 1; x++) {
								pField[(currentY + y) * fieldWidth + x] = 8;
							}

							vLines.push_back(currentY + y);
						}
					}
				}

				//计分
				score += 25;
				if (!vLines.empty()) {
					score += (1 << vLines.size()) * 100;
				}

				//生成下一个方块
				currentPiece = rand() % 7;
				currentRotation = 0;
				currentX = fieldWidth / 2;
				currentY = 0;

				//游戏是否结束
				gameOver = !DoesPieceFit(currentPiece, currentRotation, currentX, currentY);
			}

			speedCount = 0;
		}

		//绘制游戏区域
		for (int x = 0; x < fieldWidth; x++) {
			for (int y = 0; y < fieldHeight; y++) {
				screen[(y + 2) * screenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * fieldWidth + x]];
			}
		}

		//显示当前方块
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				if (tetromino[currentPiece][Rotato(x, y, currentRotation)] == L'X') {
					screen[(currentY + y + 2) * screenWidth + (currentX + x + 2)] = currentPiece + 65;
				}
			}
		}

		//显示分数
		swprintf_s(&screen[2 * screenWidth + fieldWidth + 6], 16, L"SCORE:%8d", score);

		//绘制屏幕
		if (!vLines.empty()) {
			WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);

			this_thread::sleep_for(40ms);

			for (auto& v : vLines) {
				for (int x = 1; x < fieldWidth - 1; x++) {
					for (int y = v; y > 0; y--) {
						pField[y * fieldWidth + x] = pField[(y - 1) * fieldWidth + x];
					}

					pField[x] = 0;
				}
			}

			vLines.clear();
		}

		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
	}

	//游戏结束
	CloseHandle(hConsole);
	cout << "Game Over !! Score: " << score << endl;
	system("pause");

	return;
}