#pragma once

class Singleton
{
public:
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	static Singleton& Get()
	{
		static Singleton instance;
		return instance;
	}
private:
	Singleton() = default;

};
