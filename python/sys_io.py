from dataclasses import dataclass
import os
import pygame
from random import randint

from host import EHostPlatform

PHYS_SCREEN_W = 320
PHYS_SCREEN_H = 200
PHYS_SCALE = 4

# =============================================================================
@dataclass
class SysMouse():
    x = 0
    y = 0
    bt = 0


# =============================================================================
class SysIO():
    def __init__(self, platform, workdir):
        self.platform = platform
        self.workdir = workdir
        self.mouse = SysMouse()
        
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
        print("sys.get_model is STUBBED...")
        if self.platform == EHostPlatform.Atari:
            return 0x456
        else:
            return "crashme :)"

    def random(self):
        return randint(0, 0xffff)

    def fexists(self, fname):
        ret = False
        try:
            with open(os.path.join(self.workdir, fname), "r") as f:
                f.close()
                ret = True
        except:
            pass
        return ret
    
    def shiftkey(self):
        # TODO: return 0xff if shift key is pressed at this moment ?
        return 0
    
    def inkey(self):
        # TODO: return last key pressed
        return 0

    def io_mouse(self) -> SysMouse:
        self.mouse.x = pygame.mouse.get_pos()[0]
        self.mouse.y = pygame.mouse.get_pos()[1]
        bt = pygame.mouse.get_pressed(3)
        self.mouse.bt = (bt[1] << 1) | bt[0]
        return self.mouse