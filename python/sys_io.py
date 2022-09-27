from platform import platform
import pygame

from host import EHostPlatform

PHYS_SCREEN_W = 320
PHYS_SCREEN_H = 200
PHYS_SCALE = 4


class SysIO():
    def __init__(self, platform):
        self.platform = platform
        
        pygame.init()
        pygame.display.set_caption("alis")

        # create a surface on screen that has the size of 240 x 180
        self.screen = pygame.display.set_mode((PHYS_SCREEN_W * PHYS_SCALE,
                                               PHYS_SCREEN_H * PHYS_SCALE),
                                              pygame.HWSURFACE | pygame.DOUBLEBUF | pygame.RESIZABLE)

    def render_frame(self):
        pass

    def poll_event(self):
        pass

    def get_model(self):
        if self.platform == EHostPlatform.Atari:
            return 0x456
        else:
            return "crashme :)"