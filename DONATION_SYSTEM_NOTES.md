# Donation Room/Pit Persistence System

## Overview
This system saves and loads objects from donation rooms and donation pits so they persist through copyovers and reboots.

## Files Modified
- `src/donation.c` - Main save/load functions with debug logging
- `src/act_wiz.c` - Integrated into copyover process
- `src/update.c` - Periodic saves during game updates (already present)

## How It Works

### Save Process
**When:** Called during:
1. Copyover (hot reboot) - in `do_copyover()` function
2. Periodic updates - every update cycle in `update.c`

**What it saves:**
- Objects in rooms with `ROOM_DONATION` flag
- Objects inside containers with vnum matching `OBJ_VNUM_PIT`

**Where:** `../area/donation.dat` and `../area/tokens.dat`

### Load Process
**When:** Called during MUD startup in `db.c` (line ~513)

**What it does:**
1. Reads objects from donation.dat
2. First tries to place them in pits (if OBJ_VNUM_PIT is configured)
3. Falls back to placing them in the first available donation room

## Debug Logging
The system outputs detailed logs to **stderr** showing:
- `save_donation_pits: Starting save, OBJ_VNUM_PIT=X`
- `save_donation_pits: Found donation room X, contents=yes/no`
- `save_donation_pits: Saving object vnum X from donation room`
- `save_donation_pits: Saved X objects from Y donation rooms and Z pits`

And on load:
- `load_donation_pits: Starting load, OBJ_VNUM_PIT=X`
- `load_donation_pits: Placed object vnum X in donation room Y`
- `load_donation_pits: Loaded X objects (Y to pits, Z to donation rooms)`

## Testing
1. Place objects in a donation room (e.g., room 3037 in Midgaard)
2. Do a copyover: `copyover`
3. Check stderr/logs for debug messages
4. Verify objects are still in the donation room
5. Check `../area/donation.dat` - should contain object data

## Key Changes Made
1. ✅ Fixed file paths to use `../area/donation.dat`
2. ✅ Added `#include "const.h"` for OBJ_VNUM_PIT
3. ✅ Replaced compile-time `#ifdef` with runtime `if()` checks
4. ✅ **Integrated into copyover process** - this was the critical missing piece
5. ✅ Added comprehensive debug logging
6. ✅ Fixed load logic to check if objects are already placed

## Important Notes
- Objects are saved recursively (containers and their contents)
- The system only saves top-level objects in rooms, not nested contents (those are saved with their parent)
- During load, objects try pits first, then fall back to donation rooms
- Debug messages help troubleshoot if objects aren't being found or saved

