#include <cstdio>

#include "main.h"
#include "BaseObject.h"
#include "ThreeStar.h"

using namespace std;

void ThreeStar::Update()
{
	_x += 3;

	if (_x + 1 >= WIDTH)
		_delete = true;
}

void ThreeStar::Render() const
{
	for (int i = 0; i < WIDTH; ++i)
		line[i] = ' ';

	line[_x] = '*';
	line[_x + 1] = '*';
	line[_x + 2] = '*';
	line[WIDTH] = '\0';

	printf("%s\n", line);
}
