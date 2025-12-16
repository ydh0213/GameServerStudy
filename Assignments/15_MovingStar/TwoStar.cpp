#include <cstdio>

#include "main.h"
#include "BaseObject.h"
#include "TwoStar.h"

using namespace std;

void TwoStar::Update()
{
	_x += 2;

	if (_x >= WIDTH)
		_delete = true;
}

void TwoStar::Render() const
{
	for (int i = 0; i < WIDTH; ++i)
		line[i] = ' ';

	line[_x] = '*';
	line[_x + 1] = '*';
	line[WIDTH] = '\0';

	printf("%s\n", line);
}
