#include<iostream>
#include<windows.h>
#include<thread>
#include<vector>

using namespace std;

wstring tetromino[7];                   //������ֲ�ͬ��״�ķ���

int fieldWidth = 12;
int fieldHeight = 18;
unsigned char* pField = nullptr;

//������ͬ��״�ķ���
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

//��ת
int Rotato(int x, int y, int r) {

	switch (r % 4) {
	case 0:return y * 4 + x;                         //��ת90
	case 1:return 12 + y - 4 * x;                    //��ת180
	case 2:return 15 - 4 * y - x;                    //��ת270
	case 3:return 3 + 4 * x - y;                     //��ת360
	}

	return 0;
}

//�����Ƿ��ʺϷ���ָ��λ��
bool DoesPieceFit(int teromino, int rotation, int posX, int posY) {

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			//�����ڲ�λ��
			int i = Rotato(x, y, rotation);
			//��������Ϸ�����λ��
			int fi = (posY + y) * fieldWidth + (posX + x);

			if (posX + x >= 0 && posX + x < fieldWidth) {
				if (posY + y >= 0 && posY + y < fieldHeight) {
					//���������ײ������false
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
	int screenHeight = 48;                 //������ս̵̳�30����Windows10�ϻ�����ʾ����
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

	//������Ϸ����
	pField = new unsigned char[fieldWidth * fieldHeight];

	//��9��ʾ�߿�����λ��Ϊ0
	for (int x = 0; x < fieldWidth; x++) {
		for (int y = 0; y < fieldHeight; y++) {
			pField[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) ? 9 : 0;
		}
	}

	//������Ļ
	screen = new wchar_t[screenWidth * screenHeight];

	for (int i = 0; i < screenWidth * screenHeight; i++) {
		screen[i] = L' ';
	}

	hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);

	dwBytesWritten = 0;

	while (!gameOver) {

		//��ʱ
		this_thread::sleep_for(50ms);

		speedCount++;
		forceDown = (speedCount == speed);

		//�������
		for (int k = 0; k < 4; k++) {
			//0x8000��16λ�����λΪ1������Ϊ0�����ڼ�ⰴ��״̬�е����λ�Ƿ���Ϊ��unsigned shortҲΪ16λ��
			//(unsigned char)("\x27\x25\x28Z"[k]))�����һ���ַ�����ɣ�û�㶮Ϊɶ����ôд��
			//���������жϼ��Ƿ񱻰��£�����������Ϊ�棬����Ϊ�٣���ֵ�洢����Ӧ��key[k]����ȥ��
																 //R   L    D Z
			key[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}

		//�ƶ�����
		currentX += (key[0] && DoesPieceFit(currentPiece, currentRotation, currentX + 1, currentY)) ? 1 : 0;
		currentX -= (key[1] && DoesPieceFit(currentPiece, currentRotation, currentX - 1, currentY)) ? 1 : 0;
		currentY += (key[2] && DoesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) ? 1 : 0;

		//������ת
		if (key[3]) {
			currentRotation += (!rotateHold && DoesPieceFit(currentPiece, currentRotation + 1, currentX, currentY)) ? 1 : 0;
			rotateHold = true;
		}
		else {
			rotateHold = false;
		}

		if (forceDown) {
			//����
			if (DoesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
				currentY++;
			}
			//�޷�����
			else {

				//����������ԭλ
				for (int x = 0; x < 4; x++) {
					for (int y = 0; y < 4; y++) {
						if (tetromino[currentPiece][Rotato(x, y, currentRotation)] == L'X') {
							pField[(currentY + y) * fieldWidth + (currentX + x)] = currentPiece + 1;
						}
					}
				}

				//�����ٶȣ�
				pieceCount++;
				if (pieceCount % 10 == 0) {
					if (speed >= 10) {
						speed--;
					}
				}

				//ɾ����
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

				//�Ʒ�
				score += 25;
				if (!vLines.empty()) {
					score += (1 << vLines.size()) * 100;
				}

				//������һ������
				currentPiece = rand() % 7;
				currentRotation = 0;
				currentX = fieldWidth / 2;
				currentY = 0;

				//��Ϸ�Ƿ����
				gameOver = !DoesPieceFit(currentPiece, currentRotation, currentX, currentY);
			}

			speedCount = 0;
		}

		//������Ϸ����
		for (int x = 0; x < fieldWidth; x++) {
			for (int y = 0; y < fieldHeight; y++) {
				screen[(y + 2) * screenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * fieldWidth + x]];
			}
		}

		//��ʾ��ǰ����
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				if (tetromino[currentPiece][Rotato(x, y, currentRotation)] == L'X') {
					screen[(currentY + y + 2) * screenWidth + (currentX + x + 2)] = currentPiece + 65;
				}
			}
		}

		//��ʾ����
		swprintf_s(&screen[2 * screenWidth + fieldWidth + 6], 16, L"SCORE:%8d", score);

		//������Ļ
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

	//��Ϸ����
	CloseHandle(hConsole);
	cout << "Game Over !! Score: " << score << endl;
	system("pause");

	return;
}