#pragma once

class BaseObject
{
protected:
	int _x = 0;

public:
	BaseObject() : _delete(false)
	{
	}

	virtual ~BaseObject()
	{
	}

	virtual void Update() = 0;
	virtual void Render() const = 0;

	bool _delete;
};

extern BaseObject* arr[HEIGHT];
