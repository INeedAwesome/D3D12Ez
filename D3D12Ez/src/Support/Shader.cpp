#include "Shader.h"

#include "Support/WinInclude.h"

#include <fstream>
#include <filesystem>
#include <cstdlib>



Shader::Shader(std::string_view name)
{
	static std::filesystem::path shaderDir;

	if (!shaderDir.empty())
	{
		return;
	}

	wchar_t moduleFilename[MAX_PATH];
	GetModuleFileNameW(nullptr, moduleFilename, MAX_PATH);

	shaderDir = moduleFilename;
	shaderDir.remove_filename();

	std::ifstream shaderIn(shaderDir / name, std::ios::binary);
	if (!shaderIn.is_open())
	{
		return;
	}
	shaderIn.seekg(0, std::ios::end);
	m_size = shaderIn.tellg();
	shaderIn.seekg(0, std::ios::beg);
	m_data = malloc(m_size);
	if (m_data)
	{
		shaderIn.read((char*)m_data, m_size);
	}

}

Shader::~Shader()
{
	if (m_data)
	{
		free(m_data);
	}
}