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

if [ "$1" = "hermes" ]; then
    echo "Installing Hermes Skill definition to ~/.hermes/skills/productivity/habit/..."
    mkdir -p ~/.hermes/skills/productivity/habit
    mv /tmp/habit-skill/SKILL.md ~/.hermes/skills/productivity/habit/SKILL.md
    INSTALL_TYPE="hermes"
else
    echo "Installing Universal Agent Skill to ./.agents/skills/habit/..."
    mkdir -p .agents/skills/habit
    mv /tmp/habit-skill/SKILL.md .agents/skills/habit/SKILL.md
    INSTALL_TYPE="universal"
fi
echo "Cleaning up temporary files..."
rm -rf /tmp/habit.zip /tmp/habit-skill

echo ""
echo "✅ Installation complete!"
echo "You can now run 'habit record' directly in your terminal."

if [ "$INSTALL_TYPE" = "universal" ]; then
    echo "Your IDE agent (Copilot, Claude) can now use this skill automatically."
    echo "💡 Tip: To install globally for the Hermes agent instead, run:"
    echo "   curl -sL https://raw.githubusercontent.com/nocerosoft/habit/master/install.sh | bash -s -- hermes"
else
    echo "You can now chat with Hermes to track your habits."
fi
