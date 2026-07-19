#!/bin/bash

# ==============================================================================
# Habit Tracker Agent Skill Installation Script
# ==============================================================================

set -e

echo "Downloading the latest release of Habit Tracker..."
mkdir -p /tmp/habit-skill
curl -sL https://github.com/nocerosoft/habit/releases/latest/download/habit-tracker-skill.zip -o /tmp/habit.zip

echo "Extracting files..."
unzip -q -o /tmp/habit.zip -d /tmp/habit-skill

echo "Installing binary to ~/.local/bin/habit..."
mkdir -p ~/.local/bin
mv /tmp/habit-skill/bin/habit ~/.local/bin/habit
chmod +x ~/.local/bin/habit

echo "Installing Hermes Skill definition to ~/.hermes/skills/productivity/habit/..."
mkdir -p ~/.hermes/skills/productivity/habit
mv /tmp/habit-skill/SKILL.md ~/.hermes/skills/productivity/habit/SKILL.md

echo "Cleaning up temporary files..."
rm -rf /tmp/habit.zip /tmp/habit-skill

echo ""
echo "✅ Installation complete!"
echo "You can now run 'habit record' directly in your terminal, or chat with Hermes."
