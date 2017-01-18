
#include "ApplicationContext.hpp"

#include <cstdio>

#include "Werk/OS/Signals.hpp"

#include "QuitCommand.hpp"

namespace werk
{

ApplicationContext::ApplicationContext(const std::string &logFilePath)
{
	//Setup handlers for certain signals
	setupSegfaultHandler();

	_stdoutLog = new AsyncLog("StdoutLog", &_backgroundThread.backgroundClock());
	_backgroundThread.addTask(_stdoutLog);

	FILE *file = logFilePath.length() == 0 ? stdout : std::fopen(logFilePath.c_str(), "r");
	if (nullptr == file) {
		std::fprintf(stderr, "Could not open log file, redirecting to stderr.\n");
		file = stderr;
	}

	_log = new AsyncLog("Log", &_backgroundThread.backgroundClock(), file),
	_backgroundThread.addTask(_log);
	_log->logRaw(LogLevel::SUCCESS, "<Log> Initialized.");
	_log->logRaw(LogLevel::INFO, "Initializing other subsystems.");

	_config = new Config("Config", _log);
	_backgroundThread.addTask(_config);
	_log->logRaw(LogLevel::SUCCESS, "<Config> Initialized.");

	_commandManager = new CommandManager(_log);
	_commandManager->add("quit", new QuitCommand(this));
	_log->logRaw(LogLevel::SUCCESS, "<CommandManager> Initialized.");

	_log->logRaw(LogLevel::SUCCESS, "<ApplicationContext> Initialized.");
}

ApplicationContext::~ApplicationContext()
{
	//Shut down, if not already done
	if (!_backgroundThread.stopped()) {
		shutdown();
	}
}

void ApplicationContext::shutdown()
{
	if (_backgroundThread.stopped()) {
		fprintf(stderr, "ApplicationContext::shutdown - Already shut down.\n");
		return;
	}

	_log->logRaw(LogLevel::INFO, "Running shutdown actions...");
	for (Action *action : _shutdownActions) {
		action->execute();
	}
	_log->logRaw(LogLevel::SUCCESS, "Shutdown actions complete.");

	_log->logRaw(LogLevel::INFO, "Shutting down...");
	_backgroundThread.stop();
}

}
