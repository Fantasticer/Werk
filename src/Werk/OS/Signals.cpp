
#include "Signals.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>

#include "Werk/Utility/Action.hpp"

namespace werk
{

static void handleBusError(int /*signal*/, siginfo_t *info, void * /*context*/)
{
    const char *cause;
    switch (info->si_code) {
    case BUS_ADRALN: cause = "Incorrect Memory Alignment";  break;
    case BUS_ADRERR: cause = "Nonexistent Physical Address"; break;
    case BUS_OBJERR: cause = "Object Error (Hardware)"; break;
    default: cause = "Unknown";
    }

    std::printf("\n\n******************** Bus Error ********************\n\n");
    std::printf("Faulting address: 0x%p\nCause: %s (%d)\n", info->si_addr, cause, info->si_code);

    std::abort();
}

static void handleSegfault(int /*signal*/, siginfo_t *info, void * /*context*/)
{
    const char *cause;
    switch (info->si_code) {
    case SEGV_ACCERR: cause = "Access Permissions Error";  break;
    case SEGV_MAPERR: cause = "Map Error"; break;
    default: cause = "Unknown";
    }

	std::printf("\n\n******************** Segmentation Fault ********************\n\n");
	std::printf("Faulting address: 0x%p\nCause: %s (%d)\n", info->si_addr, cause, info->si_code);

	std::abort();
}

static Action *sighupAction = nullptr;
static Action *sigintAction = nullptr;
static Action *sigusr1Action = nullptr;
static Action *sigusr2Action = nullptr;

static void handleSignal(int signal, siginfo_t * /*info*/, void * /*context*/)
{
    //Get the appropriate action
    Action *action = nullptr;
    switch (signal) {
    case SIGHUP:
        action = sighupAction;
        break;
    case SIGINT:
        action = sigintAction;
        break;
    case SIGUSR1:
        action = sigusr1Action;
        break;
    case SIGUSR2:
        action = sigusr2Action;
        break;
    }

    //Run it
    if (nullptr != action) {
        action->execute();
    }
}

bool setupSegfaultHandler()
{
    struct sigaction sa;

    std::memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = handleSegfault;
    sa.sa_flags = SA_SIGINFO;
    if (0 > sigaction(SIGSEGV, &sa, nullptr)) {
        //TODO: log
        return false;
    }

    std::memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = handleBusError;
    sa.sa_flags = SA_SIGINFO;
    if (0 > sigaction(SIGBUS, &sa, nullptr)) {
        //TODO: log
        return false;
    }

    return true;
}

bool setupSignalHandler(int signal, Action *action)
{
    struct sigaction sa;

    //Set the action
    switch (signal) {
    case SIGHUP:
        sighupAction = action;
        break;
    case SIGINT:
        sigintAction = action;
        break;
    case SIGUSR1:
        sigusr1Action = action;
        break;
    case SIGUSR2:
        sigusr2Action = action;
        break;
    default:
        fprintf(stderr, "Cannot setup signal handler for signal %d - only SIGHUP, SIGUSR1 & 2 and SIGINT may be set this way\n", signal);
        return false;
    }

    //Register the handler
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = handleSignal;
    sa.sa_flags = SA_SIGINFO;
    if (0 > sigaction(signal, &sa, nullptr)) {
        //TODO: log
        return false;
    }

    return true;
}

}
