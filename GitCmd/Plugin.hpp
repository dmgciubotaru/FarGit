#pragma once
#include <string>

#include "../include/far/plugin.hpp"
#include "../include/far/farkeys.hpp"
#include "../include/fmt/format.h"
#include "gitctl.hpp"

class Plugin
{
public:
	static Plugin& GetPlugin();

	void SetInfo(const struct PluginStartupInfo* info);
	void GetInfo(struct PluginInfo* info);
	void Exit();
	void Run(const std::string& cmd);

	void ShowError(std::string&& message);

	const struct PluginStartupInfo* operator->();
	void Debug(const std::string& msg);

private:
	Plugin();

	struct PluginStartupInfo m_far;

};