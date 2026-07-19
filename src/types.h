#ifndef TYPES_H
#define TYPES_H

// Data structures for habit tracker

typedef struct {
    int id;
    char name[256];
    int weekly_goal;
    int monthly_goal;
    int current_streak;
    int weekly_count;
    int monthly_count;
} HabitStats;

typedef struct {
    char name[256];
} RemainingHabit;

typedef struct {
    int streak;
    int is_duplicate;
    int completed_today;
    int total_active;
    RemainingHabit *remaining;
    int remaining_count;
    int weekly_goal;
    int monthly_goal;
    int weekly_count;
    int monthly_count;
} RecordResult;

#endif // TYPES_H
