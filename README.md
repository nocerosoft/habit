# Habit Tracker

A command-line habit tracker written in C. It uses SQLite for local data storage.

## Features
- Record daily habit completions.
- Set weekly and monthly completion goals.
- Display a 24-week activity heatmap in the terminal.

## Installation & Compilation
To compile and run the application manually:
1. Clone this repository
2. Install SQLite3 dependencies (`sudo apt-get install libsqlite3-dev` on Debian/Ubuntu)
3. Run `make`
4. Use the CLI: `./bin/habit help`

## Hermes Agent Skill (Optional)
If you prefer not to type commands manually, you can install this tool as a skill for the Hermes AI agent. This allows you to track habits using natural language.

Run this command to download the compiled binary and skill definition into your Hermes agent:

```bash
mkdir -p ~/hermes/skills/productivity/habit && curl -L https://github.com/nocerosoft/habit/releases/download/v0.1.0/habit-tracker-skill.zip -o /tmp/habit.zip && unzip -o /tmp/habit.zip -d ~/hermes/skills/productivity/habit && rm /tmp/habit.zip
```
