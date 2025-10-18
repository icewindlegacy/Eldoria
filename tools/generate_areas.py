#!/usr/bin/env python3
"""
Generate areas.html for the Eldoria MUD website
Parses .are files and creates a sortable area list
"""

import os
import re
import json
from pathlib import Path

def parse_area_file(filepath):
    """Parse an .are file and extract area information"""
    area_info = {
        'name': 'Unknown',
        'filename': os.path.basename(filepath),
        'builders': 'Unknown',
        'vnums': 'N/A',
        'level_range': 'All',
        'credits': 'Unknown',
        'continent': '0'
    }
    
    try:
        with open(filepath, 'r', encoding='latin-1') as f:
            in_areadata = False
            for line in f:
                line = line.strip()
                
                if line == '#AREADATA':
                    in_areadata = True
                    continue
                    
                if in_areadata:
                    if line == 'End':
                        break
                        
                    # Parse Name
                    if line.startswith('Name '):
                        area_info['name'] = line[5:].rstrip('~').strip()
                    
                    # Parse Builders
                    elif line.startswith('Builders '):
                        area_info['builders'] = line[9:].rstrip('~').strip()
                    
                    # Parse VNUMs
                    elif line.startswith('VNUMs '):
                        vnums = line[6:].strip()
                        area_info['vnums'] = vnums
                    
                    # Parse Credits (contains level range)
                    elif line.startswith('Credits '):
                        credits = line[8:].rstrip('~').strip()
                        area_info['credits'] = credits
                        
                        # Extract level range from credits like "{ 1  5} Builder Name~"
                        level_match = re.match(r'\{\s*(\d+)\s+(\d+)\s*\}', credits)
                        if level_match:
                            min_level = level_match.group(1)
                            max_level = level_match.group(2)
                            area_info['level_range'] = f"{min_level}-{max_level}"
                            area_info['min_level'] = int(min_level)
                            area_info['max_level'] = int(max_level)
                    
                    # Parse Continent
                    elif line.startswith('Continent '):
                        area_info['continent'] = line[10:].strip()
    
    except Exception as e:
        print(f"Error parsing {filepath}: {e}")
    
    return area_info

