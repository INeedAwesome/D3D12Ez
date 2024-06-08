#pragma once

#include <string_view>

class Shader
{
public:
	Shader(std::string_view name);
	Shader() {}
	~Shader();

	inline const void* GetBuffer() const { return m_data; }
	inline size_t GetSize() const { return m_size; }

	void SetBuffer(void* data) { m_data = data; }
	void SetSize(size_t size) { m_size = size; }
private:
	void* m_data = nullptr;
	size_t m_size = 0;
};

