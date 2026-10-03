#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
struct Result { int status; std::string output; };

// Run a fixed system utility with argv (never through a shell).
Result run(const std::vector<std::string>& args) {
    int pipefd[2];
    if (pipe(pipefd) != 0) return {-1, std::string("pipe: ") + std::strerror(errno)};
    pid_t pid = fork();
    if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]); close(pipefd[1]);
        std::vector<char*> argv;
        for (const auto& arg : args) argv.push_back(const_cast<char*>(arg.c_str()));
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        _exit(127);
    }
    close(pipefd[1]);
    if (pid < 0) { close(pipefd[0]); return {-1, std::string("fork: ") + std::strerror(errno)}; }
    std::string output; char buf[4096]; ssize_t n;
    while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) output.append(buf, static_cast<size_t>(n));
    close(pipefd[0]); int status = 0;
    if (waitpid(pid, &status, 0) < 0) return {-1, "waitpid failed"};
    return {WIFEXITED(status) ? WEXITSTATUS(status) : 128, output};
}

bool isLinux() {
    return access("/proc/sys/kernel", F_OK) == 0 && access("/proc/uptime", R_OK) == 0;
}
void usage() {
    std::cout << "BootScope — Linux boot-time profiler and dependency analyzer\n\n"
              << "Usage:\n  bootscope profile [--json]\n  bootscope analyze <unit>\n  bootscope recommend\n  bootscope report [--json]\n  bootscope help\n\n"
              << "Examples:\n  bootscope profile\n  bootscope analyze graphical.target\n  bootscope report --json > bootscope-report.json\n";
}
std::vector<std::pair<std::string,std::string>> parseBlame(const std::string& s) {
    std::vector<std::pair<std::string,std::string>> rows;
    std::istringstream in(s); std::string line;
    std::regex row(R"(^\s*([^\s]+)\s+(.+?)\s*$)"); std::smatch m;
    while (std::getline(in,line)) if (std::regex_match(line,m,row)) rows.emplace_back(m[1],m[2]);
    return rows;
}
void profile(bool json) {
    Result r = run({"systemd-analyze","blame","--no-pager"});
    if (r.status != 0) { std::cerr << "Could not read systemd boot timings. Is systemd-analyze installed?\n" << r.output; return; }
    auto rows = parseBlame(r.output);
    if (json) {
        std::cout << "{\n  \"source\": \"systemd-analyze blame\",\n  \"units\": [\n";
        for (size_t i=0;i<rows.size();++i) std::cout << "    {\"duration\": \"" << rows[i].first << "\", \"unit\": \"" << rows[i].second << "\"}" << (i+1<rows.size()?",":"") << "\n";
        std::cout << "  ]\n}\n";
    } else {
        std::cout << "Boot unit startup durations (longest first; systemd-analyze output):\n" << r.output;
        auto critical = run({"systemd-analyze","time"});
        if (!critical.output.empty()) std::cout << "\nBoot summary:\n" << critical.output;
    }
}
void analyze(const std::string& unit) {
    if (unit.empty() || unit.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.@:-") != std::string::npos) {
        std::cerr << "Invalid unit name. Use letters, digits, ., _, @, :, or - only.\n"; return;
    }
    Result r = run({"systemctl","list-dependencies","--all","--no-pager",unit});
    if (r.status != 0) { std::cerr << "Dependency query failed:\n" << r.output; return; }
    std::cout << "Dependency tree for " << unit << ":\n" << r.output;
    Result rev = run({"systemctl","list-dependencies","--reverse","--all","--no-pager",unit});
    if (rev.status == 0) std::cout << "\nUnits depending on " << unit << ":\n" << rev.output;
}
void recommend() {
    auto rows = parseBlame(run({"systemd-analyze","blame","--no-pager"}).output);
    if (rows.empty()) { std::cerr << "No timing data available. Run on a systemd-based Linux installation.\n"; return; }
    std::cout << "Optimization candidates (review before changing service configuration):\n";
    size_t shown=0;
    for (const auto& row : rows) {
        // The raw duration is shown as a triage lead. No service is disabled automatically.
        std::cout << "- Inspect " << row.second << " (startup time " << row.first
                  << "): check whether it is needed at boot, its unit dependencies, and its journal for delays.\n";
        if (++shown == 5) break;
    }
    std::cout << "\nUse `systemd-analyze critical-chain` to distinguish critical-path delays from parallel startup work.\n"
              << "BootScope only reports candidates; it does not modify or disable services.\n";
}
}

int main(int argc, char** argv) {
    if (!isLinux()) { std::cerr << "BootScope runs on Linux only.\n"; return 2; }
    if (argc < 2 || std::string(argv[1]) == "help" || std::string(argv[1]) == "--help") { usage(); return 0; }
    std::string cmd=argv[1]; bool json=(argc>2 && std::string(argv[2])=="--json");
    if (cmd=="profile") profile(json);
    else if (cmd=="analyze") { if (argc<3) { usage(); return 2; } analyze(argv[2]); }
    else if (cmd=="recommend") recommend();
    else if (cmd=="report") { profile(json); if (!json) { std::cout << "\n"; recommend(); } }
    else { usage(); return 2; }
    return 0;
}
