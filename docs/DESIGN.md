# BootScope Design Notes

## Goals

Provide an inspectable capstone implementation in C++ that helps a Linux user understand startup timings, service dependencies, and areas worth reviewing.

## Components

| Component | Responsibility | Linux interface |
| --- | --- | --- |
| CLI dispatcher | Routes command and validates arguments | Process argv |
| Command runner | Executes fixed utilities and captures output | `fork`, `execvp`, pipe, `waitpid` |
| Profiler | Displays startup timings and boot summary | `systemd-analyze` |
| Dependency analyzer | Displays forward/reverse unit graph | `systemctl list-dependencies` |
| Recommendation engine | Selects slow units for human review | Profiler output |

## Data flow

The command runner invokes tools directly with argument vectors, without a shell. The profiler parses systemd's human-readable timing output for JSON presentation. Dependency analysis passes the supplied unit as a single argument after validating its characters. Tool output remains host-specific and may vary with systemd versions.

## Kernel and driver concepts

The application is a user-space observer. systemd and the kernel expose managed system state through established interfaces. A custom device driver is not a useful dependency for service boot analysis: drivers serve hardware devices and would add risk without improving the requested measurements. The project therefore demonstrates the OS boundary and process/interface architecture without kernel code.

## Future extensions

- Store timestamped profiles and compare runs.
- Parse structured systemd output where supported.
- Add CSV export and configurable thresholds.
- Correlate unit timings with journal messages and critical-chain position.
