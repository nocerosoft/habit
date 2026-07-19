# Habit Tracker

A command-line habit tracker written in C. It uses SQLite for local data storage.

## Features
- Record daily habit completions.
- Set weekly and monthly completion goals.
- Display a 24-week activity heatmap in the terminal.

## Installation & Compilation
To compile and run the application manually:
1. Clone this repository
2. Run `make`
3. Use the CLI: `./bin/habit help`

## Hermes Agent Skill (Optional)
If you prefer not to type commands manually, you can install this tool as a skill for the Hermes AI agent. This allows you to track habits using natural language.

Run this command to download the compiled binary and skill definition into your Hermes agent:

```bash
curl -sL https://raw.githubusercontent.com/nocerosoft/habit/master/install.sh | bash
```
