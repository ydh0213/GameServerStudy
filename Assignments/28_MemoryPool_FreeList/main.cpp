#include <iostream>
#include <vector>
#include "MemoryPool.h"

using namespace std;

class Bullet
{
public:
	int x, y;

	Bullet() : x(0), y(0) { cout << "총알 생성!\n"; }

	~Bullet() { cout << "총알 소멸!\n"; }

	void Fire(int startX, int startY)
	{
		x = startX;
		y = startY;
		cout << "총알 발사! (" << x << ", " << y << ")\n";
	}
};

int main()
{
	const int n = 10;
	MemoryPool<Bullet> BulletPool(n, true);

	cout << "\n[메모리풀] Use: " << BulletPool.GetUseCount() << " / Capacity: " << BulletPool.GetCapacityCount() << "\n\n";

	vector<Bullet*> bullets(n);
	for (int i = 0; i < n; i++)
	{
		bullets[i] = BulletPool.Alloc();
		bullets[i]->Fire(2 * i + 3, 3 * i + 7);
	}

	cout << "\n[메모리풀] Use: " << BulletPool.GetUseCount() << " / Capacity: " << BulletPool.GetCapacityCount() << "\n\n";

	for (int i = 0; i < 2; ++i)
	{
		Bullet* pBullet = BulletPool.Alloc();
		pBullet->Fire(1000 + 2 * i, 1000 + 3 * i);
		bullets.emplace_back(pBullet);
	}

	cout << "\n[메모리풀] Use: " << BulletPool.GetUseCount() << " / Capacity: " << BulletPool.GetCapacityCount() << "\n\n";

	for (Bullet* const bullet : bullets)
		BulletPool.Free(bullet);

	bullets.clear();

	cout << "\n[메모리풀] Use: " << BulletPool.GetUseCount() << " / Capacity: " << BulletPool.GetCapacityCount() << "\n\n";

	return 0;
}
