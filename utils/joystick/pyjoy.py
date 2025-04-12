#!/usr/bin/env python3

import sys
import pygame

pygame.init()
pygame.joystick.init()

# Check for connected joysticks
if pygame.joystick.get_count() == 0:
    print("No joystick connected")
    sys.exit()

# Init joystick
joystick = pygame.joystick.Joystick(0)
joystick.init()

# Configure pygame window
WIDTH = 600
HEIGHT = 400
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Joystick Visualizer")
font = pygame.font.SysFont(None, 24)

clock = pygame.time.Clock()


def draw_stick_circle(center_x, center_y, axis_x_val, axis_y_val, radius, label):
    # Draw title
    title = font.render("JoyStick Simple Visualizer", True, (255, 0, 0))
    screen.blit(title, (WIDTH // 3, 15))
    # Draw the outer circle
    pygame.draw.circle(screen, (100, 100, 255), (center_x, center_y), radius, 2)
    pygame.draw.line(screen, (80, 80, 80), (center_x - radius, center_y), (center_x + radius, center_y), 1)
    pygame.draw.line(screen, (80, 80, 80), (center_x, center_y - radius), (center_x, center_y + radius), 1)

    # Draw inner circle: current position
    dot_x = int(center_x + axis_x_val * radius)
    dot_y = int(center_y + axis_y_val * radius)
    pygame.draw.circle(screen, (0, 200, 255), (dot_x, dot_y), 8)

    # Draw label
    text = font.render(label, True, (255, 255, 255))
    screen.blit(text, (center_x - text.get_width() // 2, center_y + radius + 10))


while True:
    screen.fill((20, 20, 20))
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            sys.exit()

    # For debug get mouse pose
    mouse_x, mouse_y = pygame.mouse.get_pos()
    coord_text = f"Mouse: ({mouse_x}, {mouse_y})"
    text_surface = font.render(coord_text, True, (255, 255, 255))
    screen.blit(text_surface, (450, 370))

    axis0 = joystick.get_axis(0)
    axis1 = joystick.get_axis(1)
    axis2 = joystick.get_axis(2) if joystick.get_numaxes() > 2 else 0
    axis3 = joystick.get_axis(3) if joystick.get_numaxes() > 3 else 0

    draw_stick_circle(180, 180, axis0, axis1, 60, "Left Stick")
    draw_stick_circle(420, 180, axis3, axis2, 60, "Right Stick")

    pygame.display.flip()
    clock.tick(60)

