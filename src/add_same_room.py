#!/usr/bin/env python3
"""
Script to add same_room() checks to character and object loops in ROM MUD code.
"""

import re
import sys

def add_same_room_checks(content, filename):
    """Add same_room() checks to for loops that iterate over people or objects."""
    
    modified = False
    lines = content.split('\n')
    result = []
    i = 0
    
    while i < len(lines):
        line = lines[i]
        
        # Pattern 1: for ( victim = ... people; victim != NULL; victim = victim->next_in_room )
        people_match = re.match(r'(\s*)for\s*\(\s*(\w+)\s*=\s*.*?people\s*;.*?next_in_room\s*\)', line)
        
        # Pattern 2: for ( obj = ... contents; obj != NULL; obj = obj->next_content )
        obj_match = re.match(r'(\s*)for\s*\(\s*(\w+)\s*=\s*.*?contents\s*;.*?next_content\s*\)', line)
        
        if people_match or obj_match:
            indent = people_match.group(1) if people_match else obj_match.group(1)
            var_name = people_match.group(2) if people_match else obj_match.group(2)
            
            result.append(line)
            i += 1
            
            # Look for the opening brace
            while i < len(lines) and '{' not in lines[i]:
                result.append(lines[i])
                i += 1
            
            if i < len(lines) and '{' in lines[i]:
                result.append(lines[i])  # Add the { line
                
                # Check if same_room check already exists or if it's a return function
                check_exists = False
                is_return_func = False
                for j in range(i+1, min(i+8, len(lines))):
                    if 'same_room' in lines[j]:
                        check_exists = True
                        break
                    if 'return ' in lines[j] and 'continue' not in lines[j]:
                        is_return_func = True
                
                if not check_exists and not is_return_func:
                    # Add the same_room check
                    if people_match:
                        result.append(f"{indent}    if(!same_room(ch, {var_name}, NULL)) continue;")
                    else:  # obj_match
                        result.append(f"{indent}    if(!same_room(ch, NULL, {var_name})) continue;")
                    modified = True
                    print(f"Added same_room check in {filename} at line {i+1} for variable '{var_name}'")
                
                i += 1
                continue
        
        result.append(line)
        i += 1
    
    return '\n'.join(result), modified

def process_file(filepath):
    """Process a single file."""
    try:
        with open(filepath, 'r', encoding='latin-1') as f:
            content = f.read()
        
        new_content, modified = add_same_room_checks(content, filepath)
        
        if modified:
            with open(filepath, 'w', encoding='latin-1') as f:
                f.write(new_content)
            print(f"✓ Modified {filepath}")
            return True
        else:
            print(f"  No changes needed in {filepath}")
            return False
    except Exception as e:
        print(f"✗ Error processing {filepath}: {e}")
        return False

if __name__ == "__main__":
    files_to_process = [
        "instaroom.c",
        "mob_prog.c",
        "quest.c",
        "bard.c",
        "lycanth.c",
        "religion.c",
        "guild.c",
        "trivia.c",
    ]
    
    modified_count = 0
    for filename in files_to_process:
        if process_file(filename):
            modified_count += 1
    
    print(f"\n{modified_count} file(s) modified")

