import math

def generate_color_wheel_svg(filename="color_wheel.svg", size=400):
    cx, cy = size / 2, size / 2
    radius = (size / 2) - 20
    
    with open(filename, "w") as f:
        f.write(f'<svg width="{size}" height="{size}" viewBox="0 0 {size} {size}" xmlns="http://www.w3.org/2000/svg">\n')
        
        # Draw the wheel segments
        num_segments = 256
        for i in range(num_segments):
            angle = (i / num_segments) * 360
            start_angle = (angle - 90) * (math.pi / 180)
            end_angle = ((angle + 360/num_segments) - 90) * (math.pi / 180)
            
            x1 = cx + radius * math.cos(start_angle)
            y1 = cy + radius * math.sin(start_angle)
            x2 = cx + radius * math.cos(end_angle)
            y2 = cy + radius * math.sin(end_angle)
            
            # HSV to RGB conversion (simplified for V=1, S=1)
            h = i / 255.0 * 360
            c = 1
            x = c * (1 - abs((h / 60) % 2 - 1))
            m = 0
            
            if 0 <= h < 60: r, g, b = c, x, 0
            elif 60 <= h < 120: r, g, b = x, c, 0
            elif 120 <= h < 180: r, g, b = 0, c, x
            elif 180 <= h < 240: r, g, b = 0, x, c
            elif 240 <= h < 300: r, g, b = x, 0, c
            else: r, g, b = c, 0, x
            
            r, g, b = int((r+m)*255), int((g+m)*255), int((b+m)*255)
            color = f"rgb({r},{g},{b})"
            
            f.write(f'  <path d="M {cx} {cy} L {x1} {y1} A {radius} {radius} 0 0 1 {x2} {y2} Z" fill="{color}" stroke="none" />\n')

        # Add labels
        labels = [
            (0, "Red (0/255)"),
            (43, "Yellow (43)"),
            (85, "Green (85)"),
            (128, "Cyan (128)"),
            (170, "Blue (170)"),
            (213, "Magenta (213)"),
        ]
        
        for hue_val, text in labels:
            angle_deg = (hue_val / 255.0) * 360
            angle_rad = (angle_deg - 90) * (math.pi / 180)
            
            # Position for text (slightly outside)
            text_r = radius - 40
            tx = cx + text_r * math.cos(angle_rad)
            ty = cy + text_r * math.sin(angle_rad)
            
            # Text styling
            f.write(f'  <text x="{tx}" y="{ty}" font-family="Arial" font-size="12" font-weight="bold" fill="white" text-anchor="middle" dominant-baseline="middle" stroke="black" stroke-width="0.5">{text}</text>\n')

        f.write('</svg>\n')

if __name__ == "__main__":
    generate_color_wheel_svg()
