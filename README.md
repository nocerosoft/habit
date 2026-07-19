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

## Universal Agent Skills (VS Code Copilot, Claude Code, etc.)
Because this skill conforms to the [agentskills.io](https://agentskills.io) open standard, you can install it directly into any project workspace for compatible IDEs and agents.

Run this command inside your project directory to install the CLI tool and add the `SKILL.md` to your local `.agents/skills/habit` folder:

```bash
curl -sL https://raw.githubusercontent.com/nocerosoft/habit/master/install.sh | bash
```
Your IDE's agent will now automatically discover and use the Habit Tracker CLI to record your habits!

## Hermes Agent Skill (Optional)
If you are using the Hermes AI agent and want to install the skill globally, run the installer with the `hermes` argument:

```bash
curl -sL https://raw.githubusercontent.com/nocerosoft/habit/master/install.sh | bash -s -- hermes
```
