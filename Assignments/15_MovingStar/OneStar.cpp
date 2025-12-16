#include <cstdio>

#include "main.h"
#include "BaseObject.h"
#include "OneStar.h"

using namespace std;

void OneStar::Update()
{
	if (++_x == WIDTH + 1)
		_delete = true;
}

void OneStar::Render() const
{
	for (int i = 0; i < WIDTH; ++i)
		line[i] = ' ';

	line[_x] = '*';
	line[WIDTH] = '\0';

	printf("%s\n", line);
}
