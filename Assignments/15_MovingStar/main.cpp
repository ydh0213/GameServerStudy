#include <conio.h>
#include <windows.h>
#include <cstdio>

#include "main.h"
#include "BaseObject.h"
#include "OneStar.h"
#include "TwoStar.h"
#include "ThreeStar.h"

using namespace std;

char inputKey = 0;
char line[WIDTH + 1] = { 0 };
BaseObject* arr[HEIGHT] = { nullptr };

int main()
{
	while (true)
	{
		KeyProcess();

		Update();

		system("cls");
		Render();

		Sleep(50);
	}

	return 0;
}

void KeyProcess()
{
	if (_kbhit())
		inputKey = _getch();
}

void Update()
{
	switch (inputKey)
	{
	case '1':
		for (int i = 0; i < HEIGHT; ++i)
			if (!arr[i])
			{
				arr[i] = new OneStar();
				break;
			}

		break;
	case '2':
		for (int i = 0; i < HEIGHT; ++i)
			if (!arr[i])
			{
				arr[i] = new TwoStar();
				break;
			}

		break;
	case '3':
		for (int i = 0; i < HEIGHT; ++i)
			if (!arr[i])
			{
				arr[i] = new ThreeStar();
				break;
			}

		break;
	}

	inputKey = 0;

	for (int i = 0; i < HEIGHT; ++i)
		if (arr[i])
		{
			arr[i]->Update();

			if (arr[i]->_delete)
			{
				delete arr[i];
				arr[i] = nullptr;
			}
		}
}

void Render()
{
	for (int i = 0; i < HEIGHT; ++i)
		if (arr[i])
			arr[i]->Render();
		else
			printf("\n");
}