def generate_areas_html(areas):
    """Generate the HTML page for the area list"""
    
    html = '''<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Eldoria MUD - Area List</title>
    <link rel="stylesheet" href="style.css">
    <style>
        .search-box {
            margin: 20px 0;
            text-align: center;
        }
        
        .search-box input {
            width: 80%;
            max-width: 500px;
            padding: 10px;
            font-size: 16px;
            background: #001a00;
            border: 2px solid #00ff00;
            color: #00ff00;
            border-radius: 5px;
            font-family: 'Courier New', monospace;
        }
        
        .search-box input::placeholder {
            color: #006600;
        }
        
        .areas-table {
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
            background: rgba(0, 20, 0, 0.8);
            border: 2px solid #00ff00;
        }
        
        .areas-table th {
            background: linear-gradient(135deg, #003300, #006600);
            color: #00ff00;
            padding: 15px 10px;
            text-align: left;
            border-bottom: 2px solid #00ff00;
            cursor: pointer;
            user-select: none;
        }
        
        .areas-table th:hover {
            background: linear-gradient(135deg, #004400, #008800);
            text-shadow: 0 0 10px #00ff00;
        }
        
        .areas-table th::after {
            content: ' ⇅';
            opacity: 0.5;
        }
        
        .areas-table td {
            padding: 12px 10px;
            border-bottom: 1px solid #003300;
            color: #00dd00;
        }
        
        .areas-table tr:hover {
            background: rgba(0, 50, 0, 0.5);
        }
        
        .level-badge {
            display: inline-block;
            padding: 3px 8px;
            background: #003300;
            border: 1px solid #00ff00;
            border-radius: 3px;
            font-size: 0.9em;
            white-space: nowrap;
        }
        
        .level-newbie { border-color: #00ff00; color: #00ff00; }
        .level-low { border-color: #88ff00; color: #88ff00; }
        .level-mid { border-color: #ffff00; color: #ffff00; }
        .level-high { border-color: #ff8800; color: #ff8800; }
        .level-epic { border-color: #ff0000; color: #ff0000; }
        .level-all { border-color: #00dddd; color: #00dddd; }
        
        .back-button {
            margin: 20px 0;
            text-align: center;
        }
        
        .stats {
            text-align: center;
            margin: 20px 0;
            color: #00dd00;
            font-size: 1.1em;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1 class="glow">ELDORIA</h1>
            <p class="subtitle">~ Area List ~</p>
        </div>

        <div class="back-button">
            <a href="index.html" class="retro-button" style="text-decoration: none;">
                ⬅️ Back to Home
            </a>
        </div>

        <div class="content-box">
            <h2>🗺️ Explore the World of Eldoria 🗺️</h2>
            
            <div class="stats">
                <strong>Total Areas:</strong> <span id="totalAreas">''' + str(len(areas)) + '''</span>
            </div>
            
            <div class="search-box">
                <input type="text" id="searchInput" placeholder="🔍 Search areas by name, builder, or level..." onkeyup="filterAreas()">
            </div>
            
            <table class="areas-table" id="areasTable">
                <thead>
                    <tr>
                        <th onclick="sortTable(0)">Area Name</th>
                        <th onclick="sortTable(1)">Level Range</th>
                        <th onclick="sortTable(2)">VNUMs</th>
                        <th onclick="sortTable(3)">Builders</th>
                    </tr>
                </thead>
                <tbody id="areasBody">
'''
    
    # Sort areas by minimum level, then name
    areas.sort(key=lambda x: (x.get('min_level', 999), x['name'].lower()))
    
    for area in areas:
        level_range = area['level_range']
        level_class = 'level-all'
        
        # Determine level badge color
        if 'min_level' in area:
            min_lvl = area['min_level']
            if min_lvl <= 10:
                level_class = 'level-newbie'
            elif min_lvl <= 30:
                level_class = 'level-low'
            elif min_lvl <= 60:
                level_class = 'level-mid'
            elif min_lvl <= 90:
                level_class = 'level-high'
            else:
                level_class = 'level-epic'
        
        html += f'''                    <tr>
                        <td><strong>{area['name']}</strong></td>
                        <td><span class="level-badge {level_class}">{level_range}</span></td>
                        <td>{area['vnums']}</td>
                        <td>{area['builders']}</td>
                    </tr>
'''
    
    html += '''                </tbody>
            </table>
        </div>

        <div class="footer">
            <p>
                Based on ROM 2.4b6 | ShadowStorm by Synon & Davion<br>
                Built with ❤️ for the MUD community | Est. 2025
            </p>
        </div>
    </div>

    <canvas id="starfield"></canvas>

    <script>
        // Search/filter function
        function filterAreas() {
            const input = document.getElementById('searchInput');
            const filter = input.value.toLowerCase();
            const table = document.getElementById('areasTable');
            const tbody = document.getElementById('areasBody');
            const rows = tbody.getElementsByTagName('tr');
            let visibleCount = 0;
            
            for (let i = 0; i < rows.length; i++) {
                const row = rows[i];
                const cells = row.getElementsByTagName('td');
                let found = false;
                
                for (let j = 0; j < cells.length; j++) {
                    const cell = cells[j];
                    if (cell.textContent.toLowerCase().indexOf(filter) > -1) {
                        found = true;
                        break;
                    }
                }
                
                if (found) {
                    row.style.display = '';
                    visibleCount++;
                } else {
                    row.style.display = 'none';
                }
            }
            
            document.getElementById('totalAreas').textContent = visibleCount + ' / ''' + str(len(areas)) + '''';
        }
        
        // Sort table function
        function sortTable(columnIndex) {
            const table = document.getElementById('areasTable');
            const tbody = document.getElementById('areasBody');
            const rows = Array.from(tbody.getElementsByTagName('tr'));
            
            // Toggle sort direction
            const currentDir = table.getAttribute('data-sort-dir') || 'asc';
            const newDir = currentDir === 'asc' ? 'desc' : 'asc';
            table.setAttribute('data-sort-dir', newDir);
            
            rows.sort((a, b) => {
                let aValue = a.getElementsByTagName('td')[columnIndex].textContent.trim();
                let bValue = b.getElementsByTagName('td')[columnIndex].textContent.trim();
                
                // Handle level ranges specially
                if (columnIndex === 1) {
                    const aMatch = aValue.match(/(\\d+)/);
                    const bMatch = bValue.match(/(\\d+)/);
                    if (aMatch && bMatch) {
                        aValue = parseInt(aMatch[1]);
                        bValue = parseInt(bMatch[1]);
                    }
                }
                
                if (typeof aValue === 'number' && typeof bValue === 'number') {
                    return newDir === 'asc' ? aValue - bValue : bValue - aValue;
                } else {
                    if (newDir === 'asc') {
                        return aValue.localeCompare(bValue);
                    } else {
                        return bValue.localeCompare(aValue);
                    }
                }
            });
            
            // Rebuild tbody
            rows.forEach(row => tbody.appendChild(row));
        }

        // Starfield effect (reused from index.html)
        const canvas = document.getElementById('starfield');
        const ctx = canvas.getContext('2d');
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;

        const stars = [];
        const numStars = 100;

        for (let i = 0; i < numStars; i++) {
            stars.push({
                x: Math.random() * canvas.width,
                y: Math.random() * canvas.height,
                size: Math.random() * 2,
                speed: Math.random() * 0.5 + 0.1
            });
        }

        function drawStars() {
            ctx.fillStyle = 'rgba(0, 0, 0, 0.1)';
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            
            ctx.fillStyle = 'white';
            stars.forEach(star => {
                ctx.fillRect(star.x, star.y, star.size, star.size);
                star.y += star.speed;
                if (star.y > canvas.height) {
                    star.y = 0;
                    star.x = Math.random() * canvas.width;
                }
            });
            
            requestAnimationFrame(drawStars);
        }

        drawStars();

        window.addEventListener('resize', () => {
            canvas.width = window.innerWidth;
            canvas.height = window.innerHeight;
        });
    </script>
</body>
</html>
'''
    
    return html

def main():
    # Path to area directory
    area_dir = Path('/home/ro/eldoria-github/area')
    output_file = Path('/home/ro/public_html/areas.html')
    
    # Find all .are files
    area_files = sorted(area_dir.glob('*.are'))
    
    print(f"Found {len(area_files)} area files")
    
    # Parse all area files
    areas = []
    for area_file in area_files:
        area_info = parse_area_file(area_file)
        areas.append(area_info)
        print(f"  Parsed: {area_info['name']}")
    
    # Generate HTML
    html = generate_areas_html(areas)
    
    # Write to file
    output_file.write_text(html, encoding='utf-8')
    print(f"\n✅ Generated {output_file}")
    print(f"   Total areas: {len(areas)}")

if __name__ == '__main__':
    main()

