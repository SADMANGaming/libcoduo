#include "jump.hpp"
extern cvar_t *jump_slowdownEnable;
extern cvar_t *jump_height;


extern pmove_t *pm;

double getJumpHeight()
{
    return jump_height->value;
}

void hook_Jump_Check()
{
    float value = getJumpHeight();
    Jump_Set_f(value);
}

double custom_PM_GetReducedFriction()
{
    playerState_t *ps = ((pmove_t*)*((int*)pm))->ps;

    // If slowdown is disabled, return normal friction
    if (jump_slowdownEnable->integer == 0)
        return 1.0;

    // Only apply slowdown if pm_time > 0
    if (ps->pm_time > 0)
    {
        if (ps->pm_time < 1700)
            return (double)ps->pm_time * 1.5 * 0.00058823527 + 1.0;
        return 2.5;
    }

    // Normal movement
    return 1.0;
}

double custom_Jump_GetLandFactor()
{
    playerState_t *ps = ((pmove_t*)*((int*)pm))->ps;

    if (jump_slowdownEnable->integer == 0)
        return 1.0;

    if (ps->pm_time > 0)
    {
        if (ps->pm_time < 1700)
            return (double)ps->pm_time * 1.5 * 0.00058823527 + 1.0;
        return 2.5;
    }

    return 1.0;
}
