---
name: Habit Tracker
description: A command-line habit tracker to record and review daily habits, streaks, weekly and monthly goals.
version: 1.0.0
---

# Habit Tracker Agent Skill

This skill allows you to track and manage user habits using a compiled C CLI application backed by SQLite. 
The executable is located at `bin/habit` relative to this skill directory.

**Important Application Behaviors:**
- **Auto-Normalization**: The app automatically Title Cases all habit names (e.g., `drink water` becomes `Drink Water`). You do not need to manually normalize strings before passing them to the CLI.
- **Idempotency**: Recording the same habit multiple times in a single day is completely safe. The app will simply return a message saying it was already recorded today.

## Usage Instructions

Whenever the user asks to manage or track their habits, use the following commands:

### Record a Habit
To log a habit for today (or create it if it doesn't exist), run:
```bash
./bin/habit record "<activity_name>"
```

### List Habits
To list all tracked habits, current streaks, and progress for weekly/monthly goals, run:
```bash
./bin/habit list
```

### Set a Goal
To set a specific weekly or monthly goal for a habit, run:
```bash
./bin/habit goal "<activity_name>" <target> <weekly|monthly>
```
*Example: `./bin/habit goal "Workout" 3 weekly`*

### View Heatmap
To view a 24-week GitHub-style heatmap of habit completions, run:
```bash
./bin/habit heatmap
```

### Remove a Habit
To delete a habit and all its history, run:
```bash
./bin/habit remove "<activity_name>"
```

## Handling Natural Language Input
Often, the user will interact with you using natural language rather than explicit commands. You must interpret their intent, extract the core activity, run the corresponding tool, and then report the app's output back to the user.

**Examples:**

1. **User says**: *"i swim today"* or *"i sang today"*
   - **Your Action**: Extract the core activity ("Swim" or "Sing") and run: `./bin/habit record "Swim"`
   - **Your Response**: Return the exact CLI output (e.g., *"Recorded 'Swim' for today! You have completed 1/4 habits."*)

2. **User says**: *"what ive been done"* or *"what is my progress"*
   - **Your Action**: Run: `./bin/habit list` (and optionally `./bin/habit heatmap`)
   - **Your Response**: Summarize the returned table or heatmap for the user.

3. **User says**: *"what to do this week"* or *"what do you think i should do today"*
   - **Your Action**: Run: `./bin/habit list`
   - **Your Response**: Identify habits that have not yet met their weekly/monthly goals or that have low completion counts, and recommend them to the user.

4. **User says**: *"i want to run 3 times a week"*
   - **Your Action**: Run: `./bin/habit goal "Run" 3 weekly`
   - **Your Response**: Return the confirmation from the CLI tool.

5. **User says**: *"i don't want to track swimming anymore"*
   - **Your Action**: Run: `./bin/habit remove "Swim"`
   - **Your Response**: Confirm that the habit has been deleted.

## Data Storage
The habit data is stored locally in `habits.db` in the current working directory. You do not need to interact with the database directly; the `habit` executable handles all SQLite operations.
