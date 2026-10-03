# BootScope: Linux Boot-Time Profiler, Dependency Analyzer & Optimization Engine

BootScope is a C++ command-line capstone project for inspecting Linux startup performance. It uses the host's systemd tools to collect unit startup timings, display unit dependency trees, and produce human-reviewed optimization leads. It is designed for Linux with systemd and runs in VS Code using CMake.

## Features

- **Boot profiler:** reads per-unit timing data from `systemd-analyze blame` and can emit JSON.
- **Dependency analyzer:** shows forward and reverse relationships for a systemd unit.
- **Optimization engine:** highlights the five slowest units for investigation and suggests safe next steps. It never changes services automatically.
- **Linux architecture concepts:** process creation with `fork`/`exec`, pipes, `/proc` platform detection, and user-space interaction with Linux service management.

## Architecture

```text
CLI commands
    ├── profile / report ──> systemd-analyze ──> boot timing output
    ├── analyze UNIT ───────> systemctl ────────> dependency tree
    └── recommend ──────────> timings ──────────> review suggestions
```

BootScope is a user-space diagnostic. Linux device-driver concepts are included in the capstone context: drivers expose kernel-managed hardware through kernel interfaces, while this project's applicable system architecture boundary is user space → systemd/kernel interfaces. A custom device driver is not relevant to profiling systemd service startup and is intentionally not loaded or required.

## Requirements

- Linux OS with systemd
- C++17 compiler (GCC or Clang)
- CMake 3.16+
- VS Code with the C/C++ and CMake Tools extensions (recommended)

## Build on Linux

Open the `BootScope` folder in VS Code, open its terminal, and run:

```bash
cmake -S . -B build
cmake --build build
```

The executable is `build/bootscope`.

## Run

```bash
./build/bootscope help
./build/bootscope profile
./build/bootscope profile --json
./build/bootscope analyze graphical.target
./build/bootscope recommend
./build/bootscope report
./build/bootscope report --json > bootscope-report.json
```

Use a valid systemd unit name with `analyze` (for example `NetworkManager.service`). Results depend on the Linux distribution, boot, privileges, and whether systemd is the init system. Some machines may require elevated privileges for complete information; run as an administrator only when needed.

## VS Code

1. Extract `BootScope.zip` and open the extracted `BootScope` folder in VS Code on Linux.
2. Install C/C++ and CMake Tools extensions if needed.
3. Configure the project with the CMake Tools status bar, or use the build commands above.
4. Run the binary from the integrated terminal. The project cannot run natively on Windows or macOS because it relies on Linux `/proc`, `systemctl`, and `systemd-analyze`.

## Project structure

```text
BootScope/
├── CMakeLists.txt
├── README.md
├── docs/
│   └── DESIGN.md
└── src/
    └── main.cpp
```

## Safety and limitations

BootScope is read-only. It does not disable services, edit unit files, reboot, or load kernel modules. `systemd-analyze blame` reports service activation durations, which can include parallel work and do not alone establish impact on total boot time. Use `systemd-analyze critical-chain` and inspect logs before making system changes.

## Capstone evaluation/demo (5–10 minutes)

1. Explain the CLI-to-systemd architecture and Linux-only interfaces.
2. Run `profile` and identify a slow unit.
3. Run `analyze <unit>` and describe its dependency relationships.
4. Run `recommend` and explain why suggestions require review.
5. Discuss the user-space/kernel boundary and why a custom driver is not necessary for this problem.

## GitHub submission checklist

Create a GitHub repository, add the extracted project files, and push the repository. Keep the README and design note with the source. A sample command sequence:

```bash
git init
git add CMakeLists.txt README.md src docs .gitignore
git commit -m "Add BootScope Linux boot profiler"
git branch -M main
git remote add origin <your-github-repository-url>
git push -u origin main
```

Complete the upload by **5 October 2026**, then arrange the evaluation with your trainer.
