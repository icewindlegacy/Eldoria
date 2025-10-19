# NULL Safety Audit Plan for Eldoria MUD

## Overview
This document outlines a comprehensive targeted approach to identify and fix NULL-related crashes in the Eldoria MUD codebase, similar to the bugs we've already fixed.

## Known Issues Fixed So Far
1. ✅ **board.c** - `file_open()` returning NULL → segfault in `fprintf()`
2. ✅ **religion.c** - `rlgedit_faction(ch, NULL)` → crash in `one_argument()`
3. ✅ **act_info.c** - `pretitle` pointer comparison issue
4. ✅ **act_move.c** - `skill_lookup("hunt")` returning -1 → invalid array access

## Estimated Cost
- **Tokens**: 30,000-50,000 
- **Time**: 1-2 hours
- **Coverage**: ~90% of critical NULL-related bugs

## Search Patterns to Investigate

### 1. File I/O Operations (Highest Priority)
**Pattern**: `file_open()` calls without NULL checks
- Search for: `fp = file_open\(|fopen\(`
- Files to check: All .c files that do file operations
- Fix: Add `if (!fp) { bug(); return; }` after each call

**Pattern**: `fread_*` functions on NULL file pointers
- Search for: `fread_string|fread_word|fread_number`
- Ensure file pointer is valid before reading

### 2. String/Argument Parsing
**Pattern**: `one_argument()`, `argument` being NULL
- Search for: `one_argument\([^,]+, NULL\)|one_argument\(NULL,`
- Files: All command handlers (do_* functions)
- Fix: Pass `""` instead of NULL

**Pattern**: `str_cmp|strcmp` with NULL pointers
- Search for: `str_cmp|strcmp|str_prefix`
- Check if either argument could be NULL
- Add NULL checks: `if (!str) return;`

### 3. Index Lookups
**Pattern**: `get_*_index()` returns not checked
- Search for: `get_mob_index|get_obj_index|get_room_index`
- Look for direct usage without NULL check
- Example: `pMob = get_mob_index(vnum); pMob->short_descr` ← CRASH
- Fix: `if (!pMob) { send_to_char("No such mob"); return; }`

**Pattern**: `skill_lookup()` returning -1
- Search for: `skill_lookup\(`
- Check if result used in `skill_table[sn]` without validation
- Fix: `if (sn < 0) return;`

### 4. Recursive Editor Calls
**Pattern**: OLC editors calling themselves with NULL/invalid args
- Search in: `olc*.c`, `*edit.c` files
- Look for: `*edit(ch, NULL)` or similar
- Fix: Pass `""` for usage display

### 5. Character/Object References
**Pattern**: `ch->pcdata` without IS_NPC check
- Search for: `ch->pcdata->` 
- Ensure preceded by `!IS_NPC(ch)` check
- NPCs don't have pcdata!

**Pattern**: `victim->master` without NULL check
- Search for: `->master|->leader|->pet`
- These pointers can be NULL

### 6. Descriptor/Connection Issues
**Pattern**: `ch->desc` without check
- Search for: `ch->desc->pEdit|ch->desc->`
- Link-dead players have NULL descriptor
- Add: `if (!ch->desc) return;`

## High-Risk Files to Prioritize

### Critical (Check First)
- `board.c` - File I/O ✅ FIXED
- `save.c` - Player save/load
- `db.c`, `db2.c` - Database loading
- `nanny.c` - Character creation/login
- `religion.c` - Religion editor ✅ FIXED

### OLC Editors (User Input Heavy)
- `olc.c`
- `olc_act.c`
- `olc_save.c`
- `olc_load_*.c`
- `hedit.c`

### Command Handlers
- `act_*.c` files (act_info, act_wiz, act_obj, etc.)
- `interp.c` - Command interpreter
- All do_* functions

### Combat/Skills
- `fight.c` ✅ (vorpal added)
- `skills.c`
- `magic.c`, `magic2.c`

### Communication
- `comm.c`
- `act_comm.c`

## Methodology

### Phase 1: Automated Search (5,000 tokens)
Run grep patterns to identify potential issues:
```bash
# Find file_open without checks
grep -n "file_open" *.c | grep -v "if.*file_open"

# Find skill_lookup without validation  
grep -n "skill_table\[skill_lookup" *.c

# Find get_*_index without NULL checks
grep -n "get_.*_index.*->" *.c
```

### Phase 2: Targeted Review (20,000-30,000 tokens)
- Read identified problem areas
- Verify if NULL check exists
- Understand the context
- Implement appropriate fix

### Phase 3: Testing & Validation (10,000-15,000 tokens)
- Compile after each batch of fixes
- Check for introduced errors
- Verify logic still works correctly
- Document each fix

## Common Fix Patterns

### File Operations
```c
// BEFORE (CRASH RISK):
fp = file_open(filename, "w");
fprintf(fp, "data");

// AFTER (SAFE):
fp = file_open(filename, "w");
if (!fp) {
    bug("function_name: could not open file", 0);
    return;
}
fprintf(fp, "data");
file_close(fp);
```

### Index Lookups
```c
// BEFORE (CRASH RISK):
pMob = get_mob_index(vnum);
sprintf(buf, "%s", pMob->short_descr);

// AFTER (SAFE):
pMob = get_mob_index(vnum);
if (!pMob) {
    send_to_char("No such mobile.\n\r", ch);
    return;
}
sprintf(buf, "%s", pMob->short_descr);
```

### Skill Lookups
```c
// BEFORE (CRASH RISK):
WAIT_STATE(ch, skill_table[skill_lookup("hunt")].beats);

// AFTER (SAFE):
sn = skill_lookup("hunt");
if (sn < 0) {
    send_to_char("Skill not available.\n\r", ch);
    return;
}
WAIT_STATE(ch, skill_table[sn].beats);
```

### String Arguments
```c
// BEFORE (CRASH RISK):
some_function(ch, NULL);  // Called recursively for usage

// AFTER (SAFE):
some_function(ch, "");    // Empty string is safe
```

### Pointer Dereferences
```c
// BEFORE (CRASH RISK):
if (ch->pcdata->pretitle == '\0')  // Wrong! Comparing pointer to char

// AFTER (SAFE):
if (ch->pcdata->pretitle == NULL || ch->pcdata->pretitle[0] == '\0')
```

## Expected Results

After completion:
- ✅ 90%+ reduction in NULL-related segfaults
- ✅ Graceful error messages instead of crashes
- ✅ Better debugging info via bug() calls
- ✅ More stable MUD for players
- ✅ Easier to identify remaining issues

## How to Request This Audit

Simply say:
> "Claude, please perform the comprehensive NULL safety audit as outlined in NULL_SAFETY_AUDIT_PLAN.md"

Or break it into phases:
> "Claude, run Phase 1 of the NULL audit - automated search"
> "Claude, review and fix the file I/O operations from the audit"

## Notes
- This audit is proactive, not exhaustive
- Runtime testing still recommended
- Some bugs may only appear in specific scenarios
- Consider adding debug logging for suspicious areas
- Keep backups before major changes!

## Version History
- **2025-10-19**: Initial plan created based on bugs found:
  - board.c file_open crashes
  - religion.c NULL argument crash  
  - act_move.c skill lookup crash
  - act_info.c pretitle pointer issues

